#include "../include/name_mapper.h"

#include "../include/library.h"
#include "../include/manager.h"
#include "gtest/gtest.h"

#include "testing_tools.h"

namespace
{


const char libraryXML[] = R"(
<library version="1.0">
  <book id="01"         path="/data/zero_one.zim">         </book>
  <book id="02"         path="/data/zero two.zim">         </book>
  <book id="03"         path="/data/ZERO thrêë.zim">       </book>
  <book id="04-2021-10" path="/data/zero_four_2021-10.zim"></book>
  <book id="04-2021-11" path="/data/zero_four_2021-11.zim"></book>
</library>
)";

class NameMapperTest : public ::testing::Test {
 public:
  explicit NameMapperTest()
  {}

 protected:
  void SetUp() override {
     kiwix::Manager manager{NotOwned<kiwix::Library>(lib)};
     manager.readXml(libraryXML, false, "./library.xml", true);
     for ( const std::string& id : lib.getBooksIds() ) {
       kiwix::Book bookCopy = lib.getBookById(id);
       bookCopy.setPathValid(true);
       lib.addBook(bookCopy);
     }
  }

  kiwix::Library lib;
};

class CapturedStderr
{
  std::ostringstream buffer;
  std::streambuf* const sbuf;
public:
  CapturedStderr()
    : sbuf(std::cerr.rdbuf())
  {
    std::cerr.rdbuf(buffer.rdbuf());
  }

  CapturedStderr(const CapturedStderr&) = delete;

  ~CapturedStderr()
  {
    std::cerr.rdbuf(sbuf);
  }

  operator std::string() const { return buffer.str(); }
};

} // unnamed namespace

void checkUnaliasedEntriesInNameMapper(const kiwix::NameMapper& nm)
{
  EXPECT_EQ("zero_one",           nm.getNameForId("01"));
  EXPECT_EQ("zero_two",           nm.getNameForId("02"));
  EXPECT_EQ("zero_three",         nm.getNameForId("03"));
  EXPECT_EQ("zero_four_2021-10",  nm.getNameForId("04-2021-10"));
  EXPECT_EQ("zero_four_2021-11",  nm.getNameForId("04-2021-11"));

  EXPECT_EQ("01",         nm.getIdForName("zero_one"));
  EXPECT_EQ("02",         nm.getIdForName("zero_two"));
  EXPECT_EQ("03",         nm.getIdForName("zero_three"));
  EXPECT_EQ("04-2021-10", nm.getIdForName("zero_four_2021-10"));
  EXPECT_EQ("04-2021-11", nm.getIdForName("zero_four_2021-11"));
}

TEST_F(NameMapperTest, HumanReadableNameMapperWithoutAliases)
{
  CapturedStderr stderror;
  kiwix::HumanReadableNameMapper nm(lib, false);
  EXPECT_EQ("", std::string(stderror));

  checkUnaliasedEntriesInNameMapper(nm);
  EXPECT_THROW(nm.getIdForName("zero_four"), std::out_of_range);

  lib.removeBookById("04-2021-10");
  EXPECT_EQ("zero_four_2021-10",  nm.getNameForId("04-2021-10"));
  EXPECT_EQ("04-2021-10", nm.getIdForName("zero_four_2021-10"));
  EXPECT_THROW(nm.getIdForName("zero_four"), std::out_of_range);
}

TEST_F(NameMapperTest, HumanReadableNameMapperWithAliases)
{
  CapturedStderr stderror;
  kiwix::HumanReadableNameMapper nm(lib, true);
  EXPECT_EQ(
      "Path collision: /data/zero_four_2021-10.zim and"
      " /data/zero_four_2021-11.zim can't share the same URL path 'zero_four'."
      " Therefore, only /data/zero_four_2021-10.zim will be served.\n"
      , std::string(stderror)
  );

  checkUnaliasedEntriesInNameMapper(nm);
  EXPECT_EQ("04-2021-10", nm.getIdForName("zero_four"));

  lib.removeBookById("04-2021-10");
  EXPECT_EQ("zero_four_2021-10",  nm.getNameForId("04-2021-10"));
  EXPECT_EQ("04-2021-10", nm.getIdForName("zero_four_2021-10"));
  EXPECT_EQ("04-2021-10", nm.getIdForName("zero_four"));
}

TEST_F(NameMapperTest, UpdatableNameMapperWithoutAliases)
{
  CapturedStderr stderror;
  kiwix::UpdatableNameMapper nm(NotOwned<kiwix::Library>(lib), false);
  EXPECT_EQ("", std::string(stderror));

  checkUnaliasedEntriesInNameMapper(nm);
  EXPECT_THROW(nm.getIdForName("zero_four"), std::out_of_range);

  lib.removeBookById("04-2021-10");
  nm.update();
  EXPECT_THROW(nm.getNameForId("04-2021-10"), std::out_of_range);
  EXPECT_THROW(nm.getIdForName("zero_four_2021-10"), std::out_of_range);
  EXPECT_THROW(nm.getIdForName("zero_four"), std::out_of_range);
}

TEST_F(NameMapperTest, UpdatableNameMapperWithAliases)
{
  CapturedStderr stderror;
  kiwix::UpdatableNameMapper nm(NotOwned<kiwix::Library>(lib), true);
  EXPECT_EQ(
      "Path collision: /data/zero_four_2021-10.zim and"
      " /data/zero_four_2021-11.zim can't share the same URL path 'zero_four'."
      " Therefore, only /data/zero_four_2021-10.zim will be served.\n"
      , std::string(stderror)
  );

  checkUnaliasedEntriesInNameMapper(nm);
  EXPECT_EQ("04-2021-10", nm.getIdForName("zero_four"));

  {
    CapturedStderr nmUpdateStderror;
    lib.removeBookById("04-2021-10");
    nm.update();
    EXPECT_EQ("", std::string(nmUpdateStderror));
  }
  EXPECT_EQ("04-2021-11", nm.getIdForName("zero_four"));
  EXPECT_THROW(nm.getNameForId("04-2021-10"), std::out_of_range);
  EXPECT_THROW(nm.getIdForName("zero_four_2021-10"), std::out_of_range);
}
