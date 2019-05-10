#ifndef KIWIXLIB_KIWIX_SERVE_H_
#define KIWIXLIB_KIWIX_SERVE_H_

#ifdef _WIN32
// winsock2.h need to be included before windows.h (included by curl.h)
# include <winsock2.h>
#endif

#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include "tools/pathTools.h"

class Subprocess;
namespace kiwix {

class KiwixServe
{
  public:
    KiwixServe();
    ~KiwixServe();

    void run();
    void shutDown();

  private:
  std::unique_ptr<Subprocess> mp_kiwixServe;
  int m_port;
};

}; //end namespace kiwix

#endif // KIWIXLIB_KIWIX_SERVE_H_
