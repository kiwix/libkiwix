

#include "aria2.h"
#include "xmlrpc.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <tools/otherTools.h>
#include <tools/pathTools.h>
#include <tools/stringTools.h>
#include <downloader.h> // For AriaError

#ifdef _WIN32
# define ARIA2_CMD "aria2c.exe"
#else
# define ARIA2_CMD "aria2c"
# include <unistd.h>
#endif



namespace kiwix {

Aria2::Aria2(std::string sessionFileDir):
  mp_aria(nullptr),
  m_port(42042),
  m_secret("kiwixariarpc"),
  mp_curl(nullptr),
  m_lock(PTHREAD_MUTEX_INITIALIZER)
{
  m_downloadDir = getDataDirectory();
  makeDirectory(m_downloadDir);
  std::vector<const char*> callCmd;

  std::string rpc_port = "--rpc-listen-port=" + to_string(m_port);
  std::string download_dir = "--dir=" + getDataDirectory();
  std::string session_file;
  if (sessionFileDir.empty()) {
    session_file = appendToDirectory(getDataDirectory(), "kiwix.session");
  } else {
    session_file = appendToDirectory(sessionFileDir, "kiwix.session");
  }
  std::string session = "--save-session=" + session_file;
  std::string inputFile = "--input-file=" + session_file;
//  std::string log_dir = "--log=\"" + logDir + "\"";
#ifdef _WIN32
  int pid = GetCurrentProcessId();
#else
  pid_t pid = getpid();
#endif
  std::string stop_with_pid = "--stop-with-process=" + to_string(pid);
  std::string rpc_secret = "--rpc-secret=" + m_secret;
  m_secret = "token:"+m_secret;

  std::string aria2cmd = appendToDirectory(
    removeLastPathElement(getExecutablePath(true)),
    ARIA2_CMD);
  if (fileExists(aria2cmd)) {
    // A local aria2c exe exists (packaged with kiwix-desktop), use it.
    callCmd.push_back(aria2cmd.c_str());
  } else {
    // Try to use a potential installed aria2c.
    callCmd.push_back(ARIA2_CMD);
  }
  callCmd.push_back("--enable-rpc");
  callCmd.push_back(rpc_secret.c_str());
  callCmd.push_back(rpc_port.c_str());
  callCmd.push_back(download_dir.c_str());
  if (fileExists(session_file)) {
    callCmd.push_back(inputFile.c_str());
  }
  callCmd.push_back(session.c_str());
//  callCmd.push_back(log_dir.c_str());
  callCmd.push_back("--auto-save-interval=10");
  callCmd.push_back(stop_with_pid.c_str());
  callCmd.push_back("--allow-overwrite=true");
  callCmd.push_back("--dht-entry-point=router.bittorrent.com:6881");
  callCmd.push_back("--dht-entry-point6=router.bittorrent.com:6881");
  callCmd.push_back("--quiet=true");
  callCmd.push_back("--bt-enable-lpd=true");
  callCmd.push_back("--always-resume=true");
  callCmd.push_back("--max-concurrent-downloads=42");
  callCmd.push_back("--rpc-max-request-size=6M");
  callCmd.push_back("--file-allocation=none");
  std::string launchCmd;
  for (auto &cmd : callCmd) {
      launchCmd.append(cmd).append(" ");
  }
  mp_aria = Subprocess::run(callCmd);
  mp_curl = curl_easy_init();
  char errbuf[CURL_ERROR_SIZE];
  curl_easy_setopt(mp_curl, CURLOPT_URL, "http://localhost/rpc");
  curl_easy_setopt(mp_curl, CURLOPT_PORT, m_port);
  curl_easy_setopt(mp_curl, CURLOPT_POST, 1L);
  curl_easy_setopt(mp_curl, CURLOPT_ERRORBUFFER, errbuf);

  int watchdog = 50;
  while(--watchdog) {
    sleep(10);
    errbuf[0] = 0;
    auto res = curl_easy_perform(mp_curl);
    if (res == CURLE_OK) {
      break;
    } else if (watchdog == 1) {
      std::cerr <<" curl_easy_perform() failed." << std::endl;
      fprintf(stderr, "\nlibcurl: (%d) ", res);
      if (errbuf[0] != 0) {
        std::cerr << errbuf << std::endl;
      } else {
        std::cerr << curl_easy_strerror(res) << std::endl;
      }
    }
  }
  if (!watchdog) {
    curl_easy_cleanup(mp_curl);
    throw std::runtime_error("Cannot connect to aria2c rpc. Aria2c launch cmd : " + launchCmd);
  }
}

Aria2::~Aria2()
{
  curl_easy_cleanup(mp_curl);
}

void Aria2::close()
{
  saveSession();
  shutdown();
}

size_t write_callback_to_iss(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  auto str = static_cast<std::stringstream*>(userdata);
  str->write(ptr, nmemb);
  return nmemb;
}

std::string Aria2::doRequest(const MethodCall& methodCall)
{
  pthread_mutex_lock(&m_lock);
  auto requestContent = methodCall.toString();
  std::stringstream stringstream;
  CURLcode res;
  curl_easy_setopt(mp_curl, CURLOPT_POSTFIELDSIZE, requestContent.size());
  curl_easy_setopt(mp_curl, CURLOPT_POSTFIELDS, requestContent.c_str());
  curl_easy_setopt(mp_curl, CURLOPT_WRITEFUNCTION, &write_callback_to_iss);
  curl_easy_setopt(mp_curl, CURLOPT_WRITEDATA, &stringstream);
  res = curl_easy_perform(mp_curl);
  if (res == CURLE_OK) {
    long response_code;
    curl_easy_getinfo(mp_curl, CURLINFO_RESPONSE_CODE, &response_code);
    pthread_mutex_unlock(&m_lock);
    if (response_code != 200) {
      throw std::runtime_error("Invalid return code from aria");
    }
    auto responseContent = stringstream.str();
    MethodResponse response(responseContent);
    if (response.isFault()) {
      throw AriaError(response.getFault().getFaultString());
    }
    return responseContent;
  }
  pthread_mutex_unlock(&m_lock);
  throw std::runtime_error("Cannot perform request");
}

std::string Aria2::addUri(const std::vector<std::string>& uris, const std::vector<std::pair<std::string, std::string>>& options)
{
  MethodCall methodCall("aria2.addUri", m_secret);
  auto uriParams = methodCall.newParamValue().getArray();
  for (auto& uri : uris) {
    uriParams.addValue().set(uri);
  }
  for (auto& option : options) {
    methodCall.newParamValue().getStruct().addMember(option.first).getValue().set(option.second);
  }
  auto ret = doRequest(methodCall);
  MethodResponse response(ret);
  return response.getParamValue(0).getAsS();
}

std::string Aria2::tellStatus(const std::string& gid, const std::vector<std::string>& statusKey)
{
  MethodCall methodCall("aria2.tellStatus", m_secret);
  methodCall.newParamValue().set(gid);
  if (!statusKey.empty()) {
    auto statusArray = methodCall.newParamValue().getArray();
    for (auto& key : statusKey) {
      statusArray.addValue().set(key);
    }
  }
  return doRequest(methodCall);
}

std::vector<std::string> Aria2::tellActive()
{
  MethodCall methodCall("aria2.tellActive", m_secret);
  auto statusArray = methodCall.newParamValue().getArray();
  statusArray.addValue().set(std::string("gid"));
  auto responseContent = doRequest(methodCall);
  MethodResponse response(responseContent);
  std::vector<std::string> activeGID;
  int index = 0;
  while(true) {
    try {
      auto structNode = response.getParamValue(0).getArray().getValue(index++).getStruct();
      auto gidNode = structNode.getMember("gid");
      activeGID.push_back(gidNode.getValue().getAsS());
    } catch (InvalidRPCNode& e) { break; }
  }
  return activeGID;
}

std::vector<std::string> Aria2::tellWaiting()
{
  MethodCall methodCall("aria2.tellWaiting", m_secret);
  methodCall.newParamValue().set(0);
  methodCall.newParamValue().set(99); // max number of downloads to be returned, don't know how to set this properly assumed that there will not be more than 99 paused downloads.
  auto statusArray = methodCall.newParamValue().getArray();
  statusArray.addValue().set(std::string("gid"));
  auto responseContent = doRequest(methodCall);
  MethodResponse response(responseContent);
  std::vector<std::string> waitingGID;
  int index = 0;
  while(true) {
    try {
      auto structNode = response.getParamValue(0).getArray().getValue(index++).getStruct();
      auto gidNode = structNode.getMember("gid");
      waitingGID.push_back(gidNode.getValue().getAsS());
    } catch (InvalidRPCNode& e) { break; }
  }
  return waitingGID;
}

void Aria2::saveSession()
{
  MethodCall methodCall("aria2.saveSession", m_secret);
  doRequest(methodCall);
  std::cout << "session saved" << std::endl;
}

void Aria2::shutdown()
{
  MethodCall methodCall("aria2.shutdown", m_secret);
  doRequest(methodCall);
}

void Aria2::pause(const std::string& gid)
{
    MethodCall methodCall("aria2.pause", m_secret);
    methodCall.newParamValue().set(gid);
    doRequest(methodCall);
}

void Aria2::unpause(const std::string& gid)
{
    MethodCall methodCall("aria2.unpause", m_secret);
    methodCall.newParamValue().set(gid);
    doRequest(methodCall);
}

void Aria2::remove(const std::string& gid)
{
    MethodCall methodCall("aria2.remove", m_secret);
    methodCall.newParamValue().set(gid);
    doRequest(methodCall);
}

} // end namespace kiwix
