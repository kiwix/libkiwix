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

#ifndef KIWIX_SERVER_I18N
#define KIWIX_SERVER_I18N

#include <string>
#include <mustache.hpp>

namespace kiwix
{

struct I18nString {
  const char* const key;
  const char* const value;
};

struct I18nStringTable {
  const char* const lang;
  const size_t entryCount;
  const I18nString* const entries;

  const char* get(const std::string& key) const;
};

std::string getTranslatedString(const std::string& lang, const std::string& key);

namespace i18n
{

typedef kainjow::mustache::object Parameters;

std::string expandParameterizedString(const std::string& lang,
                                      const std::string& key,
                                      const Parameters& params);

} // namespace i18n

struct ParameterizedMessage
{
public: // types
  typedef kainjow::mustache::object Parameters;

public: // functions
  ParameterizedMessage(const std::string& msgId, const Parameters& params)
    : msgId(msgId)
    , params(params)
  {}

  std::string getText(const std::string& lang) const;

private: // data
  const std::string msgId;
  const Parameters  params;
};

} // namespace kiwix

#endif // KIWIX_SERVER_I18N
