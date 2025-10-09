/*
 * Copyright (C) 2025 Veloman Yunkan
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
#include "../include/spelling_correction.h"
#include "../src/tools/pathTools.h"
#include "zim/archive.h"

#include <filesystem>

#include <xapian.h>

const std::string TEST_DB_PATH = "./spellings.db";

class SpellingCorrectionTest : public ::testing::Test
{
protected:
  void SetUp() override {
    tmpDirPath = makeTmpDirectory();
    archive = std::make_unique<zim::Archive>("./test/spelling_correction_test.zim");
  }

  void TearDown() override {
    std::filesystem::remove_all(tmpDirPath);
  }

protected:
  std::filesystem::path tmpDirPath;
  std::unique_ptr<zim::Archive> archive;
};

void testSpellingCorrections(const kiwix::SpellingsDB& spellingsDB)
{
#define EXPECT_SPELLING_CORRECTION(query, maxSuggestions, parenthesizedExpectedResult) \
  EXPECT_EQ(                                                                   \
      spellingsDB.getSpellingCorrections(query, maxSuggestions),               \
      std::vector<std::string> parenthesizedExpectedResult                     \
  )

  EXPECT_SPELLING_CORRECTION("", 1, ({}));

  EXPECT_SPELLING_CORRECTION("geflekt", 1, ({"gefleckt"}));
  EXPECT_SPELLING_CORRECTION("Teler", 1, ({"Teller"}));
  EXPECT_SPELLING_CORRECTION("Teler", 1, ({"Teller"}));
  EXPECT_SPELLING_CORRECTION("kämen", 1, ({"kämmen"}));
  EXPECT_SPELLING_CORRECTION("abonieren", 1, ({"abonnieren"}));
  EXPECT_SPELLING_CORRECTION("abbonnieren", 1, ({"abonnieren"}));
  EXPECT_SPELLING_CORRECTION("abbonieren", 1, ({"abonnieren"}));
  EXPECT_SPELLING_CORRECTION("Aplaus", 1, ({"Applaus"}));
  EXPECT_SPELLING_CORRECTION("konkurieren", 1, ({"konkurrieren"}));
  EXPECT_SPELLING_CORRECTION("Asisstent", 1, ({"Assistent"}));
  EXPECT_SPELLING_CORRECTION("Assisstent", 1, ({"Assistent"}));
  EXPECT_SPELLING_CORRECTION("Atacke", 1, ({"Attacke"}));
  EXPECT_SPELLING_CORRECTION("atestieren", 1, ({"attestieren"}));
  EXPECT_SPELLING_CORRECTION("entäuschen", 1, ({"enttäuschen"}));
  EXPECT_SPELLING_CORRECTION("Enzündung", 1, ({"Entzündung"}));
  EXPECT_SPELLING_CORRECTION("Schirmütze", 1, ({"Schirmmütze"}));
  EXPECT_SPELLING_CORRECTION("Termoskanne", 1, ({"Thermoskanne"}));
  EXPECT_SPELLING_CORRECTION("Tsunge", 1, ({"Zunge"}));
  EXPECT_SPELLING_CORRECTION("vort", 1, ({"fort"}));
  EXPECT_SPELLING_CORRECTION("Schtuhl", 1, ({"Stuhl"}));
  EXPECT_SPELLING_CORRECTION("beissen", 1, ({"beißen"}));
  EXPECT_SPELLING_CORRECTION("Camera", 1, ({"Kamera"}));
  EXPECT_SPELLING_CORRECTION("Kaos", 1, ({"Chaos"}));

  // The spelling correction "Lax -> Lachs" is affected by commit
  // https://github.com/xapian/xapian/commit/0cbe35de5c392623388946e6769aa03f912fdde4
  // which caps the edit distance at (length(query_word) - 1). As a result, the
  // max edit distance parameter that we pass into get_spelling_suggestion() is
  // reduced from 3 to 2 and is below the edit distance of "Lachs" from "Lax".
  const auto xapianVersion = std::make_tuple(Xapian::major_version(),
                                             Xapian::minor_version(),
                                             Xapian::revision());
  if ( xapianVersion < std::make_tuple(1, 4, 19) ) {
    EXPECT_SPELLING_CORRECTION("Lax", 1, ({"Lachs"}));
  } else {
    EXPECT_SPELLING_CORRECTION("Lax", 1, ({}));
  }

  EXPECT_SPELLING_CORRECTION("Mont", 1, ({"Mond"}));
  EXPECT_SPELLING_CORRECTION("Umweltstandart", 1, ({"Umweltstandard"}));
  EXPECT_SPELLING_CORRECTION("seid", 1, ({"seit"}));
  EXPECT_SPELLING_CORRECTION("Trok", 1, ({"Trog"}));
  EXPECT_SPELLING_CORRECTION("Unfuk", 1, ({"Unfug"}));
  EXPECT_SPELLING_CORRECTION("schupsen", 1, ({"schubsen"}));
  EXPECT_SPELLING_CORRECTION("warscheinlich", 1, ({"wahrscheinlich"}));
  EXPECT_SPELLING_CORRECTION("gefärlich", 1, ({"gefährlich"}));
  EXPECT_SPELLING_CORRECTION("Son", 1, ({"Sohn"}));
  EXPECT_SPELLING_CORRECTION("nähmlich", 1, ({"nämlich"}));
  EXPECT_SPELLING_CORRECTION("Grahl", 1, ({"Gral"}));
  EXPECT_SPELLING_CORRECTION("Bine", 1, ({"Biene"}));
  EXPECT_SPELLING_CORRECTION("Hirarchie", 1, ({"Hierarchie"}));
  EXPECT_SPELLING_CORRECTION("Priese", 1, ({"Prise"}));
  EXPECT_SPELLING_CORRECTION("auslehren", 1, ({"ausleeren"}));
  EXPECT_SPELLING_CORRECTION("Phenomen", 1, ({"Phänomen"}));
  EXPECT_SPELLING_CORRECTION("Phänomän", 1, ({"Phänomen"}));
  EXPECT_SPELLING_CORRECTION("Phenomän", 1, ({"Phänomen"}));
  EXPECT_SPELLING_CORRECTION("gewehren", 1, ({"gewähren"}));
  EXPECT_SPELLING_CORRECTION("aba", 1, ({"aber"}));
  EXPECT_SPELLING_CORRECTION("gestan", 1, ({"gestern"}));
  EXPECT_SPELLING_CORRECTION("ronterfallen", 1, ({"runterfallen"}));
  EXPECT_SPELLING_CORRECTION("Hönig", 1, ({"Honig"}));
  EXPECT_SPELLING_CORRECTION("mussen", 1, ({"müssen"}));
  EXPECT_SPELLING_CORRECTION("Bewandnis", 1, ({"Bewandtnis"}));
  EXPECT_SPELLING_CORRECTION("hässlig", 1, ({"hässlich"}));
  EXPECT_SPELLING_CORRECTION("lustich", 1, ({"lustig"}));
  EXPECT_SPELLING_CORRECTION("Botschaftler", 1, ({"Botschafter"}));
  EXPECT_SPELLING_CORRECTION("ebemfalls", 1, ({"ebenfalls"}));
  EXPECT_SPELLING_CORRECTION("samft", 1, ({"sanft"}));
  EXPECT_SPELLING_CORRECTION("Wohenzimmer", 1, ({"Wohnzimmer"}));
  EXPECT_SPELLING_CORRECTION("Flaster", 1, ({"Pflaster"}));
  EXPECT_SPELLING_CORRECTION("Imfung", 1, ({"Impfung"}));
  EXPECT_SPELLING_CORRECTION("amptieren", 1, ({"amtieren"}));
  EXPECT_SPELLING_CORRECTION("Endgeld", 1, ({"Entgelt"}));
  EXPECT_SPELLING_CORRECTION("Abendteuer", 1, ({"Abenteuer"}));
  EXPECT_SPELLING_CORRECTION("sampft", 1, ({"sanft"}));
  EXPECT_SPELLING_CORRECTION("forgestan", 1, ({"vorgestern"}));
  EXPECT_SPELLING_CORRECTION("Füreschein", 1, ({"Führerschein"}));
  EXPECT_SPELLING_CORRECTION("ronterfalen", 1, ({"runterfallen"}));
  EXPECT_SPELLING_CORRECTION("Farradschluss", 1, ({"Fahrradschloss"}));
  EXPECT_SPELLING_CORRECTION("Konkorenz", 1, ({"Konkurrenz"}));
  EXPECT_SPELLING_CORRECTION("Hirachie", 1, ({"Hierarchie"}));

  //////////////////////////////////////////////////////////////////////////////
  // Edge cases
  //////////////////////////////////////////////////////////////////////////////

  // Exact match is not considered a spelling correction
  EXPECT_SPELLING_CORRECTION("Führerschein", 1, ({}));

  // Max edit distance is 3
  EXPECT_SPELLING_CORRECTION(  "Führersch",    1, ({"Führerschein"}));
    EXPECT_SPELLING_CORRECTION("Führersc",     1, ({}));
    // Case matters in edit distance
    EXPECT_SPELLING_CORRECTION("führersch",    1, ({}));
    // Diacritics matters in edit distance
    EXPECT_SPELLING_CORRECTION("Fuhrersch",    1, ({}));
    // Mismatch in diacritics counts as 1 in edit distance (this is not trivial,
    // because from the UTF-8 perspective it is a one-byte vs two-byte encoding
    // of a Unicode codepoint).
    EXPECT_SPELLING_CORRECTION("Führersche",   1, ({"Führerschein"}));

  EXPECT_SPELLING_CORRECTION("Führershine",  1, ({"Führerschein"}));
    EXPECT_SPELLING_CORRECTION("Führershyne",  1, ({}));
    EXPECT_SPELLING_CORRECTION("führershine",  1, ({}));

  EXPECT_SPELLING_CORRECTION("Führerschrom", 1, ({"Führerschein"}));
  EXPECT_SPELLING_CORRECTION("Führerscdrom", 1, ({}));

  //////////////////////////////////////////////////////////////////////////////
  // Shortcomings of the proof-of-concept implementation
  //////////////////////////////////////////////////////////////////////////////

  // Multiword titles are treated as a single entity
  EXPECT_SPELLING_CORRECTION("Laurem", 1, ({}));
  EXPECT_SPELLING_CORRECTION("ibsum",  1, ({}));
  EXPECT_SPELLING_CORRECTION("Loremipsum", 1, ({"Lorem ipsum"}));

  // Only one spelling correction can be requested
  // EXPECT_SPELLING_CORRECTION("Kung",  2, ({"King", "Kong"}));
  EXPECT_THROW(spellingsDB.getSpellingCorrections("Kung", 2), std::runtime_error);
}

using StrCollection = std::vector<std::string>;

StrCollection directoryEntries(std::filesystem::path dirPath)
{
  StrCollection result;
  for ( const auto& dirEntry : std::filesystem::directory_iterator(dirPath) ) {
    result.push_back(dirEntry.path().string());
  }
  return result;
}

TEST_F(SpellingCorrectionTest, allInOne)
{
  const auto tmpDirModTime0 = std::filesystem::last_write_time(tmpDirPath);
  ASSERT_TRUE(directoryEntries(tmpDirPath).empty());
  {
    const kiwix::SpellingsDB spellingsDB(*archive, tmpDirPath);
    testSpellingCorrections(spellingsDB);
  }

  const auto tmpDirModTime1 = std::filesystem::last_write_time(tmpDirPath);

  const auto spellingsDbPath = tmpDirPath / "554c9707-897e-097a-53ba-1b1306d8bb88.spellingsdb.v0.1";

  const StrCollection EXPECTED_DIR_CONTENT{ spellingsDbPath.string() };
  ASSERT_EQ(directoryEntries(tmpDirPath), EXPECTED_DIR_CONTENT);
  ASSERT_LT(tmpDirModTime0, tmpDirModTime1);
  const auto fileModTime = std::filesystem::last_write_time(spellingsDbPath);

  {
    const kiwix::SpellingsDB spellingsDB(*archive, tmpDirPath);
    testSpellingCorrections(spellingsDB);
  }

  ASSERT_EQ(directoryEntries(tmpDirPath), EXPECTED_DIR_CONTENT );
  ASSERT_EQ(tmpDirModTime1, std::filesystem::last_write_time(tmpDirPath));
  ASSERT_EQ(fileModTime,    std::filesystem::last_write_time(spellingsDbPath));
}
