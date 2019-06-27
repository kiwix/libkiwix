#ifndef KIWIXLIB_KIWIX_SERVE_H_
#define KIWIXLIB_KIWIX_SERVE_H_

#include <memory>

class Subprocess;
namespace kiwix {

class KiwixServe
{
  public:
    KiwixServe(int port = 8181);
    ~KiwixServe();

    void run();
    void shutDown();
    bool isRunning();
    int getPort() { return m_port; }

  private:
  std::unique_ptr<Subprocess> mp_kiwixServe;
  int m_port;
};

}; //end namespace kiwix

#endif // KIWIXLIB_KIWIX_SERVE_H_
