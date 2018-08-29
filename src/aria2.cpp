

#include "aria2.h"
#include "xmlrpc.h"
#include <sstream>
#include <thread>
#include <chrono>
#include <common/otherTools.h>
#include <common/pathTools.h>

namespace kiwix {

Aria2::Aria2():
  mp_aria(nullptr),
  m_port(42042),
  m_secret("kiwixariarpc"),
  mp_curl(nullptr),
  m_lock(PTHREAD_MUTEX_INITIALIZER)
{
  m_downloadDir = getDataDirectory();
  std::vector<const char*> callCmd;

  std::string rpc_port = "--rpc-listen-port=" + std::to_string(m_port);
  std::string download_dir = "--dir=" + getDataDirectory();
//  std::string log_dir = "--log=\"" + logDir + "\"";
#ifdef _WIN32
  int pid = GetCurrentProcessId();
#else
  pid_t pid = getpid();
#endif
  std::string stop_with_pid = "--stop-with-process=" + std::to_string(pid);
  std::string rpc_secret = "--rpc-secret=" + m_secret;
  m_secret = "token:"+m_secret;

  callCmd.push_back("aria2c");
  callCmd.push_back("--enable-rpc");
  callCmd.push_back(rpc_port.c_str());
  callCmd.push_back(download_dir.c_str());
//  callCmd.push_back(log_dir.c_str());
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
  mp_aria = Subprocess::run(callCmd);
  mp_curl = curl_easy_init();
  curl_easy_setopt(mp_curl, CURLOPT_URL, "http://localhost/rpc");
  curl_easy_setopt(mp_curl, CURLOPT_PORT, m_port);
  curl_easy_setopt(mp_curl, CURLOPT_POST, 1L);

  while(true) {
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    auto res = curl_easy_perform(mp_curl);
    if (res == CURLE_OK) {
      break;
    }
  }
}

Aria2::~Aria2()
{
  curl_easy_cleanup(mp_curl);
};

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
    if (response_code == 200) {
      return stringstream.str();
    }
  }
  pthread_mutex_unlock(&m_lock);
  return "";
}

std::string Aria2::addUri(const std::vector<std::string>& uris)
{
  MethodCall methodCall("aria2.addUri");
  methodCall.getParams().addParam().getValue().set(m_secret);
  auto uriParams = methodCall.getParams().addParam().getValue().getArray();
  for (auto& uri : uris) {
    uriParams.addValue().set(uri);
  }
  auto ret = doRequest(methodCall);
  MethodResponse response(ret);
  try {
    return response.getParams().getParam(0).getValue().getAsS();
  } catch (InvalidRPCNode& err) {
    std::cerr << response.getFault().getFaultString();
  }
  return "";
}

std::string Aria2::tellStatus(const std::string& gid, const std::vector<std::string>& statusKey)
{
  MethodCall methodCall("aria2.tellStatus");
  methodCall.getParams().addParam().getValue().set(m_secret);
  methodCall.getParams().addParam().getValue().set(gid);
  if (!statusKey.empty()) {
    auto statusArray = methodCall.getParams().addParam().getValue().getArray();
    for (auto& key : statusKey) {
      statusArray.addValue().set(key);
    }
  }
  return doRequest(methodCall);
}

} // end namespace kiwix
