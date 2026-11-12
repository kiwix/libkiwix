
#define CPPHTTPLIB_ZLIB_SUPPORT 1
#include "./httplib.h"
#include "gtest/gtest.h"

#define SERVER_PORT 8001
#include "server_testing_tools.h"

#include "../src/tools/stringTools.h"

////////////////////////////////////////////////////////////////////////////////
// Testing of the library-related functionality of the server
////////////////////////////////////////////////////////////////////////////////

class LibraryServerTest : public ::testing::Test
{
protected:
  std::unique_ptr<ZimFileServer>   zfs1_;

  const int PORT = 8002;

protected:
  void resetServer(ZimFileServer::Cfg cfg) {
    zfs1_.reset();
    zfs1_.reset(new ZimFileServer(PORT, cfg, "./test/library.opds"));
  }

  void resetServer(ZimFileServer::Options options, std::string contentServerUrl="") {
    ZimFileServer::Cfg cfg(options);
    cfg.contentServerUrl = contentServerUrl;
    resetServer(cfg);
  }

  void SetUp() override {
    zfs1_.reset(new ZimFileServer(PORT, ZimFileServer::DEFAULT_OPTIONS, "./test/library.opds"));
  }

  void TearDown() override {
    zfs1_.reset();
  }

  ZimFileServer::Response getFromCatalogEndpoint(const std::string& suburl) const
  {
    const auto url = kiwix::urlEncode(zfs1_->cfg.root) + "/catalog/v2/" + suburl;
    return zfs1_->GET(url.c_str());
  }
};

// Returns a copy of 'text' where every line that fully matches 'pattern'
// preceded by optional whitespace is replaced with the fixed string
// 'replacement' preserving the leading whitespace
std::string replaceLines(const std::string& text,
                         const std::string& pattern,
                         const std::string& replacement)
{
  std::regex regex("^ *" + pattern + "$");
  std::ostringstream oss;
  std::istringstream iss(text);
  std::string line;
  while ( std::getline(iss, line) ) {
    if ( std::regex_match(line, regex) ) {
      for ( size_t i = 0; i < line.size() && line[i] == ' '; ++i )
        oss << ' ';
      oss << replacement << "\n";
    } else {
      oss << line << "\n";
    }
  }
  return oss.str();
}

std::string maskVariableOPDSFeedData(std::string s)
{
  s = replaceLines(s, R"(<updated>\d\d\d\d-\d\d-\d\dT\d\d:\d\d:\d\dZ</updated>)",
                      "<updated>YYYY-MM-DDThh:mm:ssZ</updated>");
  s = replaceLines(s, "<id>[[:xdigit:]]{8}-[[:xdigit:]]{4}-[[:xdigit:]]{4}-[[:xdigit:]]{4}-[[:xdigit:]]{12}</id>",
                      "<id>12345678-90ab-cdef-1234-567890abcdef</id>");
  return s;
}

#include "./expected_output/library_server.h"

#define TEST_CATALOG_ENDPOINT(SUBURL, EXPECTED_OUTPUT)           \
  {                                                              \
  const auto r = getFromCatalogEndpoint(SUBURL);                 \
  EXPECT_EQ(r->status, 200);                                     \
  EXPECT_EQ(maskVariableOPDSFeedData(r->body), EXPECTED_OUTPUT); \
  }

TEST_F(LibraryServerTest, catalog_v2_root)
{
  TEST_CATALOG_ENDPOINT("root.xml", EO(CATALOG_ROOT));
}

TEST_F(LibraryServerTest, catalog_v2_searchdescription_xml)
{
  TEST_CATALOG_ENDPOINT("searchdescription.xml", EO(CATALOG_SEARCHDESCRIPTION));
}

TEST_F(LibraryServerTest, catalog_v2_categories)
{
  TEST_CATALOG_ENDPOINT("categories", EO(CATALOG_CATEGORIES));
}

TEST_F(LibraryServerTest, catalog_v2_languages)
{
  TEST_CATALOG_ENDPOINT("languages", EO(CATALOG_LANGUAGES));
}

TEST_F(LibraryServerTest, catalog_v2_entries)
{
  TEST_CATALOG_ENDPOINT("entries",
    CATALOG_V2_ENTRIES_PREAMBLE("")
    "  <title>All Entries</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    EO(CHARLES_RAY_CATALOG_ENTRY)
    EO(RAY_CHARLES_CATALOG_ENTRY)
    EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_v2_entries_catalog_only_mode)
{
  const std::string contentServerUrl = "https://demo.kiwix.org";
  const auto fixContentLinks = [=](std::string s) -> std::string {
    s = replace(s, "/ROOT%23%3F/content", contentServerUrl + "/content");
    return s;
  };
  resetServer(ZimFileServer::CATALOG_ONLY_MODE, contentServerUrl);

  const std::string expectedOPDS =
    CATALOG_V2_ENTRIES_PREAMBLE("")
    "  <title>All Entries</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    + fixContentLinks(EO(CHARLES_RAY_CATALOG_ENTRY))
    + fixContentLinks(EO(INACCESSIBLEZIMFILE_CATALOG_ENTRY))
    + fixContentLinks(EO(RAY_CHARLES_CATALOG_ENTRY))
    + fixContentLinks(EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)) +
    "</feed>\n";

  TEST_CATALOG_ENDPOINT("entries", expectedOPDS);

  { // test with empty rootLocation
    const auto fixRoot = [=](std::string s) -> std::string {
      return replace(s, "/ROOT%23%3F/", "/");
    };

    ZimFileServer::Cfg serverCfg;
    serverCfg.root = "";
    serverCfg.options = ZimFileServer::CATALOG_ONLY_MODE;
    serverCfg.contentServerUrl = contentServerUrl;
    resetServer(serverCfg);

    TEST_CATALOG_ENDPOINT("entries", fixRoot(expectedOPDS));
  }
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtered_by_range)
{
  {
    TEST_CATALOG_ENDPOINT("entries?start=1",
      CATALOG_V2_ENTRIES_PREAMBLE("?start=1")
      "  <title>Filtered Entries (start=1)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>1</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      EO(RAY_CHARLES_CATALOG_ENTRY)
      EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
      "</feed>\n"
    );
  }

  {
    // count=-1 disables the limit on the number of results
    TEST_CATALOG_ENDPOINT("entries?count=-1",
      CATALOG_V2_ENTRIES_PREAMBLE("?count=-1")
      "  <title>Filtered Entries (count=-1)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>3</itemsPerPage>\n"
      EO(CHARLES_RAY_CATALOG_ENTRY)
      EO(RAY_CHARLES_CATALOG_ENTRY)
      EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
      "</feed>\n"
    );
  }

  {
    // count=0 returns 0 results
    TEST_CATALOG_ENDPOINT("entries?count=0",
      CATALOG_V2_ENTRIES_PREAMBLE("?count=0")
      "  <title>Filtered Entries (count=0)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>0</itemsPerPage>\n"
      "</feed>\n"
    );
  }

  {
    TEST_CATALOG_ENDPOINT("entries?count=2",
      CATALOG_V2_ENTRIES_PREAMBLE("?count=2")
      "  <title>Filtered Entries (count=2)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      EO(CHARLES_RAY_CATALOG_ENTRY)
      EO(RAY_CHARLES_CATALOG_ENTRY)
      "</feed>\n"
    );
  }

  {
    TEST_CATALOG_ENDPOINT("entries?start=1&count=1",
      CATALOG_V2_ENTRIES_PREAMBLE("?start=1&amp;count=1")
      "  <title>Filtered Entries (start=1&amp;count=1)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>1</startIndex>\n"
      "  <itemsPerPage>1</itemsPerPage>\n"
      EO(RAY_CHARLES_CATALOG_ENTRY)
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtered_by_words)
{
  TEST_CATALOG_ENDPOINT("entries?q=ray%20charles",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=ray%20charles")
    "  <title>Filtered Entries (q=ray%20charles)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>3</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>3</itemsPerPage>\n"
    EO(CHARLES_RAY_CATALOG_ENTRY)
    EO(RAY_CHARLES_CATALOG_ENTRY)
    EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtered_by_phrase)
{
  TEST_CATALOG_ENDPOINT("entries?q=\"ray%20charles\"",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=%22ray%20charles%22")
    "  <title>Filtered Entries (q=%22ray%20charles%22)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>2</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>2</itemsPerPage>\n"
    EO(CHARLES_RAY_CATALOG_ENTRY)
    EO(RAY_CHARLES_CATALOG_ENTRY)
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtering_via_prefix_search)
{
  {
    TEST_CATALOG_ENDPOINT("entries?q=description:ray%20description:charles",
      CATALOG_V2_ENTRIES_PREAMBLE("?q=description%3Aray%20description%3Acharles")
      "  <title>Filtered Entries (q=description%3Aray%20description%3Acharles)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>2</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      EO(CHARLES_RAY_CATALOG_ENTRY)
      EO(RAY_CHARLES_CATALOG_ENTRY)
      "</feed>\n"
    );
  }
  {
    TEST_CATALOG_ENDPOINT("entries?q=title:\"ray%20charles\"",
      CATALOG_V2_ENTRIES_PREAMBLE("?q=title%3A%22ray%20charles%22")
      "  <title>Filtered Entries (q=title%3A%22ray%20charles%22)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>1</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>1</itemsPerPage>\n"
      EO(RAY_CHARLES_CATALOG_ENTRY)
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtering_with_word_exclusion)
{
  TEST_CATALOG_ENDPOINT("entries?q=ray%20-uncategorized",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=ray%20-uncategorized")
    "  <title>Filtered Entries (q=ray%20-uncategorized)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>2</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>2</itemsPerPage>\n"
    EO(CHARLES_RAY_CATALOG_ENTRY)
    EO(RAY_CHARLES_CATALOG_ENTRY)
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtering_special_queries)
{
  {
  // 'or' doesn't act as a Xapian boolean operator
  TEST_CATALOG_ENDPOINT("entries?q=Or",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=Or")
    "  <title>Filtered Entries (q=Or)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    EO(CHARLES_RAY_CATALOG_ENTRY)
    "</feed>\n"
  );
  }

  {
  // 'and' doesn't act as a Xapian boolean operator
  TEST_CATALOG_ENDPOINT("entries?q=and",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=and")
    "  <title>Filtered Entries (q=and)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    EO(CHARLES_RAY_CATALOG_ENTRY)
    "</feed>\n"
  );
  }

  {
  // 'not' doesn't act as a Xapian boolean operator
  TEST_CATALOG_ENDPOINT("entries?q=not",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=not")
    "  <title>Filtered Entries (q=not)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    EO(RAY_CHARLES_CATALOG_ENTRY)
    "</feed>\n"
  );
  }

  {
  // 'xor' doesn't act as a Xapian boolean operator
  TEST_CATALOG_ENDPOINT("entries?q=xor",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=xor")
    "  <title>Filtered Entries (q=xor)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
    "</feed>\n"
  );
  }

  {
  // 'or' acts as a Xapian boolean operator
  TEST_CATALOG_ENDPOINT("entries?q=wikipedia%20or%20library",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=wikipedia%20or%20library")
    "  <title>Filtered Entries (q=wikipedia%20or%20library)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    EO(CHARLES_RAY_CATALOG_ENTRY)
    "</feed>\n"
  );
  }

  {
  // 'and' acts as a Xapian boolean operator
  TEST_CATALOG_ENDPOINT("entries?q=wikipedia%20and%20articles",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=wikipedia%20and%20articles")
    "  <title>Filtered Entries (q=wikipedia%20and%20articles)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    EO(CHARLES_RAY_CATALOG_ENTRY)
    "</feed>\n"
  );
  }

  {
  // 'near' doesn't act as a Xapian query operator
  TEST_CATALOG_ENDPOINT("entries?q=near",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=near")
    "  <title>Filtered Entries (q=near)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    EO(RAY_CHARLES_CATALOG_ENTRY)
    "</feed>\n"
  );
  }

  {
  // 'adj' doesn't act as a Xapian query operator
  TEST_CATALOG_ENDPOINT("entries?q=adj",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=adj")
    "  <title>Filtered Entries (q=adj)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
    "</feed>\n"
  );
  }

  {
  // 'near' doesn't act as a Xapian query operator
  TEST_CATALOG_ENDPOINT("entries?q=charles%20near%20why",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=charles%20near%20why")
    "  <title>Filtered Entries (q=charles%20near%20why)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>0</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>0</itemsPerPage>\n"
    "</feed>\n"
  );
  }

  {
  // 'adj' doesn't act as a Xapian query operator
  TEST_CATALOG_ENDPOINT("entries?q=charles%20adj%20why",
    CATALOG_V2_ENTRIES_PREAMBLE("?q=charles%20adj%20why")
    "  <title>Filtered Entries (q=charles%20adj%20why)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>0</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>0</itemsPerPage>\n"
    "</feed>\n"
  );
  }
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtered_by_language)
{
  {
    TEST_CATALOG_ENDPOINT("entries?lang=eng",
      CATALOG_V2_ENTRIES_PREAMBLE("?lang=eng")
      "  <title>Filtered Entries (lang=eng)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>2</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
      EO(RAY_CHARLES_CATALOG_ENTRY)
      "</feed>\n"
    );
  }

  {
    TEST_CATALOG_ENDPOINT("entries?lang=eng,fra",
      CATALOG_V2_ENTRIES_PREAMBLE("?lang=eng%2Cfra")
      "  <title>Filtered Entries (lang=eng%2Cfra)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>3</itemsPerPage>\n"
      EO(CHARLES_RAY_CATALOG_ENTRY)
      EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
      EO(RAY_CHARLES_CATALOG_ENTRY)
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtered_by_category)
{
  {
    TEST_CATALOG_ENDPOINT("entries?category=jazz",
      CATALOG_V2_ENTRIES_PREAMBLE("?category=jazz")
      "  <title>Filtered Entries (category=jazz)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>1</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>1</itemsPerPage>\n"
      EO(CHARLES_RAY_CATALOG_ENTRY)
      "</feed>\n"
    );
  }

  {
    TEST_CATALOG_ENDPOINT("entries?category=jazz,wikipedia",
      CATALOG_V2_ENTRIES_PREAMBLE("?category=jazz%2Cwikipedia")
      "  <title>Filtered Entries (category=jazz%2Cwikipedia)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>2</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      EO(CHARLES_RAY_CATALOG_ENTRY)
      EO(RAY_CHARLES_CATALOG_ENTRY)
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_v2_entries_multiple_filters)
{
  {
    TEST_CATALOG_ENDPOINT("entries?lang=fra&category=jazz",
      CATALOG_V2_ENTRIES_PREAMBLE("?lang=fra&amp;category=jazz")
      "  <title>Filtered Entries (lang=fra&amp;category=jazz)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>1</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>1</itemsPerPage>\n"
      EO(CHARLES_RAY_CATALOG_ENTRY)
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_v2_individual_entry_access)
{
  TEST_CATALOG_ENDPOINT("entry/raycharles",
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    EO(RAY_CHARLES_CATALOG_ENTRY)
  );

  const auto r1 = getFromCatalogEndpoint("entry/non-existent-entry");
  EXPECT_EQ(r1->status, 404);
}

TEST_F(LibraryServerTest, catalog_v2_partial_entries)
{
  TEST_CATALOG_ENDPOINT("partial_entries", EO(CATALOG_PARTIAL_ENTRIES));
}

TEST_F(LibraryServerTest, ThumbnailMimeTypeHandling)
{
  // XXX: The checks performed in this unit-test are implicitly present
  // XXX: in the catalog_v2_entries_catalog_only_mode unit-test.
  resetServer(ZimFileServer::CATALOG_ONLY_MODE);
  const auto r = getFromCatalogEndpoint("entries");
  EXPECT_EQ(r->status, 200);

  EXPECT_TRUE(kiwix::contains(r->body,
    "<link rel=\"http://opds-spec.org/image/thumbnail\"\n"
    "          href=\"/ROOT%23%3F/catalog/v2/illustration/raycharles/?size=48\"\n"
    "          type=\"image/png;width=48;height=48;scale=1\"/>"
  ));

  EXPECT_FALSE(kiwix::contains(r->body, "illustration/charlesray/"));
  EXPECT_FALSE(kiwix::contains(r->body, "illustration/inaccessiblezim/"));
}

#define EXPECT_FILTERING_RESULTS(SEARCH_TERM, RESULT_COUNT, OPDS_ENTRIES)      \
  {                                                                         \
    TEST_CATALOG_ENDPOINT("entries?q=" SEARCH_TERM,                         \
      CATALOG_V2_ENTRIES_PREAMBLE("?q=" SEARCH_TERM)                        \
      "  <title>Filtered Entries (q=" SEARCH_TERM ")</title>\n"             \
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"                         \
      "  <totalResults>" #RESULT_COUNT "</totalResults>\n"                  \
      "  <startIndex>0</startIndex>\n"                                      \
      "  <itemsPerPage>" #RESULT_COUNT "</itemsPerPage>\n"                  \
                                                                            \
      OPDS_ENTRIES                                                          \
                                                                            \
      "</feed>\n"                                                           \
    );                                                                      \
  }

TEST_F(LibraryServerTest, catalog_v2_entries_filtering_includes_public_tags)
{
  EXPECT_FILTERING_RESULTS("public_tag_without_a_value",
                        1,
                        EO(RAY_CHARLES_CATALOG_ENTRY)
  );

  EXPECT_FILTERING_RESULTS("public_tag_with_a_value",
                        1,
                        EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
  );

  // prefix search works on tag names
  EXPECT_FILTERING_RESULTS("public_tag",
                        2,
                        EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
                        EO(RAY_CHARLES_CATALOG_ENTRY)
  );

  EXPECT_FILTERING_RESULTS("value_of_a_public_tag",
                        1,
                        EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
  );

  // prefix search works on tag values
  EXPECT_FILTERING_RESULTS("value_of",
                        1,
                        EO(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY)
  );
}

#define EXPECT_ZERO_RESULTS(SEARCH_TERM) EXPECT_FILTERING_RESULTS(SEARCH_TERM, 0, )

TEST_F(LibraryServerTest, catalog_v2_entries_filtering_on_tags_is_not_an_any_substring_match)
{
  EXPECT_ZERO_RESULTS("tag_with")
  EXPECT_ZERO_RESULTS("alue_of_a_public_tag")
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtering_excludes_hidden_tags)
{
  EXPECT_ZERO_RESULTS("_private_tag_without_a_value");
  EXPECT_ZERO_RESULTS("private_tag_without_a_value");
  EXPECT_ZERO_RESULTS("value_of_a_private_tag");

#undef EXPECT_ZERO_RESULTS
}

TEST_F(LibraryServerTest, no_name_mapper_returned_catalog_use_uuid_in_link)
{
  resetServer(ZimFileServer::NO_NAME_MAPPER);
  const auto r = getFromCatalogEndpoint("entries?tag=_category:jazz");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("?tag=_category%3Ajazz")
    "  <title>Filtered Entries (tag=_category%3Ajazz)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    EO(CHARLES_RAY_CATALOG_ENTRY_NO_NAMEMAPPER)
    "</feed>\n"
  );
}


TEST_F(LibraryServerTest, no_name_mapper_catalog_v2_individual_entry_access)
{
  resetServer(ZimFileServer::NO_NAME_MAPPER);
  const auto r = getFromCatalogEndpoint("entry/raycharles");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    EO(RAY_CHARLES_CATALOG_ENTRY_NO_NAMEMAPPER)
  );

  const auto r1 = getFromCatalogEndpoint("entry/non-existent-entry");
  EXPECT_EQ(r1->status, 404);
}

TEST_F(LibraryServerTest, noJS) {
  // no_js_default
  auto r = zfs1_->GET("/ROOT%23%3F/nojs");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(r->body,
            HTML_PREAMBLE
            FILTERS_HTML("")
            HOME_BODY_TEXT("3")
            CHARLES_RAY_BOOK_HTML
            RAY_CHARLES_BOOK_HTML
            RAY_CHARLES_UNCTZ_BOOK_HTML
            FINAL_HTML_TEXT);

  // no_js_eng_lang
  r = zfs1_->GET("/ROOT%23%3F/nojs?lang=eng");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(r->body,
            HTML_PREAMBLE
            FILTERS_HTML(" selected ")
            HOME_BODY_TEXT("2")
            RAY_CHARLES_UNCTZ_BOOK_HTML
            RAY_CHARLES_BOOK_HTML
            FINAL_HTML_TEXT);

  // no_js_no_books
  r = zfs1_->GET("/ROOT%23%3F/nojs?lang=fas");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(r->body,
            HTML_PREAMBLE
            FILTERS_HTML("")
            HOME_BODY_0_RESULTS
            FINAL_HTML_TEXT);

  // no_js_download
  r = zfs1_->GET("/ROOT%23%3F/nojs/download/zimfile_raycharles_uncategorized");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(r->body, RAY_CHARLES_UNCTZ_DOWNLOAD);
}

TEST(NoJSDownloadLinks, eachAcquisitionLinkIsUsedAsIs) {
  ZimFileServer server(8003, ZimFileServer::DEFAULT_OPTIONS, "./test/nojs_download_links.opds");

  auto r = server.GET("/ROOT%23%3F/nojs/download/zimfile_raycharles_uncategorized");
  EXPECT_EQ(r->status, 200);
  const char expected_output[] = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Download book</title>
</head>
<style>
    .downloadLinksTitle {
        text-align: center;
        font-size: 32px;
        margin-bottom: 8px;
    }
</style>
<body>
    <div class="downloadLinksTitle">
        Download links for <b><i>All links</i></b>
    </div>
    <a href="https://example.com/all.zim" download>
        <div>Direct</div>
    </a>
    <a href="https://example.com/all.zim.meta4" download>
        <div>Metalink</div>
    </a>
    <a href="https://example.com/all.zim.sha256" download>
        <div>SHA-256 checksum</div>
    </a>
    <a href="magnet:?xt=urn:btih:allzim" target="_blank">
        <div>Magnet link</div>
    </a>
    <a href="https://example.com/all.zim.tor" download>
        <div>BitTorrent</div>
    </a>
</body>
</html>)";
  EXPECT_EQ(r->body, expected_output);
}

TEST(NoJSDownloadLinks, missingLinkKindsAreOmitted) {
  ZimFileServer server(8003, ZimFileServer::DEFAULT_OPTIONS, "./test/nojs_download_links.opds");

  auto r = server.GET("/ROOT%23%3F/nojs/download/zimfile_raycharles");
  EXPECT_EQ(r->status, 200);
  const char expected_output[] = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Download book</title>
</head>
<style>
    .downloadLinksTitle {
        text-align: center;
        font-size: 32px;
        margin-bottom: 8px;
    }
</style>
<body>
    <div class="downloadLinksTitle">
        Download links for <b><i>Metalink only</i></b>
    </div>
    <a href="https://example.com/only.zim.meta4" download>
        <div>Metalink</div>
    </a>
</body>
</html>)";
  EXPECT_EQ(r->body, expected_output);
}

TEST_F(LibraryServerTest, noJS_catalogOnlyMode) {
  const std::string contentServerUrl = "https://demo.kiwix.org";
  const auto fixContentLinks = [=](std::string s) -> std::string {
    s = replace(s, "/ROOT%23%3F/content", contentServerUrl + "/content");
    return s;
  };

  resetServer(ZimFileServer::CATALOG_ONLY_MODE, contentServerUrl);

  auto r = zfs1_->GET("/ROOT%23%3F/nojs");
  EXPECT_EQ(r->status, 200);

  const std::string expectedHTML =
            HTML_PREAMBLE
            FILTERS_HTML("")
            HOME_BODY_TEXT("4")
            + fixContentLinks(CHARLES_RAY_BOOK_HTML)
            + fixContentLinks(INACCESSIBLEZIMFILE_BOOK_HTML)
            + fixContentLinks(RAY_CHARLES_BOOK_HTML)
            + fixContentLinks(RAY_CHARLES_UNCTZ_BOOK_HTML)
            + FINAL_HTML_TEXT;

  EXPECT_EQ(r->body, expectedHTML);

  { // test with empty rootLocation
    const auto fixRoot = [=](std::string s) -> std::string {
      s = replace(s, "/ROOT%23%3F/", "/");
      return s;
    };

    ZimFileServer::Cfg serverCfg;
    serverCfg.root = "";
    serverCfg.options = ZimFileServer::CATALOG_ONLY_MODE;
    serverCfg.contentServerUrl = contentServerUrl;

    resetServer(serverCfg);

    auto r = zfs1_->GET("/nojs");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(r->body, fixRoot(expectedHTML));
  }
}

#undef EXPECT_FILTERING_RESULTS
