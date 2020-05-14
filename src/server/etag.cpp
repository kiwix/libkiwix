/*
 * Copyright 2020 Veloman Yunkan <veloman.yunkan@gmail.com>
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


#include "etag.h"

#include <algorithm>

namespace kiwix {

namespace {

// Characters in the options part of the ETag could in principle be picked up
// from the latin alphabet in natural order (the character corresponding to
// ETag::Option opt would be 'a'+opt; that would somewhat simplify the code in
// this file). However it is better to have some mnemonics in the option names,
// hence below variable: all_options[opt] corresponds to the character going
// into the ETag for ETag::Option opt.
const char all_options[] = "cz";

static_assert(ETag::OPTION_COUNT == sizeof(all_options) - 1, "");

} // namespace


void ETag::set_option(Option opt)
{
  if ( ! get_option(opt) )
  {
    m_options.push_back(all_options[opt]);
    std::sort(m_options.begin(), m_options.end());
  }
}

bool ETag::get_option(Option opt) const
{
  return m_options.find(all_options[opt]) != std::string::npos;
}

std::string ETag::get_etag() const
{
  if ( m_serverId.empty() )
    return std::string();

  return "\"" + m_serverId + "/" + m_options + "\"";
}

} // namespace kiwix
