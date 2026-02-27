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

#include "libtorrent.h"

#include <libtorrent/session.hpp>
#include <libtorrent/version.hpp>

namespace kiwix
{

LibTorrent::LibTorrent()
  : mp_session(new libtorrent::session())
{
}

LibTorrent::~LibTorrent() = default;

std::string LibTorrent::getVersion() const
{
  return LIBTORRENT_VERSION;
}

} // namespace kiwix
