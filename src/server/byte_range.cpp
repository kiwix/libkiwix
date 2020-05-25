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


#include "byte_range.h"

#include "tools/stringTools.h"

namespace kiwix {

ByteRange::ByteRange()
  : kind_(NONE)
  , first_(0)
  , last_(INT64_MAX)
{}

ByteRange::ByteRange(Kind kind, int64_t first, int64_t last)
  : kind_(kind)
  , first_(first)
  , last_(last)
{}

ByteRange ByteRange::parse(std::string rangeStr)
{
  ByteRange byteRange(INVALID, 0, INT64_MAX);
  const std::string byteUnitSpec("bytes=");
  if ( kiwix::startsWith(rangeStr, byteUnitSpec) ) {
    rangeStr.erase(0, byteUnitSpec.size());
    std::istringstream iss(rangeStr);

    int64_t start, end = INT64_MAX;
    if (iss >> start) {
      if ( start < 0 ) {
        if ( iss.eof() )
          byteRange = ByteRange(PARSED, start, end);
      } else {
        char c;
        if (iss >> c && c=='-') {
          iss >> end; // if this fails, end is not modified, which is OK
          if (iss.eof() && start <= end)
            byteRange = ByteRange(PARSED, start, end);
        }
      }
    }
  }

  return byteRange;
}

ByteRange ByteRange::resolve(int64_t contentSize) const
{
  if ( kind() == NONE )
    return ByteRange(RESOLVED_FULL_CONTENT, 0, contentSize-1);

  if ( kind() == INVALID )
    return ByteRange(INVALID, 0, contentSize-1);

  const int64_t resolved_first = first() < 0
                               ? std::max(int64_t(0), contentSize + first())
                               : first();

  const int64_t resolved_last = std::min(contentSize-1, last());

  return ByteRange(RESOLVED_PARTIAL_CONTENT, resolved_first, resolved_last);
}

} // namespace kiwix
