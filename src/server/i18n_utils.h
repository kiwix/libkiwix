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

#ifndef KIWIX_SERVER_I18N_UTILS
#define KIWIX_SERVER_I18N_UTILS

#include <map>
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

typedef std::map<std::string, std::string> Parameters;

std::string expandParameterizedString(const std::string& lang,
                                      const std::string& key,
                                      const Parameters& params);

class GetTranslatedString
{
public:
  explicit GetTranslatedString(const std::string& lang) : m_lang(lang) {}

  std::string operator()(const std::string& key) const
  {
    return getTranslatedString(m_lang, key);
  }

  std::string operator()(const std::string& key, const Parameters& params) const
  {
    return expandParameterizedString(m_lang, key, params);
  }

private:
  const std::string m_lang;
};

class GetTranslatedStringWithMsgId
{
  typedef kainjow::mustache::basic_data<std::string> MustacheString;
  typedef std::pair<std::string, MustacheString> MsgIdAndTranslation;

public:
  explicit GetTranslatedStringWithMsgId(const std::string& lang) : m_lang(lang) {}

  MsgIdAndTranslation operator()(const std::string& key) const
  {
    return {key, getTranslatedString(m_lang, key)};
  }

  MsgIdAndTranslation operator()(const std::string& key, const Parameters& params) const
  {
    return {key, expandParameterizedString(m_lang, key, params)};
  }

private:
  const std::string m_lang;
};

} // namespace i18n

class ParameterizedMessage
{
public: // types
  typedef i18n::Parameters Parameters;

public: // functions
  ParameterizedMessage(const std::string& msgId, const Parameters& params)
    : msgId(msgId)
    , params(params)
  {}

  std::string getText(const std::string& lang) const;

  const std::string& getMsgId()  const { return msgId; }
  const Parameters&  getParams() const { return params; }

private: // data
  const std::string msgId;
  const Parameters  params;
};

inline ParameterizedMessage nonParameterizedMessage(const std::string& msgId)
{
  const ParameterizedMessage::Parameters noParams;
  return ParameterizedMessage(msgId, noParams);
}

struct LangPreference
{
  const std::string lang;
  const float preference;
};

typedef std::vector<LangPreference> UserLangPreferences;

UserLangPreferences parseUserLanguagePreferences(const std::string& s);

std::string selectMostSuitableLanguage(const UserLangPreferences& prefs);

} // namespace kiwix

#endif // KIWIX_SERVER_I18N_UTILS
