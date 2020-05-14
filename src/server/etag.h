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


#ifndef KIWIXLIB_SERVER_ETAG_H
#define KIWIXLIB_SERVER_ETAG_H

#include <string>

namespace kiwix {

// The ETag string used by Kiwix server (more precisely, its value inside the
// double quotes) consists of two parts:
//
// 1. ServerId - The string obtained on server start up
//
// 2. Options -  Zero or more characters encoding the values of some of the
//               headers of the response
//
// The two parts are separated with a slash (/) symbol (which is always present,
// even when the the options part is empty). Neither portion of a Kiwix ETag
// may contain the slash symbol.
// Examples of valid Kiwix server ETags (including the double quotes):
//
//   "abcdefghijklmn/"
//   "1234567890/z"
//   "1234567890/cz"
//
// The options part of the Kiwix ETag allows to correctly set the required
// headers when responding to a conditional If-None-Match request with a 304
// (Not Modified) response without following the full code path that would
// discover the necessary options.

class ETag
{
  public: // types
    enum Option {
      CACHEABLE_ENTITY,
      COMPRESSED_CONTENT,
      OPTION_COUNT
    };

  public: // functions
    ETag() {}

    void set_server_id(const std::string& id) { m_serverId = id; }
    void set_option(Option opt);

    explicit operator bool() const { return !m_serverId.empty(); }

    bool get_option(Option opt) const;
    std::string get_etag() const;


    static ETag match(const std::string& etags, const std::string& server_id);

  private: // functions
    ETag(const std::string& serverId, const std::string& options);

    static ETag parse(std::string s);

  private: // data
    std::string m_serverId;
    std::string m_options;
};

} // namespace kiwix

#endif // KIWIXLIB_SERVER_ETAG_H
