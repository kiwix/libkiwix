/*
 * Copyright 2026 Veloman Yunkan
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

#ifndef LIBKIWIX_TESTING_TOOLS_H
#define LIBKIWIX_TESTING_TOOLS_H

#include <sstream>
#include <iostream>
#include <regex>

namespace kiwix
{

namespace testing
{

class CapturedStderr
{
  std::ostringstream buffer;
  std::streambuf* const sbuf;
public:
  CapturedStderr()
    : sbuf(std::cerr.rdbuf())
  {
    std::cerr.rdbuf(buffer.rdbuf());
  }

  CapturedStderr(const CapturedStderr&) = delete;

  ~CapturedStderr()
  {
    std::cerr.rdbuf(sbuf);
  }

  operator std::string() const { return buffer.str(); }
};

// Returns a copy of 'text' where every line that fully matches 'pattern'
// (preceded by optional whitespace) is replaced with the fixed string
// 'replacement' preserving the leading whitespace
inline std::string replaceLines(const std::string& text,
                         const std::string& pattern,
                         const std::string& replacement)
{
  std::regex regex("^ *" + pattern + "$");
  std::ostringstream oss;
  std::istringstream iss(text);
  std::string line;
  while ( std::getline(iss, line) ) {
    if ( std::regex_match(line, regex) ) {
      for ( size_t i = 0; i < line.size() && line[i] == ' '; ++i )
        oss << ' ';
      oss << replacement << "\n";
    } else {
      oss << line << "\n";
    }
  }
  return oss.str();
}

// Masks every <updated>...</updated> timestamp - both the feed-level one
// (always "now", via gen_date_str()) and the per-entry/per-category/
// per-language ones used by categoriesOPDSFeed()/languagesOPDSFeed() (also
// "now" - see LibraryDumper::getCategoryData()/getLanguageData()). Per-book
// entries elsewhere carry a real, deterministic book date, but this matches
// them too, same as library_server.cpp's CATALOG_ENTRY does for the live
// HTTP catalog - the trade-off of a slightly less precise assertion in
// exchange for a much simpler, more robust one.
inline std::string maskUpdatedTimestampsInOPDS(std::string s)
{
  return replaceLines(s, R"(<updated>\d\d\d\d-\d\d-\d\dT\d\d:\d\d:\d\dZ</updated>)",
                          "<updated>YYYY-MM-DDThh:mm:ssZ</updated>");
}

inline std::string maskVariableOPDSFeedData(std::string s)
{
  s = maskUpdatedTimestampsInOPDS(s);
  s = replaceLines(s, "<id>[[:xdigit:]]{8}-[[:xdigit:]]{4}-[[:xdigit:]]{4}-[[:xdigit:]]{4}-[[:xdigit:]]{12}</id>",
                      "<id>12345678-90ab-cdef-1234-567890abcdef</id>");
  return s;
}

} // namespace testing

} // namespace kiwix

#endif // LIBKIWIX_TESTING_TOOLS_H
