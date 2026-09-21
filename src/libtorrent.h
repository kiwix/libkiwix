/*
 * Copyright 2025 Kiwix developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef KIWIX_LIBTORRENT_H
#define KIWIX_LIBTORRENT_H

#include <memory>
#include <string>

namespace libtorrent
{
struct session;
}

namespace kiwix
{

/**
 * A minimal wrapper class around libtorrent.
 *
 * This is a stub implementation to verify that libkiwix can successfully
 * compile and link against libtorrent library.
 */
class LibTorrent
{
 public:
  LibTorrent();
  ~LibTorrent();

  /**
   * Get the version of libtorrent being used.
   *
   * @return Version string of libtorrent
   */
  std::string getVersion() const;

 private:
  std::unique_ptr<libtorrent::session> mp_session;
};

} // namespace kiwix

#endif // KIWIX_LIBTORRENT_H
