/*
 * Copyright 2021 Veloman Yunkan <veloman.yunkan@gmail.com>
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

#include "../include/opds_catalog.h"

#include "gtest/gtest.h"

using namespace kiwix;

TEST(OpdsCatalog, getSearchUrl)
{
  #define EXPECT_SEARCH_URL(url) EXPECT_EQ(url, getSearchUrl(f))
  {
    Filter f;
    EXPECT_SEARCH_URL("/catalog/v2/entries");
  }
  {
    Filter f;
    f.query("abc");
    EXPECT_SEARCH_URL("/catalog/v2/entries?q=abc");
  }
  {
    Filter f;
    f.query("abc def#xyz");
    EXPECT_SEARCH_URL("/catalog/v2/entries?q=abc%20def%23xyz");
  }
  {
    Filter f;
    f.category("ted&bob");
    EXPECT_SEARCH_URL("/catalog/v2/entries?category=ted%26bob");
  }
  {
    Filter f;
    f.lang("eng,fra");
    EXPECT_SEARCH_URL("/catalog/v2/entries?lang=eng%2Cfra");
  }
  {
    Filter f;
    f.name("second?");
    EXPECT_SEARCH_URL("/catalog/v2/entries?name=second%3F");
  }
  {
    Filter f;
    f.acceptTags({"#paper", "#plastic"});
    EXPECT_SEARCH_URL("/catalog/v2/entries?tag=%23paper%3B%23plastic");
  }
  {
    Filter f;
    f.query("abc=123");
    f.category("@ted");
    EXPECT_SEARCH_URL("/catalog/v2/entries?q=abc%3D123&category=%40ted");
  }
  {
    Filter f;
    f.category("ted");
    f.query("abc");
    EXPECT_SEARCH_URL("/catalog/v2/entries?q=abc&category=ted");
  }
  {
    Filter f;
    f.query("peru");
    f.category("scifi");
    f.lang("html");
    f.name("edsonarantesdonascimento");
    f.acceptTags({"body", "script"});
    EXPECT_SEARCH_URL("/catalog/v2/entries?q=peru&category=scifi&lang=html&name=edsonarantesdonascimento&tag=body%3Bscript");
  }
  #undef EXPECT_SEARCH_URL
}
