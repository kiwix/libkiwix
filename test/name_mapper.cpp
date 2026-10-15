#include "../include/name_mapper.h"

#include "../include/library.h"
#include "../include/manager.h"
#include "testing_tools.h"
using namespace kiwix::testing;

#include "gtest/gtest.h"

namespace
{

#if _WIN32
# define DATA_DIR "C:\\data\\"
# define SUBDIR   "subdir\\"
#else
# define DATA_DIR "/data/"
# define SUBDIR   "subdir/"
#endif

struct LibraryBook
{
  const char* id;
  const char* fileName;
  const char* name;
};

const LibraryBook libraryBooks[] = {
  { "01",            "zero_one.zim",            "" },
  { "02",            "zero two.zim",            "" },
  { "03",            "ZERO thrêë.zim",          "" },
  { "04-2021-10",    "zero_four_2021-10.zim",   "" },
  { "04-2021-11",    "zero_four_2021-11.zim",   "" },
  { "05-a",          "zero_five-a.zim",         "zero_five" },
  { "05-b",          "zero_five-b.zim",         "zero_five" },
  { "06+",           "zërô + SIX.zim",          "" },
  { "06plus",        "zero_plus_six.zim",       "" },
  { "07-super",      "zero_seven.zim",          "" },
  { "07-sub",        SUBDIR "zero_seven.zim",   "" },
  { "08-2026-07a",   "zero_eight_2026-07a.zim", "" },
  { "09-2026-07aa",  "zero_nine_2026-07aa.zim", "" },
  { "10-2026-07abc", "ten_2026-07abc.zim",      "" },
};

std::string libraryOpds()
{
  std::string opds = R"(<feed xmlns="http://www.w3.org/2005/Atom">)";
  for ( const auto& b : libraryBooks ) {
    opds += std::string("<entry><id>") + b.id + "</id>"
          + "<name>" + b.name + "</name>"
          + R"(<link rel="http://opds-spec.org/acquisition/open-access" href=")"
          + DATA_DIR + b.fileName + R"("/></entry>)";
  }
  return opds + "</feed>";
}

class NameMapperTest : public ::testing::Test {
 public:
  NameMapperTest(): lib(kiwix::Library::create()) {}
 protected:
  void SetUp() override {
     kiwix::Manager manager(lib);
     manager.readOpds(libraryOpds(), "./library.opds", /*readOnly=*/false);
     for ( const std::string& id : lib->getBooksIds() ) {
       kiwix::Book bookCopy = lib->getBookById(id);
       bookCopy.setPathValid(true);
       lib->addBook(bookCopy);
     }
  }

  std::shared_ptr<kiwix::Library> lib;
};

#if _WIN32
const std::string ZERO_FOUR_NAME_CONFLICT_MSG =
    "Path collision: 'C:\\data\\zero_four_2021-10.zim' and"
    " 'C:\\data\\zero_four_2021-11.zim' can't share the same URL path 'zero_four'."
    " Therefore, only 'C:\\data\\zero_four_2021-10.zim' will be served.\n";

const std::string ZERO_SIX_NAME_CONFLICT_MSG =
    "Path collision: 'C:\\data\\zërô + SIX.zim' and "
    "'C:\\data\\zero_plus_six.zim' can't share the same URL path 'zero_plus_six'."
    " Therefore, only 'C:\\data\\zërô + SIX.zim' will be served.\n";

const std::string ZERO_SEVEN_NAME_CONFLICT_MSG =
    "Path collision: 'C:\\data\\subdir\\zero_seven.zim' and"
    " 'C:\\data\\zero_seven.zim' can't share the same URL path 'zero_seven'."
    " Therefore, only 'C:\\data\\subdir\\zero_seven.zim' will be served.\n";
#else
const std::string ZERO_FOUR_NAME_CONFLICT_MSG =
    "Path collision: '/data/zero_four_2021-10.zim' and"
    " '/data/zero_four_2021-11.zim' can't share the same URL path 'zero_four'."
    " Therefore, only '/data/zero_four_2021-10.zim' will be served.\n";

const std::string ZERO_SIX_NAME_CONFLICT_MSG =
    "Path collision: '/data/zërô + SIX.zim' and "
    "'/data/zero_plus_six.zim' can't share the same URL path 'zero_plus_six'."
    " Therefore, only '/data/zërô + SIX.zim' will be served.\n";

const std::string ZERO_SEVEN_NAME_CONFLICT_MSG =
    "Path collision: '/data/subdir/zero_seven.zim' and"
    " '/data/zero_seven.zim' can't share the same URL path 'zero_seven'."
    " Therefore, only '/data/subdir/zero_seven.zim' will be served.\n";
#endif

// Name conflicts in the default mode (without the --nodatealiases is off
const std::string DEFAULT_NAME_CONFLICTS = ZERO_SIX_NAME_CONFLICT_MSG
                                         + ZERO_SEVEN_NAME_CONFLICT_MSG;

// Name conflicts in --nodatealiases mode
const std::string ALL_NAME_CONFLICTS = ZERO_FOUR_NAME_CONFLICT_MSG
                                     + ZERO_SIX_NAME_CONFLICT_MSG
                                     + ZERO_SEVEN_NAME_CONFLICT_MSG;

} // unnamed namespace

void checkUnaliasedEntriesInNameMapper(const kiwix::NameMapper& nm)
{
  EXPECT_EQ("zero_one",            nm.getNameForId("01"));
  EXPECT_EQ("zero_two",            nm.getNameForId("02"));
  EXPECT_EQ("zero_three",          nm.getNameForId("03"));
  EXPECT_EQ("zero_four_2021-10",   nm.getNameForId("04-2021-10"));
  EXPECT_EQ("zero_four_2021-11",   nm.getNameForId("04-2021-11"));
  EXPECT_EQ("zero_five-a",         nm.getNameForId("05-a"));
  EXPECT_EQ("zero_five-b",         nm.getNameForId("05-b"));
  EXPECT_EQ("zero_eight_2026-07a",  nm.getNameForId("08-2026-07a"));
  EXPECT_EQ("zero_nine_2026-07aa",  nm.getNameForId("09-2026-07aa"));
  EXPECT_EQ("ten_2026-07abc",       nm.getNameForId("10-2026-07abc"));

  // unreported conflict
  EXPECT_EQ("zero_plus_six",      nm.getNameForId("06+"));
  EXPECT_EQ("zero_plus_six",      nm.getNameForId("06plus"));

  // unreported conflict
  EXPECT_EQ("zero_seven",         nm.getNameForId("07-super"));
  EXPECT_EQ("zero_seven",         nm.getNameForId("07-sub"));

  EXPECT_EQ("01",           nm.getIdForName("zero_one"));
  EXPECT_EQ("02",           nm.getIdForName("zero_two"));
  EXPECT_EQ("03",           nm.getIdForName("zero_three"));
  EXPECT_EQ("04-2021-10",   nm.getIdForName("zero_four_2021-10"));
  EXPECT_EQ("04-2021-11",   nm.getIdForName("zero_four_2021-11"));
  EXPECT_EQ("08-2026-07a",   nm.getIdForName("zero_eight_2026-07a"));
  EXPECT_EQ("09-2026-07aa",  nm.getIdForName("zero_nine_2026-07aa"));
  EXPECT_EQ("10-2026-07abc", nm.getIdForName("ten_2026-07abc"));

  // book name doesn't participate in name mapping
  EXPECT_THROW(nm.getIdForName("zero_five"), std::out_of_range);
  EXPECT_EQ("05-a",       nm.getIdForName("zero_five-a"));
  EXPECT_EQ("05-b",       nm.getIdForName("zero_five-b"));

  EXPECT_EQ("06+",        nm.getIdForName("zero_plus_six"));
  EXPECT_EQ("07-sub",     nm.getIdForName("zero_seven"));
}

TEST_F(NameMapperTest, HumanReadableNameMapperWithoutAliases)
{
  CapturedStderr stderror;
  kiwix::HumanReadableNameMapper nm(*lib, false);
  EXPECT_EQ(DEFAULT_NAME_CONFLICTS, std::string(stderror));

  checkUnaliasedEntriesInNameMapper(nm);
  EXPECT_THROW(nm.getIdForName("zero_four"), std::out_of_range);
  EXPECT_THROW(nm.getIdForName("zero_eight"), std::out_of_range);
  EXPECT_THROW(nm.getIdForName("zero_nine"), std::out_of_range);
  EXPECT_THROW(nm.getIdForName("ten"), std::out_of_range);

  lib->removeBookById("04-2021-10");
  EXPECT_EQ("zero_four_2021-10",  nm.getNameForId("04-2021-10"));
  EXPECT_EQ("04-2021-10", nm.getIdForName("zero_four_2021-10"));
  EXPECT_THROW(nm.getIdForName("zero_four"), std::out_of_range);
}

TEST_F(NameMapperTest, HumanReadableNameMapperWithAliases)
{
  CapturedStderr stderror;
  kiwix::HumanReadableNameMapper nm(*lib, true);
  EXPECT_EQ(ALL_NAME_CONFLICTS, std::string(stderror));

  checkUnaliasedEntriesInNameMapper(nm);
  EXPECT_EQ("04-2021-10",  nm.getIdForName("zero_four"));
  EXPECT_EQ("08-2026-07a",  nm.getIdForName("zero_eight"));
  EXPECT_EQ("09-2026-07aa", nm.getIdForName("zero_nine"));
  EXPECT_THROW(nm.getIdForName("ten"), std::out_of_range);

  lib->removeBookById("04-2021-10");
  EXPECT_EQ("zero_four_2021-10",  nm.getNameForId("04-2021-10"));
  EXPECT_EQ("04-2021-10", nm.getIdForName("zero_four_2021-10"));
  EXPECT_EQ("04-2021-10", nm.getIdForName("zero_four"));
}

TEST_F(NameMapperTest, UpdatableNameMapperWithoutAliases)
{
  CapturedStderr stderror;
  kiwix::UpdatableNameMapper nm(lib, false);
  EXPECT_EQ(DEFAULT_NAME_CONFLICTS, std::string(stderror));

  checkUnaliasedEntriesInNameMapper(nm);
  EXPECT_THROW(nm.getIdForName("zero_four"), std::out_of_range);

  lib->removeBookById("04-2021-10");
  nm.update();
  EXPECT_THROW(nm.getNameForId("04-2021-10"), std::out_of_range);
  EXPECT_THROW(nm.getIdForName("zero_four_2021-10"), std::out_of_range);
  EXPECT_THROW(nm.getIdForName("zero_four"), std::out_of_range);
}

TEST_F(NameMapperTest, UpdatableNameMapperWithAliases)
{
  CapturedStderr stderror;
  kiwix::UpdatableNameMapper nm(lib, true);
  EXPECT_EQ(ALL_NAME_CONFLICTS, std::string(stderror));

  checkUnaliasedEntriesInNameMapper(nm);
  EXPECT_EQ("04-2021-10", nm.getIdForName("zero_four"));

  {
    CapturedStderr nmUpdateStderror;
    lib->removeBookById("04-2021-10");
    nm.update();
    EXPECT_EQ(DEFAULT_NAME_CONFLICTS, std::string(nmUpdateStderror));
  }
  EXPECT_EQ("04-2021-11", nm.getIdForName("zero_four"));
  EXPECT_THROW(nm.getNameForId("04-2021-10"), std::out_of_range);
  EXPECT_THROW(nm.getIdForName("zero_four_2021-10"), std::out_of_range);
}
