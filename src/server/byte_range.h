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


#ifndef KIWIXLIB_SERVER_BYTE_RANGE_H
#define KIWIXLIB_SERVER_BYTE_RANGE_H

#include <cstdint>
#include <string>

namespace kiwix {

class ByteRange
{
  public: // types
    enum Kind {
      // No byte-range was present in the request
      NONE,

      // The value of the Range header is not a valid continuous range.
      // Note that a valid (according to RFC7233) sequence of byte ranges is
      // considered invalid in this context.
      INVALID,

      // This byte-range has been parsed from request
      PARSED,

      // This is a response to a regular request
      RESOLVED_FULL_CONTENT,

      // This is a response to a range request
      RESOLVED_PARTIAL_CONTENT
    };

  public: // functions
    ByteRange();
    ByteRange(Kind kind, int64_t first, int64_t last);

    Kind kind() const { return kind_; }
    int64_t first() const { return first_; }
    int64_t last() const { return last_; }
    int64_t length() const { return last_ + 1 - first_; }

    static ByteRange parse(const std::string& rangeStr);
    ByteRange resolve(int64_t contentSize) const;

  private: // data
    Kind kind_;
    int64_t first_;
    int64_t last_;
};

} // namespace kiwix

#endif //KIWIXLIB_SERVER_BYTE_RANGE_H
