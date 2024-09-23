/*
 * Copyright (C) 2022 Veloman Yunkan
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include "gtest/gtest.h"
#include "../include/tools.h"
#include "../src/tools/otherTools.h"
#include "zim/suggestion_iterator.h"
#include "../src/server/i18n_utils.h"

#include <regex>

namespace
{

// Output generated via mustache templates sometimes contains end-of-line
// whitespace. This complicates representing the expected output of a unit-test
// as C++ raw strings in editors that are configured to delete EOL whitespace.
// A workaround is to put special markers (//EOLWHITESPACEMARKER) at the end
// of such lines in the expected output string and remove them at runtime.
// This is exactly what this function is for.
std::string removeEOLWhitespaceMarkers(const std::string& s)
{
  const std::regex pattern("//EOLWHITESPACEMARKER");
  return std::regex_replace(s, pattern, "");
}

} // unnamed namespace

#define CHECK_SUGGESTIONS(actual, expected) \
        EXPECT_EQ(actual, removeEOLWhitespaceMarkers(expected))

TEST(Suggestions, basicTest)
{
  kiwix::Suggestions s;
  CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  //EOLWHITESPACEMARKER
]
)EXPECTEDJSON"
  );

  s.add(zim::SuggestionItem("Title", "/PATH", "Snippet"));

  CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "Title",
    "label" : "Snippet",
    "kind" : "path"
      , "path" : "/PATH"
  }
]
)EXPECTEDJSON"
  );

  s.add(zim::SuggestionItem("Title Without Snippet", "/P/a/t/h"));
  s.addFTSearchSuggestion("en", "kiwi");

  CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "Title",
    "label" : "Snippet",
    "kind" : "path"
      , "path" : "/PATH"
  },
  {
    "value" : "Title Without Snippet",
    "label" : "Title Without Snippet",
    "kind" : "path"
      , "path" : "/P/a/t/h"
  },
  {
    "value" : "kiwi ",
    "label" : "containing &apos;kiwi&apos;...",
    "kind" : "pattern"
    //EOLWHITESPACEMARKER
  }
]
)EXPECTEDJSON"
  );
}

TEST(Suggestions, specialCharHandling)
{
  // HTML special symbols (<, >, &, ", and ') must be HTML-escaped
  // Backslash symbols (\) must be duplicated.
  const std::string SYMBOLS("\t\n\r" R"(\<>&'"~!@#$%^*()_+`-=[]{}|:;,.?)");
  {
    kiwix::Suggestions s;
    s.add(zim::SuggestionItem("Title with "   + SYMBOLS,
                              "Path with "    + SYMBOLS,
                              "Snippet with " + SYMBOLS));

    CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "Title with \t\n\r\\&lt;&gt;&amp;&apos;&quot;~!@#$%^*()_+`-=[]{}|:;,.?",
    "label" : "Snippet with \t\n\r\\&lt;&gt;&amp;&apos;&quot;~!@#$%^*()_+`-=[]{}|:;,.?",
    "kind" : "path"
      , "path" : "Path with \t\n\r\\&lt;&gt;&amp;&apos;&quot;~!@#$%^*()_+`-=[]{}|:;,.?"
  }
]
)EXPECTEDJSON"
    );
  }

  {
    kiwix::Suggestions s;
    s.add(zim::SuggestionItem("Snippetless title with " + SYMBOLS,
                              "Path with " + SYMBOLS));

    CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "Snippetless title with \t\n\r\\&lt;&gt;&amp;&apos;&quot;~!@#$%^*()_+`-=[]{}|:;,.?",
    "label" : "Snippetless title with \t\n\r\\&lt;&gt;&amp;&apos;&quot;~!@#$%^*()_+`-=[]{}|:;,.?",
    "kind" : "path"
      , "path" : "Path with \t\n\r\\&lt;&gt;&amp;&apos;&quot;~!@#$%^*()_+`-=[]{}|:;,.?"
  }
]
)EXPECTEDJSON"
    );
  }

  {
    kiwix::Suggestions s;
    s.addFTSearchSuggestion("eng", "text with " + SYMBOLS);

    CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "text with \t\n\r\\&lt;&gt;&amp;&apos;&quot;~!@#$%^*()_+`-=[]{}|:;,.? ",
    "label" : "containing &apos;text with \t\n\r\\&lt;&gt;&amp;&apos;&quot;~!@#$%^*()_+`-=[]{}|:;,.?&apos;...",
    "kind" : "pattern"
    //EOLWHITESPACEMARKER
  }
]
)EXPECTEDJSON"
    );
  }
}

TEST(Suggestions, fulltextSearchSuggestionIsTranslated)
{
  kiwix::Suggestions s;
  s.addFTSearchSuggestion("it", "kiwi");

  CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "kiwi ",
    "label" : "contenente &apos;kiwi&apos;...",
    "kind" : "pattern"
    //EOLWHITESPACEMARKER
  }
]
)EXPECTEDJSON"
  );
}

std::string toString(const kiwix::LangPreference& x)
{
  std::ostringstream oss;
  oss << "{" << x.lang << ", " << x.preference << "}";
  return oss.str();
}

std::string toString(const kiwix::UserLangPreferences& prefs) {
  std::ostringstream oss;
  for ( const auto& x : prefs )
    oss << toString(x);
  return oss.str();
}

TEST(I18n, parseUserLanguagePreferences)
{
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("")),
      ""
  );
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("*")),
      "{*, 1}"
  );
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("fr")),
      "{fr, 1}"
  );
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("fr-CH")),
      "{fr-CH, 1}"
  );
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("fr, en-US")),
      "{fr, 1}{en-US, 1}"
  );
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("ru;q=0.5")),
      "{ru, 0.5}"
  );
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("fr-CH,ru;q=0.5")),
      "{fr-CH, 1}{ru, 0.5}"
  );
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("ru;q=0.5, *;q=0.1")),
      "{ru, 0.5}{*, 0.1}"
  );

  // rejected input
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("ru;")),
      ""
  );
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("ru;q")),
      ""
  );
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("ru;q=")),
      ""
  );
  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("ru;0.8")),
      ""
  );

  EXPECT_EQ(toString(kiwix::parseUserLanguagePreferences("fr,ru;0.8,en;q=0.5")),
      "{fr, 1}{en, 0.5}"
  );
}

#include "../include/tools.h"

TEST(networkTools, getNetworkInterfacesIPv4Or6)
{
  for ( const auto& kv : kiwix::getNetworkInterfacesIPv4Or6() ) {
    std::cout << kv.first << " : IPv4 addr = " << kv.second.addr
                          << " ; IPv6 addr = " << kv.second.addr6
                          << std::endl;
  }
}

TEST(networkTools, getNetworkInterfaces)
{
  for ( const auto& kv : kiwix::getNetworkInterfaces() ) {
    std::cout << kv.first << " : IPv4 addr = " << kv.second << std::endl;
  }
}

TEST(networkTools, getBestPublicIps)
{
  std::cout << "getBestPublicIps(): " << "[" << kiwix::getBestPublicIps().addr << ", " << kiwix::getBestPublicIps().addr6 << "]" << std::endl;
  std::cout << "getBestPublicIp(): " << kiwix::getBestPublicIp() << std::endl;
}
