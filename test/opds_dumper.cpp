/*
 * Copyright 2021 Matthieu Gautier <mgautier@kymeria.fr>
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

#include "../include/opds_dumper.h"

#include "gtest/gtest.h"

namespace kiwix {
  namespace internal {
    std::string getLanguageSelfName(const std::string& lang);
    void fillLanguagesMap();
  }
}

TEST(OpdsDumper, getLanguageSelfName)
{
  // A lang not in icu (already in iso639_3 map)
  EXPECT_EQ(kiwix::internal::getLanguageSelfName("dty"), "डोटेली");
  // A lang added by icu
  EXPECT_EQ(kiwix::internal::getLanguageSelfName("fra"), "fra");
  kiwix::internal::fillLanguagesMap();
  // A lang not in icu (already in iso639_3 map)
  EXPECT_EQ(kiwix::internal::getLanguageSelfName("dty"), "डोटेली");
  // A lang added by icu
  EXPECT_EQ(kiwix::internal::getLanguageSelfName("fra"), "français");
}
