/*
 * Copyright 2022 Veloman Yunkan <veloman.yunkan@gmail.com>
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

#include "i18n.h"

#include <algorithm>
#include <map>

namespace kiwix
{

const char* I18nStringTable::get(const std::string& key) const
{
  const I18nString* const begin = entries;
  const I18nString* const end = begin + entryCount;
  const I18nString* found = std::lower_bound(begin, end, key,
      [](const I18nString& a, const std::string& k) {
        return a.key < k;
  });
  return (found == end || found->key != key) ? nullptr : found->value;
}

namespace
{

const I18nString enStrings[] = {
  // must be sorted by key
  { "suggest-full-text-search", "containing '{{{SEARCH_TERMS}}}'..."}
};

#define ARRAY_ELEMENT_COUNT(a) (sizeof(a)/sizeof(a[0]))

const I18nStringTable i18nStringTables[] = {
  { "en", ARRAY_ELEMENT_COUNT(enStrings), enStrings }
};

class I18nStringDB
{
public: // functions
  I18nStringDB() {
    for ( size_t i = 0; i < ARRAY_ELEMENT_COUNT(i18nStringTables); ++i ) {
      const auto& t = i18nStringTables[i];
      lang2TableMap[t.lang] = &t;
    }
    enStrings = lang2TableMap.at("en");
  };

  std::string get(const std::string& lang, const std::string& key) const {
    const char* s = getStringsFor(lang)->get(key);
    if ( s == nullptr ) {
      s = enStrings->get(key);
      if ( s == nullptr ) {
        throw std::runtime_error("Invalid message id");
      }
    }
    return s;
  }

private: // functions
  const I18nStringTable* getStringsFor(const std::string& lang) const {
    try {
      return lang2TableMap.at(lang);
    } catch(const std::out_of_range&) {
      return enStrings;
    }
  }

private: // data
  std::map<std::string, const I18nStringTable*> lang2TableMap;
  const I18nStringTable* enStrings;
};

} // unnamed namespace

std::string getTranslatedString(const std::string& lang, const std::string& key)
{
  static const I18nStringDB stringDb;

  return stringDb.get(lang, key);
}

} // namespace kiwix
