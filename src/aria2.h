

#ifndef KIWIXLIB_ARIA2_H_
#define KIWIXLIB_ARIA2_H_

#ifdef _WIN32
// winsock2.h need to be included before windows.h (included by curl.h)
# include <winsock2.h>
#endif

#include "subprocess.h"
#include "xmlrpc.h"

#include <memory>
#include <curl/curl.h>

namespace kiwix {

class Aria2
{
  private:
    std::unique_ptr<Subprocess> mp_aria;
    int m_port;
    std::string m_secret;
    std::string m_downloadDir;
    CURL* mp_curl;
    pthread_mutex_t m_lock;

    std::string doRequest(const MethodCall& methodCall);

  public:
    Aria2();
    virtual ~Aria2();
    void close();

    std::string addUri(const std::vector<std::string>& uri);
    std::string tellStatus(const std::string& gid, const std::vector<std::string>& statusKey);
    std::vector<std::string> tellActive();
    void saveSession();
    void shutdown();
};

}; //end namespace kiwix

#endif // KIWIXLIB_ARIA2_H_
