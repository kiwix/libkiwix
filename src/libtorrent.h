

#ifndef KIWIXLIB_LIBTORRENT_H_
#define KIWIXLIB_LIBTORRENT_H_

#include <memory>
#include <string>

namespace libtorrent
{
struct session;
}

namespace kiwix
{

/**
 * @brief A minimal wrapper class around libtorrent-rasterbar
 *
 * This is a stub implementation to verify that libkiwix can successfully
 * compile and link against libtorrent-rasterbar library.
 */
class LibTorrent
{
 private:
  std::unique_ptr<libtorrent::session> m_session;

 public:
  LibTorrent();
  virtual ~LibTorrent();

  /**
   * @brief Get the version of libtorrent being used
   * @return Version string of libtorrent-rasterbar
   */
  std::string getVersion() const;
};

}  // end namespace kiwix

#endif  // KIWIXLIB_LIBTORRENT_H_
