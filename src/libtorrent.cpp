

#include "libtorrent.h"

#include <libtorrent/session.hpp>
#include <libtorrent/version.hpp>

namespace kiwix
{

LibTorrent::LibTorrent() : m_session(new libtorrent::session())
{
  // Create a basic libtorrent session
  // This ensures we can successfully link against libtorrent
}

LibTorrent::~LibTorrent()
{
  // Cleanup is handled automatically by unique_ptr
}

std::string LibTorrent::getVersion() const
{
  return LIBTORRENT_VERSION;
}

}  // end namespace kiwix
