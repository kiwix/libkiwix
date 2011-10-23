/*
 * Copyright 2011 Emmanuel Engelhart <kelson@kiwix.org>
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

#include "regexTools.h"

std::map<std::string, regex_t> regexCache;

regex_t buildRegex(const std::string &regex) {
  regex_t regexStruct;
  std::map<std::string, regex_t>::iterator itr = regexCache.find(regex);
  
  /* Regex is in cache */
  if (itr != regexCache.end()) {
    regexStruct = itr->second;
  }

  /* Regex needs to be parsed (and cached) */
  else {
    regcomp(&regexStruct, regex.data(), REG_ICASE);
    regexCache[regex] = regexStruct;
  }

  return regexStruct;
}

/* todo */
void freeRegexCache() {
  //regfree(&regexStructure);
}

bool matchRegex(const std::string &content, const std::string &regex) {
  regex_t regexStructure = buildRegex(regex);
  bool result = !regexec(&regexStructure, content.data(), 0, 0, 0);
  return result;
}
