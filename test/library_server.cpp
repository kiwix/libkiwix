
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
  void SetUp() override {
    zfs1_.reset(new ZimFileServer(PORT, "./test/library.xml"));
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
           " href=\"/ROOT/catalog/searchdescription.xml\" />\n"

#define CHARLES_RAY_CATALOG_ENTRY \
    "  <entry>\n"                                                       \
    "    <id>urn:uuid:charlesray</id>\n"                                \
    "    <title>Charles, Ray</title>\n"                                 \
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"                     \
    "    <summary>Wikipedia articles about Ray Charles</summary>\n"     \
    "    <language>fra</language>\n"                                    \
    "    <name>wikipedia_fr_ray_charles</name>\n"                       \
    "    <flavour></flavour>\n"                                         \
    "    <category>jazz</category>\n"                                   \
    "    <tags>unittest;wikipedia;_category:jazz;_pictures:no;_videos:no;_details:no;_ftindex:yes</tags>\n" \
    "    <articleCount>284</articleCount>\n"                            \
    "    <mediaCount>2</mediaCount>\n"                                  \
    "    <link type=\"text/html\" href=\"/ROOT/zimfile%26other\" />\n"               \
    "    <author>\n"                                                    \
    "      <name>Wikipedia</name>\n"                                    \
    "    </author>\n"                                                   \
    "    <publisher>\n"                                                 \
    "      <name>Kiwix</name>\n"                                        \
    "    </publisher>\n"                                                \
    "    <dc:issued>2020-03-31T00:00:00Z</dc:issued>\n"                 \
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile%26other.zim\" length=\"569344\" />\n" \
    "  </entry>\n"

#define RAY_CHARLES_CATALOG_ENTRY \
    "  <entry>\n"                                                       \
    "    <id>urn:uuid:raycharles</id>\n"                                \
    "    <title>Ray Charles</title>\n"                                  \
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"                     \
    "    <summary>Wikipedia articles about Ray Charles</summary>\n"     \
    "    <language>eng</language>\n"                                    \
    "    <name>wikipedia_en_ray_charles</name>\n"                       \
    "    <flavour></flavour>\n"                                         \
    "    <category>wikipedia</category>\n"                              \
    "    <tags>unittest;wikipedia;_category:wikipedia;_pictures:no;_videos:no;_details:no;_ftindex:yes</tags>\n" \
    "    <articleCount>284</articleCount>\n"                            \
    "    <mediaCount>2</mediaCount>\n"                                  \
    "    <link rel=\"http://opds-spec.org/image/thumbnail\"\n"          \
    "          href=\"/ROOT/catalog/v2/illustration/zimfile/?size=48\"\n" \
    "          type=\"image/png;width=48;height=48;scale=1\"/>\n"               \
    "    <link type=\"text/html\" href=\"/ROOT/zimfile\" />\n"               \
    "    <author>\n"                                                    \
    "      <name>Wikipedia</name>\n"                                    \
    "    </author>\n"                                                   \
    "    <publisher>\n"                                                 \
    "      <name>Kiwix</name>\n"                                        \
    "    </publisher>\n"                                                \
    "    <dc:issued>2020-03-31T00:00:00Z</dc:issued>\n"                 \
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile.zim\" length=\"569344\" />\n" \
    "  </entry>\n"

#define UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY \
    "  <entry>\n"                                                       \
    "    <id>urn:uuid:raycharles_uncategorized</id>\n"                  \
    "    <title>Ray (uncategorized) Charles</title>\n"                  \
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"                     \
    "    <summary>No category is assigned to this library entry.</summary>\n" \
    "    <language>rus</language>\n"                                    \
    "    <name>wikipedia_ru_ray_charles</name>\n"                       \
    "    <flavour></flavour>\n"                                         \
    "    <category></category>\n"                                \
    "    <tags>unittest;wikipedia;_pictures:no;_videos:no;_details:no</tags>\n" \
    "    <articleCount>284</articleCount>\n"                            \
    "    <mediaCount>2</mediaCount>\n"                                  \
    "    <link type=\"text/html\" href=\"/ROOT/zimfile\" />\n"               \
    "    <author>\n"                                                    \
    "      <name>Wikipedia</name>\n"                                    \
    "    </author>\n"                                                   \
    "    <publisher>\n"                                                 \
    "      <name>Kiwix</name>\n"                                        \
    "    </publisher>\n"                                                \
    "    <dc:issued>2020-03-31T00:00:00Z</dc:issued>\n"                 \
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile.zim\" length=\"125952\" />\n" \
    "  </entry>\n"

TEST_F(LibraryServerTest, catalog_root_xml)
{
  const auto r = zfs1_->GET("/ROOT/catalog/root.xml");
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
  const auto r = zfs1_->GET("/ROOT/catalog/searchdescription.xml");
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
    "       template=\"/ROOT/catalog/search?q={searchTerms?}&lang={language?}&name={k:name?}&tag={k:tag?}&notag={k:notag?}&maxsize={k:maxsize?}&count={count?}&start={startIndex?}\"/>\n"
    "</OpenSearchDescription>\n"
  );
}

TEST_F(LibraryServerTest, catalog_search_by_phrase)
{
  const auto r = zfs1_->GET("/ROOT/catalog/search?q=\"ray%20charles\"");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    OPDS_FEED_TAG
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
    "  <title>Filtered zims (q=&quot;ray charles&quot;)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>2</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>2</itemsPerPage>\n"
    CATALOG_LINK_TAGS
    RAY_CHARLES_CATALOG_ENTRY
    CHARLES_RAY_CATALOG_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_search_by_words)
{
  const auto r = zfs1_->GET("/ROOT/catalog/search?q=ray%20charles");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    OPDS_FEED_TAG
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
    "  <title>Filtered zims (q=ray charles)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>3</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>3</itemsPerPage>\n"
    CATALOG_LINK_TAGS
    RAY_CHARLES_CATALOG_ENTRY
    CHARLES_RAY_CATALOG_ENTRY
    UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_prefix_search)
{
  {
    const auto r = zfs1_->GET("/ROOT/catalog/search?q=description:ray%20description:charles");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (q=description:ray description:charles)</title>\n"
      "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
      "  <totalResults>2</totalResults>\n"
      "  <startIndex>0</startIndex>\n"
      "  <itemsPerPage>2</itemsPerPage>\n"
      CATALOG_LINK_TAGS
      RAY_CHARLES_CATALOG_ENTRY
      CHARLES_RAY_CATALOG_ENTRY
      "</feed>\n"
    );
  }
  {
    const auto r = zfs1_->GET("/ROOT/catalog/search?q=title:\"ray%20charles\"");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (q=title:&quot;ray charles&quot;)</title>\n"
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
  const auto r = zfs1_->GET("/ROOT/catalog/search?q=ray%20-uncategorized");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    OPDS_FEED_TAG
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
    "  <title>Filtered zims (q=ray -uncategorized)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>2</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>2</itemsPerPage>\n"
    CATALOG_LINK_TAGS
    RAY_CHARLES_CATALOG_ENTRY
    CHARLES_RAY_CATALOG_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_search_by_tag)
{
  const auto r = zfs1_->GET("/ROOT/catalog/search?tag=_category:jazz");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    OPDS_FEED_TAG
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
    "  <title>Filtered zims (tag=_category:jazz)</title>\n"
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
  const auto r = zfs1_->GET("/ROOT/catalog/search?category=jazz");
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

TEST_F(LibraryServerTest, catalog_search_results_pagination)
{
  {
    const auto r = zfs1_->GET("/ROOT/catalog/search?count=0");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (count=0)</title>\n"
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
    const auto r = zfs1_->GET("/ROOT/catalog/search?count=1");
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
    const auto r = zfs1_->GET("/ROOT/catalog/search?start=1&count=1");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (count=1&amp;start=1)</title>\n"
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
    const auto r = zfs1_->GET("/ROOT/catalog/search?start=100&count=10");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      OPDS_FEED_TAG
      "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"
      "  <title>Filtered zims (count=10&amp;start=100)</title>\n"
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
  const auto r = zfs1_->GET("/ROOT/catalog/v2/root.xml");
  EXPECT_EQ(r->status, 200);
  const char expected_output[] = R"(<?xml version="1.0" encoding="UTF-8"?>
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:opds="https://specs.opds.io/opds-1.2">
  <id>12345678-90ab-cdef-1234-567890abcdef</id>
  <link rel="self"
        href="/ROOT/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="start"
        href="/ROOT/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="search"
        href="/ROOT/catalog/v2/searchdescription.xml"
        type="application/opensearchdescription+xml"/>
  <title>OPDS Catalog Root</title>
  <updated>YYYY-MM-DDThh:mm:ssZ</updated>

  <entry>
    <title>All entries</title>
    <link rel="subsection"
          href="/ROOT/catalog/v2/entries"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">All entries from this catalog.</content>
  </entry>
  <entry>
    <title>All entries (partial)</title>
    <link rel="subsection"
          href="/ROOT/catalog/v2/partial_entries"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">All entries from this catalog in partial format.</content>
  </entry>
  <entry>
    <title>List of categories</title>
    <link rel="subsection"
          href="/ROOT/catalog/v2/categories"
          type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">List of all categories in this catalog.</content>
  </entry>
  <entry>
    <title>List of languages</title>
    <link rel="subsection"
          href="/ROOT/catalog/v2/languages"
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
  const auto r = zfs1_->GET("/ROOT/catalog/v2/searchdescription.xml");
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
    "       template=\"/ROOT/catalog/v2/entries?q={searchTerms?}&lang={language?}&name={k:name?}&tag={k:tag?}&maxsize={k:maxsize?}&count={count?}&start={startIndex?}\"/>\n"
    "</OpenSearchDescription>\n"
  );
}

TEST_F(LibraryServerTest, catalog_v2_categories)
{
  const auto r = zfs1_->GET("/ROOT/catalog/v2/categories");
  EXPECT_EQ(r->status, 200);
  const char expected_output[] = R"(<?xml version="1.0" encoding="UTF-8"?>
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:opds="https://specs.opds.io/opds-1.2">
  <id>12345678-90ab-cdef-1234-567890abcdef</id>
  <link rel="self"
        href="/ROOT/catalog/v2/categories"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="start"
        href="/ROOT/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <title>List of categories</title>
  <updated>YYYY-MM-DDThh:mm:ssZ</updated>

  <entry>
    <title>jazz</title>
    <link rel="subsection"
          href="/ROOT/catalog/v2/entries?category=jazz"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">All entries with category of 'jazz'.</content>
  </entry>
  <entry>
    <title>wikipedia</title>
    <link rel="subsection"
          href="/ROOT/catalog/v2/entries?category=wikipedia"
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
  const auto r = zfs1_->GET("/ROOT/catalog/v2/languages");
  EXPECT_EQ(r->status, 200);
  const char expected_output[] = R"(<?xml version="1.0" encoding="UTF-8"?>
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:dc="http://purl.org/dc/terms/"
      xmlns:opds="https://specs.opds.io/opds-1.2"
      xmlns:thr="http://purl.org/syndication/thread/1.0">
  <id>12345678-90ab-cdef-1234-567890abcdef</id>
  <link rel="self"
        href="/ROOT/catalog/v2/languages"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="start"
        href="/ROOT/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <title>List of languages</title>
  <updated>YYYY-MM-DDThh:mm:ssZ</updated>

  <entry>
    <title>English</title>
    <dc:language>eng</dc:language>
    <thr:count>1</thr:count>
    <link rel="subsection"
          href="/ROOT/catalog/v2/entries?lang=eng"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
  </entry>
  <entry>
    <title>français</title>
    <dc:language>fra</dc:language>
    <thr:count>1</thr:count>
    <link rel="subsection"
          href="/ROOT/catalog/v2/entries?lang=fra"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
  </entry>
  <entry>
    <title>русский</title>
    <dc:language>rus</dc:language>
    <thr:count>1</thr:count>
    <link rel="subsection"
          href="/ROOT/catalog/v2/entries?lang=rus"
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
    "        href=\"/ROOT/catalog/v2/" x "\"\n"                    \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=acquisition\"/>\n" \
    "  <link rel=\"start\"\n"                                 \
    "        href=\"/ROOT/catalog/v2/root.xml\"\n"              \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n" \
    "  <link rel=\"up\"\n"                                    \
    "        href=\"/ROOT/catalog/v2/root.xml\"\n"              \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n" \
    "\n"                                                      \

#define CATALOG_V2_ENTRIES_PREAMBLE(q) \
            CATALOG_V2_ENTRIES_PREAMBLE0("entries" q)

#define CATALOG_V2_PARTIAL_ENTRIES_PREAMBLE(q) \
            CATALOG_V2_ENTRIES_PREAMBLE0("partial_entries" q)

TEST_F(LibraryServerTest, catalog_v2_entries)
{
  const auto r = zfs1_->GET("/ROOT/catalog/v2/entries");
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

TEST_F(LibraryServerTest, catalog_v2_entries_filtered_by_range)
{
  {
    const auto r = zfs1_->GET("/ROOT/catalog/v2/entries?start=1");
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
    const auto r = zfs1_->GET("/ROOT/catalog/v2/entries?count=2");
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
    const auto r = zfs1_->GET("/ROOT/catalog/v2/entries?start=1&count=1");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(maskVariableOPDSFeedData(r->body),
      CATALOG_V2_ENTRIES_PREAMBLE("?count=1&start=1")
      "  <title>Filtered Entries (count=1&amp;start=1)</title>\n"
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
  const auto r = zfs1_->GET("/ROOT/catalog/v2/entries?q=\"ray%20charles\"");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    CATALOG_V2_ENTRIES_PREAMBLE("?q=%22ray%20charles%22")
    "  <title>Filtered Entries (q=&quot;ray charles&quot;)</title>\n"
    "  <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "  <totalResults>2</totalResults>\n"
    "  <startIndex>0</startIndex>\n"
    "  <itemsPerPage>2</itemsPerPage>\n"
    RAY_CHARLES_CATALOG_ENTRY
    CHARLES_RAY_CATALOG_ENTRY
    "</feed>\n"
  );
}

TEST_F(LibraryServerTest, catalog_v2_individual_entry_access)
{
  const auto r = zfs1_->GET("/ROOT/catalog/v2/entry/raycharles");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(maskVariableOPDSFeedData(r->body),
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    RAY_CHARLES_CATALOG_ENTRY
  );

  const auto r1 = zfs1_->GET("/ROOT/catalog/v2/entry/non-existent-entry");
  EXPECT_EQ(r1->status, 404);
}

TEST_F(LibraryServerTest, catalog_v2_partial_entries)
{
  const auto r = zfs1_->GET("/ROOT/catalog/v2/partial_entries");
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
    "          href=\"/ROOT/catalog/v2/entry/charlesray\"\n"
    "          type=\"application/atom+xml;type=entry;profile=opds-catalog\"/>\n"
    "  </entry>\n"
    "  <entry>\n"
    "    <id>urn:uuid:raycharles</id>\n"
    "    <title>Ray Charles</title>\n"
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "    <link rel=\"alternate\"\n"
    "          href=\"/ROOT/catalog/v2/entry/raycharles\"\n"
    "          type=\"application/atom+xml;type=entry;profile=opds-catalog\"/>\n"
    "  </entry>\n"
    "  <entry>\n"
    "    <id>urn:uuid:raycharles_uncategorized</id>\n"
    "    <title>Ray (uncategorized) Charles</title>\n"
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"
    "    <link rel=\"alternate\"\n"
    "          href=\"/ROOT/catalog/v2/entry/raycharles_uncategorized\"\n"
    "          type=\"application/atom+xml;type=entry;profile=opds-catalog\"/>\n"
    "  </entry>\n"
    "</feed>\n"
  );
}