
#define CPPHTTPLIB_ZLIB_SUPPORT 1
#include "./httplib.h"
#include "gtest/gtest.h"

#define SERVER_PORT 8001
#include "server_testing_tools.h"

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
    zfs1_.reset(new ZimFileServer(PORT, cfg, "./test/library.xml"));
  }

  void resetServer(ZimFileServer::Options options, std::string contentServerUrl="") {
    ZimFileServer::Cfg cfg(options);
    cfg.contentServerUrl = contentServerUrl;
    resetServer(cfg);
  }

  void SetUp() override {
    zfs1_.reset(new ZimFileServer(PORT, ZimFileServer::DEFAULT_OPTIONS, "./test/library.xml"));
  }

  void TearDown() override {
    zfs1_.reset();
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

#define OPDS_FEED_TAG \
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n" \
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n" \
    "      xmlns:opds=\"http://opds-spec.org/2010/catalog\">\n"

#define CATALOG_LINK_TAGS \
    "  <link rel=\"self\" href=\"\" type=\"application/atom+xml\" />\n" \
    "  <link rel=\"search\""                                            \
           " type=\"application/opensearchdescription+xml\""            \
           " href=\"/ROOT%23%3F/catalog/searchdescription.xml\" />\n"

#define CATALOG_ENTRY(UUID, TITLE, SUMMARY, LANG, NAME, CATEGORY, TAGS, EXTRA_LINK, CONTENT_NAME, FILE_NAME, LENGTH) \
    "  <entry>\n"                                                       \
    "    <id>urn:uuid:" UUID "</id>\n"                                \
    "    <title>" TITLE "</title>\n"                                 \
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"                     \
    "    <summary>" SUMMARY "</summary>\n"     \
    "    <language>" LANG "</language>\n"                                    \
    "    <name>" NAME "</name>\n"                       \
    "    <flavour></flavour>\n"                                         \
    "    <category>" CATEGORY "</category>\n"                                   \
    "    <tags>" TAGS "</tags>\n" \
    "    <articleCount>284</articleCount>\n"                            \
    "    <mediaCount>2</mediaCount>\n"                                  \
    "    " EXTRA_LINK "<link type=\"text/html\" href=\"/ROOT%23%3F/content/" CONTENT_NAME "\" />\n"               \
    "    <author>\n"                                                    \
    "      <name>Wikipedia</name>\n"                                    \
    "    </author>\n"                                                   \
    "    <publisher>\n"                                                 \
    "      <name>Kiwix</name>\n"                                        \
    "    </publisher>\n"                                                \
    "    <dc:issued>2020-03-31T00:00:00Z</dc:issued>\n"                 \
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/" FILE_NAME ".zim\" length=\"" LENGTH "\" />\n" \
    "  </entry>\n"


#define _CHARLES_RAY_CATALOG_ENTRY(CONTENT_NAME) CATALOG_ENTRY(  \
  "charlesray",   \
  "Charles, Ray", \
  "Wikipedia articles about Ray Charles or why and when one should go to library", \
  "fra", \
  "wikipedia_fr_ray_charles",\
  "jazz",\
  "unittest;wikipedia;_category:jazz;_pictures:no;_videos:no;_details:no;_ftindex:yes",\
  "", \
  CONTENT_NAME, \
  "zimfile%26other", \
  "569344" \
)

#define CHARLES_RAY_CATALOG_ENTRY           _CHARLES_RAY_CATALOG_ENTRY("zimfile%26other")
#define CHARLES_RAY_CATALOG_ENTRY_NO_MAPPER _CHARLES_RAY_CATALOG_ENTRY("charlesray")

#define _RAY_CHARLES_CATALOG_ENTRY(CONTENT_NAME) CATALOG_ENTRY(\
  "raycharles",\
  "Ray Charles",\
  "Wikipedia articles about Ray Charles (not all of them but near to what an average newborn may find more than enough)",\
  "eng",\
  "wikipedia_en_ray_charles",\
  "wikipedia",\
  "public_tag_without_a_value;_private_tag_without_a_value;wikipedia;_category:wikipedia;_pictures:no;_videos:no;_details:no;_ftindex:yes",\
  "<link rel=\"http://opds-spec.org/image/thumbnail\"\n"          \
  "          href=\"/ROOT%23%3F/catalog/v2/illustration/raycharles/?size=48\"\n" \
  "          type=\"image/png;width=48;height=48;scale=1\"/>\n    ", \
  CONTENT_NAME, \
  "zimfile_raycharles", \
  "569344"\
)

#define RAY_CHARLES_CATALOG_ENTRY           _RAY_CHARLES_CATALOG_ENTRY("zimfile_raycharles")
#define RAY_CHARLES_CATALOG_ENTRY_NO_MAPPER _RAY_CHARLES_CATALOG_ENTRY("raycharles")

#define UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY CATALOG_ENTRY(\
  "raycharles_uncategorized",\
  "Ray (uncategorized) Charles",\
  "No category is assigned to this library entry (neither adj nor xor was considered a good option)",\
  "rus,eng",\
  "wikipedia_ru_ray_charles",\
  "",\
  "public_tag_with_a_value:value_of_a_public_tag;_private_tag_with_a_value:value_of_a_private_tag;wikipedia;_pictures:no;_videos:no;_details:no",\
  "",\
  "zimfile_raycharles_uncategorized", \
  "zimfile_raycharles_uncategorized", \
  "125952"\
)

#define INACCESSIBLEZIMFILE_CATALOG_ENTRY \
"  <entry>\n" \
"    <id>urn:uuid:inaccessiblezim</id>\n" \
"    <title>Catalog of all catalogs</title>\n" \
"    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n" \
"    <summary>Testing that running kiwix-serve without access to ZIM files doesn&apos;t lead to a catastrophe</summary>\n" \
"    <language>cat</language>\n" \
"    <name>catalog_of_all_catalogs</name>\n" \
"    <flavour></flavour>\n" \
"    <category>cats</category>\n" \
"    <tags>unittest;_category:cats</tags>\n" \
"    <articleCount>12107</articleCount>\n" \
"    <mediaCount>8</mediaCount>\n" \
"    <link type=\"text/html\" href=\"/ROOT%23%3F/content/nosuchzimfile\" />\n" \
"    <author>\n" \
"      <name>Catherine of Catalonia</name>\n" \
"    </author>\n" \
"    <publisher>\n" \
"      <name>Caterpillar</name>\n" \
"    </publisher>\n" \
"    <dc:issued>2025-09-04T00:00:00Z</dc:issued>\n" \
"    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/nosuchzimfile.zim\" length=\"20736925696\" />\n" \
"  </entry>\n"

TEST_F(LibraryServerTest, catalog_root_xml)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/root.xml");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    OPDS_FEED_TAG
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
    "  <title>All zims</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    CATALOG_LINK_TAGS
    CHARLES_RAY_CATALOG_ENTRY
    RAY_CHARLES_CATALOG_ENTRY
    UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_searchdescription_xml)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/searchdescription.xml");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(r->body,
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<OpenSearchDescription xmlns=\"http://a9.com/-/spec/opensearch/1.1/\">\n"
    "  <ShortName>Zim catalog search</ShortName>\n"
    "  <Description>Search zim files in the catalog.</Description>\n"
    "  <Url type=\"application/atom+xml;profile=opds-catalog\"\n"
    "       xmlns:atom=\"http://www.w3.org/2005/Atom\"\n"
    "       xmlns:k=\"http://kiwix.org/opensearchextension/1.0\"\n"
    "       indexOffset=\"0\"\n"
    "       template=\"/ROOT%23%3F/catalog/search?q={searchTerms?}&lang={language?}&name={k:name?}&tag={k:tag?}&notag={k:notag?}&maxsize={k:maxsize?}&count={count?}&start={startIndex?}\"/>\n"
    "</OpenSearchDescription>\n"
  );
}

TEST_F(LibraryServerTest, catalog_search_by_phrase)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?q=\"ray%20charles\"");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    OPDS_FEED_TAG
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
    "  <title>Filtered zims (q=%22ray%20charles%22)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>2</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>2</itemsPerPage>\n"
    CATALOG_LINK_TAGS
    CHARLES_RAY_CATALOG_ENTRY
    RAY_CHARLES_CATALOG_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_search_by_words)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?q=ray%20charles");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    OPDS_FEED_TAG
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
    "  <title>Filtered zims (q=ray%20charles)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>3</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>3</itemsPerPage>\n"
    CATALOG_LINK_TAGS
    CHARLES_RAY_CATALOG_ENTRY
    RAY_CHARLES_CATALOG_ENTRY
    UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_prefix_search)
{
  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?q=description:ray%20description:charles");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (q=description%3Aray%20description%3Acharles)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>2</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      CHARLES_RAY_CATALOG_ENTRY
      RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }
  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?q=title:\"ray%20charles\"");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (q=title%3A%22ray%20charles%22)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>1</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>1</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_search_with_word_exclusion)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?q=ray%20-uncategorized");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    OPDS_FEED_TAG
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
    "  <title>Filtered zims (q=ray%20-uncategorized)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>2</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>2</itemsPerPage>\n"
    CATALOG_LINK_TAGS
    CHARLES_RAY_CATALOG_ENTRY
    RAY_CHARLES_CATALOG_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_search_by_tag)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?tag=_category:jazz");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    OPDS_FEED_TAG
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
    "  <title>Filtered zims (tag=_category%3Ajazz)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    CATALOG_LINK_TAGS
    CHARLES_RAY_CATALOG_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_search_by_category)
{

  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?category=jazz");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (category=jazz)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>1</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>1</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      CHARLES_RAY_CATALOG_ENTRY
      "</feed>\n"
    );
  }

  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?category=jazz,wikipedia");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (category=jazz%2Cwikipedia)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>2</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      CHARLES_RAY_CATALOG_ENTRY
      RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_search_by_language)
{
  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?lang=eng");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (lang=eng)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>2</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
      RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }

  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?lang=eng,fra");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (lang=eng%2Cfra)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>3</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      CHARLES_RAY_CATALOG_ENTRY
      UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
      RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_search_results_pagination)
{
  {
    // count=-1 disables the limit on the number of results
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?count=-1");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (count=-1)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>3</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      CHARLES_RAY_CATALOG_ENTRY
      RAY_CHARLES_CATALOG_ENTRY
      UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }
  {
    // count=0 returns 0 results
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?count=0");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (count=0)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>0</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      "</feed>\n"
    );
  }
  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?count=1");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (count=1)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>1</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      CHARLES_RAY_CATALOG_ENTRY
      "</feed>\n"
    );
  }
  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?start=1&count=1");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (start=1&amp;count=1)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>1</startIndex>\n"
      "  <itemsPerPage>1</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }
  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?start=100&count=10");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (start=100&amp;count=10)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>100</startIndex>\n"
      "  <itemsPerPage>0</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_v2_root)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/root.xml");
  EXPECT_EQ(r->status, 200);
  const char expected_output[] = R"(<?xml version="1.0" encoding="UTF-8"?>
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:opds="https://specs.opds.io/opds-1.2">
  <id>12345678-90ab-cdef-1234-567890abcdef</id>
  <link rel="self"
        href="/ROOT%23%3F/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="start"
        href="/ROOT%23%3F/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="search"
        href="/ROOT%23%3F/catalog/v2/searchdescription.xml"
        type="application/opensearchdescription+xml"/>
  <title>OPDS Catalog Root</title>
  <updated>YYYY-MM-DDThh:mm:ssZ</updated>

  <entry>
    <title>All entries</title>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/entries"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">All entries from this catalog.</content>
  </entry>
  <entry>
    <title>All entries (partial)</title>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/partial_entries"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">All entries from this catalog in partial format.</content>
  </entry>
  <entry>
    <title>List of categories</title>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/categories"
          type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">List of all categories in this catalog.</content>
  </entry>
  <entry>
    <title>List of languages</title>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/languages"
          type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">List of all languages in this catalog.</content>
  </entry>
</feed>
)";
  EXPECT_EQ(maskVariableOPDSFeedData(r->body), expected_output);
}

TEST_F(LibraryServerTest, catalog_v2_searchdescription_xml)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/searchdescription.xml");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(r->body,
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<OpenSearchDescription xmlns=\"http://a9.com/-/spec/opensearch/1.1/\">\n"
    "  <ShortName>Zim catalog search</ShortName>\n"
    "  <Description>Search zim files in the catalog.</Description>\n"
    "  <Url type=\"application/atom+xml;profile=opds-catalog;kind=acquisition\"\n"
    "       xmlns:atom=\"http://www.w3.org/2005/Atom\"\n"
    "       xmlns:k=\"http://kiwix.org/opensearchextension/1.0\"\n"
    "       indexOffset=\"0\"\n"
    "       template=\"/ROOT%23%3F/catalog/v2/entries?q={searchTerms?}&lang={language?}&name={k:name?}&tag={k:tag?}&maxsize={k:maxsize?}&count={count?}&start={startIndex?}\"/>\n"
    "</OpenSearchDescription>\n"
  );
}

TEST_F(LibraryServerTest, catalog_v2_categories)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/categories");
  EXPECT_EQ(r->status, 200);
  const char expected_output[] = R"(<?xml version="1.0" encoding="UTF-8"?>
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:opds="https://specs.opds.io/opds-1.2">
  <id>12345678-90ab-cdef-1234-567890abcdef</id>
  <link rel="self"
        href="/ROOT%23%3F/catalog/v2/categories"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="start"
        href="/ROOT%23%3F/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <title>List of categories</title>
  <updated>YYYY-MM-DDThh:mm:ssZ</updated>

  <entry>
    <title>cats</title>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/entries?category=cats"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">All entries with category of 'cats'.</content>
  </entry>
  <entry>
    <title>jazz</title>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/entries?category=jazz"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">All entries with category of 'jazz'.</content>
  </entry>
  <entry>
    <title>wikipedia</title>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/entries?category=wikipedia"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">All entries with category of 'wikipedia'.</content>
  </entry>
</feed>
)";
  EXPECT_EQ(maskVariableOPDSFeedData(r->body), expected_output);
}

TEST_F(LibraryServerTest, catalog_v2_languages)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/languages");
  EXPECT_EQ(r->status, 200);
  const char expected_output[] = R"(<?xml version="1.0" encoding="UTF-8"?>
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:dc="http://purl.org/dc/terms/"
      xmlns:opds="https://specs.opds.io/opds-1.2"
      xmlns:thr="http://purl.org/syndication/thread/1.0">
  <id>12345678-90ab-cdef-1234-567890abcdef</id>
  <link rel="self"
        href="/ROOT%23%3F/catalog/v2/languages"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="start"
        href="/ROOT%23%3F/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <title>List of languages</title>
  <updated>YYYY-MM-DDThh:mm:ssZ</updated>

  <entry>
    <title>català</title>
    <dc:language>cat</dc:language>
    <thr:count>1</thr:count>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/entries?lang=cat"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
  </entry>
  <entry>
    <title>English</title>
    <dc:language>eng</dc:language>
    <thr:count>2</thr:count>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/entries?lang=eng"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
  </entry>
  <entry>
    <title>français</title>
    <dc:language>fra</dc:language>
    <thr:count>1</thr:count>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/entries?lang=fra"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
  </entry>
  <entry>
    <title>русский</title>
    <dc:language>rus</dc:language>
    <thr:count>1</thr:count>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/entries?lang=rus"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
  </entry>
</feed>
)";
  EXPECT_EQ(maskVariableOPDSFeedData(r->body), expected_output);
}

#define CATALOG_V2_ENTRIES_PREAMBLE0(x)                       \
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"            \
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"           \
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"          \
    "      xmlns:opds=\"https://specs.opds.io/opds-1.2\"\n"   \
    "      xmlns:opensearch=\"http://a9.com/-/spec/opensearch/1.1/\">\n"  \
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"       \
    "\n"                                                      \
    "  <link rel=\"self\"\n"                                  \
    "        href=\"/ROOT%23%3F/catalog/v2/" x "\"\n"                    \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=acquisition\"/>\n" \
    "  <link rel=\"start\"\n"                                 \
    "        href=\"/ROOT%23%3F/catalog/v2/root.xml\"\n"              \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n" \
    "  <link rel=\"up\"\n"                                    \
    "        href=\"/ROOT%23%3F/catalog/v2/root.xml\"\n"              \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n" \
    "\n"                                                      \

#define CATALOG_V2_ENTRIES_PREAMBLE(q) \
            CATALOG_V2_ENTRIES_PREAMBLE0("entries" q)

#define CATALOG_V2_PARTIAL_ENTRIES_PREAMBLE(q) \
            CATALOG_V2_ENTRIES_PREAMBLE0("partial_entries" q)

TEST_F(LibraryServerTest, catalog_v2_entries)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("")
    "  <title>All Entries</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    CHARLES_RAY_CATALOG_ENTRY
    RAY_CHARLES_CATALOG_ENTRY
    UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
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
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries");
  EXPECT_EQ(r->status, 200);

  const std::string expectedOPDS =
    CATALOG_V2_ENTRIES_PREAMBLE("")
    "  <title>All Entries</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    + fixContentLinks(CHARLES_RAY_CATALOG_ENTRY)
    + fixContentLinks(INACCESSIBLEZIMFILE_CATALOG_ENTRY)
    + fixContentLinks(RAY_CHARLES_CATALOG_ENTRY)
    + fixContentLinks(UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY) +
    "</feed>\n";

  EXPECT_EQ(maskVariableOPDSFeedData(r->body), expectedOPDS);

  { // test with empty rootLocation
    const auto fixRoot = [=](std::string s) -> std::string {
      s = replace(s, "/ROOT%23%3F/", "/");
      s = replace(s, "/ROOT%23%3F/", "/");
      return s;
    };

    ZimFileServer::Cfg serverCfg;
    serverCfg.root = "";
    serverCfg.options = ZimFileServer::CATALOG_ONLY_MODE;
    serverCfg.contentServerUrl = contentServerUrl;
    resetServer(serverCfg);

    const auto r = zfs1_->GET("/catalog/v2/entries");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body), fixRoot(expectedOPDS));
  }
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtered_by_range)
{
  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?start=1");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      CATALOG_V2_ENTRIES_PREAMBLE("?start=1")
      "  <title>Filtered Entries (start=1)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>1</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      RAY_CHARLES_CATALOG_ENTRY
      UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }

  {
    // count=-1 disables the limit on the number of results
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?count=-1");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      CATALOG_V2_ENTRIES_PREAMBLE("?count=-1")
      "  <title>Filtered Entries (count=-1)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>3</itemsPerPage>\n"
      CHARLES_RAY_CATALOG_ENTRY
      RAY_CHARLES_CATALOG_ENTRY
      UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }

  {
    // count=0 returns 0 results
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?count=0");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
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
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?count=2");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      CATALOG_V2_ENTRIES_PREAMBLE("?count=2")
      "  <title>Filtered Entries (count=2)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      CHARLES_RAY_CATALOG_ENTRY
      RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }

  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?start=1&count=1");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      CATALOG_V2_ENTRIES_PREAMBLE("?start=1&amp;count=1")
      "  <title>Filtered Entries (start=1&amp;count=1)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>1</startIndex>\n"
      "  <itemsPerPage>1</itemsPerPage>\n"
      RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtered_by_search_terms)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?q=\"ray%20charles\"");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("?q=%22ray%20charles%22")
    "  <title>Filtered Entries (q=%22ray%20charles%22)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>2</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>2</itemsPerPage>\n"
    CHARLES_RAY_CATALOG_ENTRY
    RAY_CHARLES_CATALOG_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtering_special_queries)
{
  {
  // 'or' doesn't act as a Xapian boolean operator
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?q=Or");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("?q=Or")
    "  <title>Filtered Entries (q=Or)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    CHARLES_RAY_CATALOG_ENTRY
    "</feed>\n"
  );
  }

  {
  // 'and' doesn't act as a Xapian boolean operator
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?q=and");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("?q=and")
    "  <title>Filtered Entries (q=and)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    CHARLES_RAY_CATALOG_ENTRY
    "</feed>\n"
  );
  }

  {
  // 'not' doesn't act as a Xapian boolean operator
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?q=not");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("?q=not")
    "  <title>Filtered Entries (q=not)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    RAY_CHARLES_CATALOG_ENTRY
    "</feed>\n"
  );
  }

  {
  // 'xor' doesn't act as a Xapian boolean operator
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?q=xor");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("?q=xor")
    "  <title>Filtered Entries (q=xor)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
    "</feed>\n"
  );
  }

  {
  // 'or' acts as a Xapian boolean operator
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?q=wikipedia%20or%20library");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("?q=wikipedia%20or%20library")
    "  <title>Filtered Entries (q=wikipedia%20or%20library)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    CHARLES_RAY_CATALOG_ENTRY
    "</feed>\n"
  );
  }

  {
  // 'and' acts as a Xapian boolean operator
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?q=wikipedia%20and%20articles");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("?q=wikipedia%20and%20articles")
    "  <title>Filtered Entries (q=wikipedia%20and%20articles)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    CHARLES_RAY_CATALOG_ENTRY
    "</feed>\n"
  );
  }

  {
  // 'near' doesn't act as a Xapian query operator
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?q=near");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("?q=near")
    "  <title>Filtered Entries (q=near)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    RAY_CHARLES_CATALOG_ENTRY
    "</feed>\n"
  );
  }

  {
  // 'adj' doesn't act as a Xapian query operator
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?q=adj");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("?q=adj")
    "  <title>Filtered Entries (q=adj)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
    "</feed>\n"
  );
  }

  {
  // 'near' doesn't act as a Xapian query operator
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?q=charles%20near%20why");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
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
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?q=charles%20adj%20why");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
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
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?lang=eng");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      CATALOG_V2_ENTRIES_PREAMBLE("?lang=eng")
      "  <title>Filtered Entries (lang=eng)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>2</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
      RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }

  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?lang=eng,fra");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      CATALOG_V2_ENTRIES_PREAMBLE("?lang=eng%2Cfra")
      "  <title>Filtered Entries (lang=eng%2Cfra)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>3</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>3</itemsPerPage>\n"
      CHARLES_RAY_CATALOG_ENTRY
      UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
      RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_v2_entries_filtered_by_category)
{
  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?category=jazz");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      CATALOG_V2_ENTRIES_PREAMBLE("?category=jazz")
      "  <title>Filtered Entries (category=jazz)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>1</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>1</itemsPerPage>\n"
      CHARLES_RAY_CATALOG_ENTRY
      "</feed>\n"
    );
  }

  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?category=jazz,wikipedia");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      CATALOG_V2_ENTRIES_PREAMBLE("?category=jazz%2Cwikipedia")
      "  <title>Filtered Entries (category=jazz%2Cwikipedia)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>2</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      CHARLES_RAY_CATALOG_ENTRY
      RAY_CHARLES_CATALOG_ENTRY
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_v2_entries_multiple_filters)
{
  {
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entries?lang=fra&category=jazz");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      CATALOG_V2_ENTRIES_PREAMBLE("?lang=fra&amp;category=jazz")
      "  <title>Filtered Entries (lang=fra&amp;category=jazz)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>1</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>1</itemsPerPage>\n"
      CHARLES_RAY_CATALOG_ENTRY
      "</feed>\n"
    );
  }
}

TEST_F(LibraryServerTest, catalog_v2_individual_entry_access)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entry/raycharles");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    RAY_CHARLES_CATALOG_ENTRY
  );

  const auto r1 = zfs1_->GET("/ROOT%23%3F/catalog/v2/entry/non-existent-entry");
  EXPECT_EQ(r1->status, 404);
}

TEST_F(LibraryServerTest, catalog_v2_partial_entries)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/partial_entries");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_PARTIAL_ENTRIES_PREAMBLE("")
    "  <title>All Entries</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "\n"
    "  <entry>\n"
    "    <id>urn:uuid:charlesray</id>\n"
    "    <title>Charles, Ray</title>\n"
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "    <link rel=\"alternate\"\n"
    "          href=\"/ROOT%23%3F/catalog/v2/entry/charlesray\"\n"
    "          type=\"application/atom+xml;type=entry;profile=opds-catalog\"/>\n"
    "  </entry>\n"
    "  <entry>\n"
    "    <id>urn:uuid:raycharles</id>\n"
    "    <title>Ray Charles</title>\n"
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "    <link rel=\"alternate\"\n"
    "          href=\"/ROOT%23%3F/catalog/v2/entry/raycharles\"\n"
    "          type=\"application/atom+xml;type=entry;profile=opds-catalog\"/>\n"
    "  </entry>\n"
    "  <entry>\n"
    "    <id>urn:uuid:raycharles_uncategorized</id>\n"
    "    <title>Ray (uncategorized) Charles</title>\n"
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "    <link rel=\"alternate\"\n"
    "          href=\"/ROOT%23%3F/catalog/v2/entry/raycharles_uncategorized\"\n"
    "          type=\"application/atom+xml;type=entry;profile=opds-catalog\"/>\n"
    "  </entry>\n"
    "</feed>\n"
  );
}

#define EXPECT_SEARCH_RESULTS(SEARCH_TERM, RESULT_COUNT, OPDS_ENTRIES)      \
  {                                                                         \
    const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?q=" SEARCH_TERM);       \
    EXPECT_EQ(r->status, 200);                                              \
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),                            \
      OPDS_FEED_TAG                                                         \
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"                   \
      "  <title>Filtered zims (q=" SEARCH_TERM ")</title>\n"                \
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"                         \
      "  <totalResults>" #RESULT_COUNT "</totalResults>\n"                  \
      "  <startIndex>0</startIndex>\n"                                      \
      "  <itemsPerPage>" #RESULT_COUNT "</itemsPerPage>\n"                  \
      CATALOG_LINK_TAGS                                                     \
                                                                            \
      OPDS_ENTRIES                                                          \
                                                                            \
      "</feed>\n"                                                           \
    );                                                                      \
  }

TEST_F(LibraryServerTest, catalog_search_includes_public_tags)
{
  EXPECT_SEARCH_RESULTS("public_tag_without_a_value",
                        1,
                        RAY_CHARLES_CATALOG_ENTRY
  );

  EXPECT_SEARCH_RESULTS("public_tag_with_a_value",
                        1,
                        UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
  );

  // prefix search works on tag names
  EXPECT_SEARCH_RESULTS("public_tag",
                        2,
                        UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
                        RAY_CHARLES_CATALOG_ENTRY
  );

  EXPECT_SEARCH_RESULTS("value_of_a_public_tag",
                        1,
                        UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
  );

  // prefix search works on tag values
  EXPECT_SEARCH_RESULTS("value_of",
                        1,
                        UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
  );
}

#define EXPECT_ZERO_RESULTS(SEARCH_TERM) EXPECT_SEARCH_RESULTS(SEARCH_TERM, 0, )

TEST_F(LibraryServerTest, catalog_search_on_tags_is_not_an_any_substring_match)
{
  EXPECT_ZERO_RESULTS("tag_with")
  EXPECT_ZERO_RESULTS("alue_of_a_public_tag")
}

TEST_F(LibraryServerTest, catalog_search_excludes_hidden_tags)
{
  EXPECT_ZERO_RESULTS("_private_tag_without_a_value");
  EXPECT_ZERO_RESULTS("private_tag_without_a_value");
  EXPECT_ZERO_RESULTS("value_of_a_private_tag");

#undef EXPECT_ZERO_RESULTS
}

TEST_F(LibraryServerTest, no_name_mapper_returned_catalog_use_uuid_in_link)
{
  resetServer(ZimFileServer::NO_NAME_MAPPER);
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/search?tag=_category:jazz");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    OPDS_FEED_TAG
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
    "  <title>Filtered zims (tag=_category%3Ajazz)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>1</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>1</itemsPerPage>\n"
    CATALOG_LINK_TAGS
    CHARLES_RAY_CATALOG_ENTRY_NO_MAPPER
    "</feed>\n"
  );
}


TEST_F(LibraryServerTest, no_name_mapper_catalog_v2_individual_entry_access)
{
  resetServer(ZimFileServer::NO_NAME_MAPPER);
  const auto r = zfs1_->GET("/ROOT%23%3F/catalog/v2/entry/raycharles");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    RAY_CHARLES_CATALOG_ENTRY_NO_MAPPER
  );

  const auto r1 = zfs1_->GET("/ROOT%23%3F/catalog/v2/entry/non-existent-entry");
  EXPECT_EQ(r1->status, 404);
}

#define HTML_PREAMBLE \
  "<!DOCTYPE html>\n" \
  "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n" \
  "  <head>\n" \
  "    <meta charset=\"UTF-8\" />\n" \
  "    <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" />\n" \
  "    <title>Welcome to Kiwix Server</title>\n" \
  "    <link\n" \
  "      type=\"text/css\"\n" \
  "      href=\"/ROOT%23%3F/skin/kiwix.css?cacheid=b4e29e64\"\n" \
  "      rel=\"Stylesheet\"\n" \
  "    />\n" \
  "    <link\n" \
  "      type=\"text/css\"\n" \
  "      href=\"/ROOT%23%3F/skin/index.css?cacheid=ae79e41a\"\n" \
  "      rel=\"Stylesheet\"\n" \
  "    />\n" \
  "    <link rel=\"apple-touch-icon\" sizes=\"180x180\" href=\"/ROOT%23%3F/skin/favicon/apple-touch-icon.png?cacheid=f86f8df3\">\n" \
  "    <link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"/ROOT%23%3F/skin/favicon/favicon-32x32.png?cacheid=79ded625\">\n" \
  "    <link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"/ROOT%23%3F/skin/favicon/favicon-16x16.png?cacheid=a986fedc\">\n" \
  "    <link rel=\"manifest\" href=\"/ROOT%23%3F/skin/favicon/site.webmanifest?cacheid=bc396efb\">\n" \
  "    <link rel=\"mask-icon\" href=\"/ROOT%23%3F/skin/favicon/safari-pinned-tab.svg?cacheid=8d487e95\" color=\"#5bbad5\">\n" \
  "    <link rel=\"shortcut icon\" href=\"/ROOT%23%3F/skin/favicon/favicon.ico?cacheid=92663314\">\n" \
  "    <meta name=\"msapplication-TileColor\" content=\"#da532c\">\n" \
  "    <meta name=\"msapplication-config\" content=\"/ROOT%23%3F/skin/favicon/browserconfig.xml?cacheid=f29a7c4a\">\n" \
  "    <meta name=\"theme-color\" content=\"#ffffff\">\n" \
  "    <style>\n" \
  "      @font-face {\n" \
  "        font-family: \"poppins\";\n" \
  "        src: url(\"/ROOT%23%3F/skin/fonts/Poppins.ttf?cacheid=af705837\") format(\"truetype\");\n" \
  "      }\n\n" \
  "      @font-face {\n" \
  "          font-family: \"roboto\";\n" \
  "          src: url(\"/ROOT%23%3F/skin/fonts/Roboto.ttf?cacheid=84d10248\") format(\"truetype\");\n" \
  "      }\n\n" \
  "      .book__list {\n" \
  "        display: flex;\n" \
  "        flex-direction: row;\n" \
  "        flex-wrap: wrap;\n" \
  "        align-items: center;\n" \
  "      }\n\n" \
  "      .book__wrapper:hover {\n" \
  "        transform: scale(1.0);\n" \
  "      }\n\n" \
  "      .tag__link {\n" \
  "        pointer-events: none;\n" \
  "      }\n\n" \
  "      .kiwixHomeBody__results {\n" \
  "        flex-basis: 100%;\n" \
  "      }\n\n" \
  "      #book__title>a {\n" \
  "        text-decoration: none;\n" \
  "        all: unset;\n" \
  "      }\n" \
  "    </style>\n" \
  "  </head>\n" \
  "  <body>\n" \
  "    <div class='kiwixNav'>\n"

#define CHARLES_RAY_BOOK_HTML \
  "        <div class=\"book__wrapper\">\n" \
  "            <a class=\"book__link\" href=\"/ROOT%23%3F/content/zimfile%26other\" title=\"Preview\" aria-label=\"Preview\">\n" \
  "            <div class=\"book__link__wrapper\">\n" \
  "            <div class=\"book__icon\" style=background-image:url(/ROOT%23%3F/catalog/v2/illustration/charlesray/?size=48)></div>\n" \
  "            <div class=\"book__header\">\n" \
  "                <div id=\"book__title\">Charles, Ray</div>\n" \
  "            </div>\n" \
  "            <div class=\"book__description\" title=\"Wikipedia articles about Ray Charles or why and when one should go to library\">Wikipedia articles about Ray Charles or why and when one should go to library</div>\n" \
  "            </div>\n" \
  "            </a>\n" \
  "            <div class=\"book__meta\">\n" \
  "              <div class=\"book__languageTag\" title=\"français\" aria-label=\"français\">fra</div>\n" \
  "              <div class=\"book__tags\"><div class=\"book__tags--wrapper\">\n" \
  "                  <span class=\"tag__link\" aria-label='unittest' title='unittest'>unittest</span>\n" \
  "                  <span class=\"tag__link\" aria-label='wikipedia' title='wikipedia'>wikipedia</span>\n" \
  "              </div>\n" \
  "              </div>\n" \
  "            </div>\n" \
  "            <div>\n" \
  "              <a class=\"book__download\" href=\"/ROOT%23%3F/nojs/download/zimfile%26other\">\n" \
  "                <img src=\"/ROOT%23%3F/skin/download-white.svg?cacheid=079ab989\">\n" \
  "                <span>Download</span>\n" \
  "              </a>\n" \
  "            </div>\n" \
  "        </div>\n"

#define RAY_CHARLES_BOOK_HTML \
  "        <div class=\"book__wrapper\">\n" \
  "            <a class=\"book__link\" href=\"/ROOT%23%3F/content/zimfile_raycharles\" title=\"Preview\" aria-label=\"Preview\">\n" \
  "            <div class=\"book__link__wrapper\">\n" \
  "            <div class=\"book__icon\" style=background-image:url(/ROOT%23%3F/catalog/v2/illustration/raycharles/?size=48)></div>\n" \
  "            <div class=\"book__header\">\n" \
  "                <div id=\"book__title\">Ray Charles</div>\n" \
  "            </div>\n" \
  "            <div class=\"book__description\" title=\"Wikipedia articles about Ray Charles (not all of them but near to what an average newborn may find more than enough)\">Wikipedia articles about Ray Charles (not all of them but near to what an average newborn may find more than enough)</div>\n" \
  "            </div>\n" \
  "            </a>\n" \
  "            <div class=\"book__meta\">\n" \
  "              <div class=\"book__languageTag\" title=\"English\" aria-label=\"English\">eng</div>\n" \
  "              <div class=\"book__tags\"><div class=\"book__tags--wrapper\">\n" \
  "                  <span class=\"tag__link\" aria-label='public_tag_without_a_value' title='public_tag_without_a_value'>public_tag_without_a_value</span>\n" \
  "                  <span class=\"tag__link\" aria-label='wikipedia' title='wikipedia'>wikipedia</span>\n" \
  "              </div>\n" \
  "              </div>\n" \
  "            </div>\n" \
  "            <div>\n" \
  "              <a class=\"book__download\" href=\"/ROOT%23%3F/nojs/download/zimfile_raycharles\">\n" \
  "                <img src=\"/ROOT%23%3F/skin/download-white.svg?cacheid=079ab989\">\n" \
  "                <span>Download</span>\n" \
  "              </a>\n" \
  "            </div>\n" \
  "        </div>\n"

#define RAY_CHARLES_UNCTZ_BOOK_HTML \
  "        <div class=\"book__wrapper\">\n" \
  "            <a class=\"book__link\" href=\"/ROOT%23%3F/content/zimfile_raycharles_uncategorized\" title=\"Preview\" aria-label=\"Preview\">\n" \
  "            <div class=\"book__link__wrapper\">\n" \
  "            <div class=\"book__icon\" style=background-image:url(/ROOT%23%3F/catalog/v2/illustration/raycharles_uncategorized/?size=48)></div>\n" \
  "            <div class=\"book__header\">\n" \
  "                <div id=\"book__title\">Ray (uncategorized) Charles</div>\n" \
  "            </div>\n" \
  "            <div class=\"book__description\" title=\"No category is assigned to this library entry (neither adj nor xor was considered a good option)\">No category is assigned to this library entry (neither adj nor xor was considered a good option)</div>\n" \
  "            </div>\n" \
  "            </a>\n" \
  "            <div class=\"book__meta\">\n" \
  "              <div class=\"book__languageTag\" title=\"русский,English\" aria-label=\"русский,English\">mul</div>\n" \
  "              <div class=\"book__tags\"><div class=\"book__tags--wrapper\">\n" \
  "                  <span class=\"tag__link\" aria-label='public_tag_with_a_value:value_of_a_public_tag' title='public_tag_with_a_value:value_of_a_public_tag'>public_tag_with_a_value:value_of_a_public_tag</span>\n" \
  "                  <span class=\"tag__link\" aria-label='wikipedia' title='wikipedia'>wikipedia</span>\n" \
  "              </div>\n" \
  "              </div>\n" \
  "            </div>\n" \
  "            <div>\n" \
  "              <a class=\"book__download\" href=\"/ROOT%23%3F/nojs/download/zimfile_raycharles_uncategorized\">\n" \
  "                <img src=\"/ROOT%23%3F/skin/download-white.svg?cacheid=079ab989\">\n" \
  "                <span>Download</span>\n" \
  "              </a>\n" \
  "            </div>\n" \
  "        </div>\n"

#define INACCESSIBLEZIMFILE_BOOK_HTML \
  "        <div class=\"book__wrapper\">\n" \
  "            <a class=\"book__link\" href=\"/ROOT%23%3F/content/nosuchzimfile\" title=\"Preview\" aria-label=\"Preview\">\n" \
  "            <div class=\"book__link__wrapper\">\n" \
  "            <div class=\"book__icon\" style=background-image:url(/ROOT%23%3F/catalog/v2/illustration/inaccessiblezim/?size=48)></div>\n" \
  "            <div class=\"book__header\">\n" \
  "                <div id=\"book__title\">Catalog of all catalogs</div>\n" \
  "            </div>\n" \
  "            <div class=\"book__description\" title=\"Testing that running kiwix-serve without access to ZIM files doesn&apos;t lead to a catastrophe\">Testing that running kiwix-serve without access to ZIM files doesn&apos;t lead to a catastrophe</div>\n" \
  "            </div>\n" \
  "            </a>\n" \
  "            <div class=\"book__meta\">\n" \
  "              <div class=\"book__languageTag\" title=\"català\" aria-label=\"català\">cat</div>\n" \
  "              <div class=\"book__tags\"><div class=\"book__tags--wrapper\">\n" \
  "                  <span class=\"tag__link\" aria-label='unittest' title='unittest'>unittest</span>\n" \
  "              </div>\n" \
  "              </div>\n" \
  "            </div>\n" \
  "            <div>\n" \
  "              <a class=\"book__download\" href=\"/ROOT%23%3F/nojs/download/nosuchzimfile\">\n" \
  "                <img src=\"/ROOT%23%3F/skin/download-white.svg?cacheid=079ab989\">\n" \
  "                <span>Download</span>\n" \
  "              </a>\n" \
  "            </div>\n" \
  "        </div>\n"

#define FINAL_HTML_TEXT \
  "        </div>\n" \
  "    </div>\n" \
  "    <div id=\"kiwixfooter\" class=\"kiwixfooter\">Powered by&nbsp;<a href=\"https://kiwix.org\">Kiwix</a></div>\n" \
  "    </body>\n" \
  "</html>\n"

#define FILTERS_HTML(SELECTED_ENG) \
  "      <div class=\"kiwixNav__filters\">\n" \
  "        <div class=\"kiwixNav__select\">\n" \
  "          <select name=\"lang\" id=\"languageFilter\" class='kiwixNav__kiwixFilter filter' form=\"kiwixSearchForm\">\n" \
  "            <option value=\"\" selected>All languages</option>\n" \
  "            <option value=\"cat\">català</option>\n" \
  "            <option value=\"eng\"" SELECTED_ENG ">English</option>\n" \
  "            <option value=\"fra\">français</option>\n" \
  "            <option value=\"rus\">русский</option>\n" \
  "          </select>\n" \
  "        </div>\n" \
  "        <div class=\"kiwixNav__select\">\n" \
  "          <select name=\"category\" id=\"categoryFilter\" class='kiwixNav__kiwixFilter filter' form=\"kiwixSearchForm\">\n" \
  "            <option value=\"\">All categories</option>\n" \
  "            <option value=\"cats\">Cats</option>\n" \
  "            <option value=\"jazz\">Jazz</option>\n" \
  "            <option value=\"wikipedia\">Wikipedia</option>\n" \
  "          </select>\n" \
  "        </div>\n" \
  "      </div>\n" \
  "      <form id='kiwixSearchForm' class='kiwixNav__SearchForm' action=\"/ROOT%23%3F/nojs\">\n" \
  "        <input type=\"text\" name=\"q\" accesskey=\"s\" placeholder=\"Search\" id=\"searchFilter\" class='kiwixSearch filter' value=\"\">\n" \
  "        <input type=\"submit\" class=\"kiwixButton kiwixButtonHover\" value=\"Search\"/>\n" \
  "      </form>\n" \
  "    </div>\n"

#define HOME_BODY_TEXT(X) \
  "    <div class=\"kiwixHomeBody\">\n" \
  "        \n" \
  "        <div class=\"book__list\">\n" \
  "        <h3 class=\"kiwixHomeBody__results\">" X " book(s)</h3>\n"

#define HOME_BODY_0_RESULTS \
  "    <div class=\"kiwixHomeBody\">\n" \
  "        <style>\n" \
  "          .book__list {\n" \
  "            display: none;\n" \
  "          }\n" \
  "          .kiwixHomeBody {\n" \
  "            justify-content: center;\n" \
  "          }\n" \
  "          .noResults {\n" \
  "            font-size: 16px;\n" \
  "            font-family: roboto;\n" \
  "          }\n" \
  "        </style>\n" \
  "        <div class=\"noResults\">\n" \
  "          No result. Would you like to <a href=\"?lang=\">reset filter</a>?\n" \
  "        </div>\n" \
  "        </style>\n" \
  "        <div class=\"book__list\">\n" \
  "        <h3 class=\"kiwixHomeBody__results\">0 book(s)</h3>\n" \
  "        \n"

#define RAY_CHARLES_UNCTZ_DOWNLOAD \
  "<!DOCTYPE html>\n" \
  "<html lang=\"en\">\n" \
  "<head>\n" \
  "    <meta charset=\"UTF-8\">\n" \
  "    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n" \
  "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n" \
  "    <title>Download book</title>\n" \
  "</head>\n" \
  "<style>\n" \
  "    .downloadLinksTitle {\n" \
  "        text-align: center;\n" \
  "        font-size: 32px;\n" \
  "        margin-bottom: 8px;\n" \
  "    }\n" \
  "</style>\n" \
  "<body>\n" \
  "    <div class=\"downloadLinksTitle\">\n" \
  "        Download links for <b><i>Ray (uncategorized) Charles</i></b>\n" \
  "    </div>\n" \
  "    <a href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile_raycharles_uncategorized.zim\" download>\n" \
  "        <div>Direct</div>\n" \
  "    </a>\n" \
  "    <a href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile_raycharles_uncategorized.zim.sha256\" download>\n" \
  "        <div>SHA-256 checksum</div>\n" \
  "    </a>\n" \
  "    <a href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile_raycharles_uncategorized.zim.magnet\" target=\"_blank\">\n" \
  "        <div>Magnet link</div>\n" \
  "    </a>\n" \
  "    <a href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile_raycharles_uncategorized.zim.torrent\" download>\n" \
  "        <div>BitTorrent</div>\n" \
  "    </a>\n" \
  "</body>\n" \
  "</html>"

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

#undef EXPECT_SEARCH_RESULTS
