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

namespace kiwix {

class ByteRange
{
  public: // functions
    ByteRange() : ByteRange(0, -1) {}
    ByteRange(int64_t first, int64_t last) : first_(first), last_(last) {}

    int64_t first() const { return first_; }
    int64_t last() const { return last_; }
    int64_t length() const { return last_ + 1 - first_; }

  private: // data
    int64_t first_;
    int64_t last_;
};

} // namespace kiwix

#endif //KIWIXLIB_SERVER_BYTE_RANGE_H
