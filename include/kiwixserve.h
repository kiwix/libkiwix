#ifndef KIWIXLIB_KIWIX_SERVE_H_
#define KIWIXLIB_KIWIX_SERVE_H_

#include <memory>

class Subprocess;
namespace kiwix {

class KiwixServe
{
  public:
    KiwixServe();
    ~KiwixServe();

    void run();
    void shutDown();
    bool isRunning();

  private:
  std::unique_ptr<Subprocess> mp_kiwixServe;
  int m_port;
};

}; //end namespace kiwix

#endif // KIWIXLIB_KIWIX_SERVE_H_
