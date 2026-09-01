/*
 * Copyright 2014 Emmanuel Engelhart <kelson@kiwix.org>
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

#ifndef KIWIX_OTHERTOOLS_H
#define KIWIX_OTHERTOOLS_H

#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <zim/zim.h>
#include <mustache.hpp>

#include "stringTools.h"

namespace pugi {
  class xml_node;
}

namespace zim {
  class SuggestionItem;
}


namespace kiwix
{
  /**
   * The pieces needed to resolve references found in OPDS content, split out
   * of the location that content was read from. See resolveContentOrigin().
   */
  struct ContentOrigin
  {
    std::string urlHost; /**< scheme+host[:port], no trailing slash; "" if the origin is a local path */
    std::string baseDir; /**< parent directory of a local path; "" if the origin is a URL */
  };

  /** Split a "content origin" locator into a urlHost/baseDir pair.
   *
   * contentOriginUri is either a URL that some content was fetched from (e.g.
   * "https://library.kiwix.org/catalog/v2/entries") or a local filesystem path
   * that content was read from (e.g. "/data/library.opds"). Classification is
   * purely syntactic: a "://" substring marks the input as a URL; everything
   * else (including a Windows drive-letter path) is treated as a local path.
   * Exactly one of the two returned fields is non-empty.
   *
   * Known limitation: a "file://" URI is classified as a URL, not a local
   * path (no current caller produces one).
   *
   * @param contentOriginUri the URL or local path some content was read from.
   * @return the corresponding urlHost/baseDir pair.
   */
  ContentOrigin resolveContentOrigin(const std::string& contentOriginUri);

  std::string nodeToString(const pugi::xml_node& node);

  /*
   * Convert all format tag string to new format
   */
  std::vector<std::string> convertTags(const std::string& tags_str);
  std::string getTagValueFromTagList(const std::vector<std::string>& tagList,
                                     const std::string& tagName);
  bool convertStrToBool(const std::string& value);

  std::string gen_date_str();
  std::string gen_uuid(const std::string& s);

  // if s is empty then returns kainjow::mustache::data(false)
  // otherwise kainjow::mustache::data(value)
  kainjow::mustache::data onlyAsNonEmptyMustacheValue(const std::string& s);

  std::string render_template(const std::string& template_str, kainjow::mustache::data data);

  template<typename T>
  T getEnvVar(const char* name, const T& defaultValue)
  {
    try {
      const char* envString = std::getenv(name);
      if (envString == nullptr) {
        throw std::runtime_error("Environment variable not set");
      }
      return extractFromString<T>(envString);
    } catch (...) {}

    return defaultValue;
  }

  class Suggestions
  {
  public:
    Suggestions();

    void add(const zim::SuggestionItem& suggestion);

    void addFTSearchSuggestion(const std::string& uiLang,
                               const std::string& query);

    std::string getJSON() const;

  private:
    kainjow::mustache::data m_data;
  };
}

#endif
