
#include "gtest/gtest.h"
#include "../include/reader.h"
#include "zim/archive.h"

namespace kiwix
{
  /**
   * This test file is written primarily to demonstrate how Reader is simply a
   * wrapper over an archive. We will be dropping this wrapper soon.
   **/
  TEST (Reader, archiveWrapper) {
    Reader reader("./test/zimfile.zim");
    zim::Archive archive = *reader.getZimArchive();

    std::ostringstream s;
    s << archive.getUuid();

    ASSERT_EQ(reader.getId(), s.str());
    ASSERT_EQ(reader.getGlobalCount(), archive.getEntryCount());
    ASSERT_EQ(reader.getMainPage().getTitle(), archive.getMainEntry().getTitle());
    ASSERT_EQ(reader.hasFulltextIndex(), archive.hasFulltextIndex());
    ASSERT_NO_THROW(reader.getRandomPage());
  }

  TEST (Reader, getFunctions) {
    zim::Archive archive("./test/zimfile.zim");
    Reader reader("./test/zimfile.zim");

    auto archiveEntry = archive.getRandomEntry();
    ASSERT_TRUE(reader.pathExists(archiveEntry.getPath()));
    auto readerEntry = reader.getEntryFromPath(archiveEntry.getPath());
    ASSERT_EQ(readerEntry.getTitle(), archiveEntry.getTitle());

    ASSERT_FALSE(reader.pathExists("invalidEntryPath"));
    ASSERT_THROW(reader.getEntryFromPath("invalidEntryPath"), NoEntry);

    readerEntry = reader.getEntryFromTitle(archiveEntry.getTitle());
    ASSERT_EQ(readerEntry.getTitle(), archiveEntry.getTitle());
  }

  TEST (Reader, suggestions) {
    Reader reader("./test/zimfile.zim");
    SuggestionsList_t suggestions;
    reader.searchSuggestionsSmart("The Genius", 4, suggestions);

    std::vector<std::string> suggestionResult, expectedResult;
    std::string suggestionTitle;
    for (auto it = suggestions.begin(); it != suggestions.end(); it++) {
      suggestionResult.push_back(it->getTitle());
    }

    expectedResult = {
      "The Genius After Hours",
      "The Genius Hits the Road",
      "The Genius Sings the Blues",
      "The Genius of Ray Charles"
    };

    ASSERT_EQ(suggestionResult, expectedResult);
  }
}