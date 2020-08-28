

#ifndef KIWIXLIB_ARIA2_H_
#define KIWIXLIB_ARIA2_H_

#ifdef _WIN32
// winsock2.h need to be included before windows.h (included by curl.h)
# include <winsock2.h>
#endif

#include "subprocess.h"
#include "xmlrpc.h"

#include <memory>
#include <mutex>
#include <curl/curl.h>

namespace kiwix {

class Aria2
{
  private:
    std::unique_ptr<Subprocess> mp_aria;
    int m_port;
    std::string m_secret;
    std::string m_downloadDir;
    std::unique_ptr<char[]> m_curlErrorBuffer;
    CURL* mp_curl;
    std::mutex m_lock;

    std::string doRequest(const MethodCall& methodCall);

  public:
    Aria2();
    virtual ~Aria2();
    void close();

    std::string addUri(const std::vector<std::string>& uri, const std::vector<std::pair<std::string, std::string>>& options = {});
    std::string tellStatus(const std::string& gid, const std::vector<std::string>& statusKey);
    std::vector<std::string> tellActive();
    std::vector<std::string> tellWaiting();
    void saveSession();
    void shutdown();
    void pause(const std::string& gid);
    void unpause(const std::string& gid);
    void remove(const std::string& gid);
};

}; //end namespace kiwix

#endif // KIWIXLIB_ARIA2_H_
