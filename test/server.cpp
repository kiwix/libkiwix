
#include "./httplib.h"
#include "gtest/gtest.h"

#include "../include/manager.h"
#include "../include/server.h"
#include "../include/name_mapper.h"
#include "../include/tools.h"

using TestContextImpl = std::vector<std::pair<std::string, std::string> >;
struct TestContext : TestContextImpl {
  TestContext(const std::initializer_list<value_type>& il)
    : TestContextImpl(il)
  {}
};

std::ostream& operator<<(std::ostream& out, const TestContext& ctx)
{
  out << "Test context:\n";
  for ( const auto& kv : ctx )
    out << "\t" << kv.first << ": " << kv.second << "\n";
  out << std::endl;
  return out;
}

bool is_valid_etag(const std::string& etag)
{
  return etag.size() >= 2 &&
         etag.front() == '"' &&
         etag.back() == '"';
}

template<class T1, class T2>
T1 concat(T1 a, const T2& b)
{
  a.insert(a.end(), b.begin(), b.end());
  return a;
}

typedef httplib::Headers Headers;

Headers invariantHeaders(Headers headers)
{
  headers.erase("Date");
  return headers;
}

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


class ZimFileServer
{
public: // types
  typedef std::shared_ptr<httplib::Response>  Response;
  typedef std::vector<std::string> FilePathCollection;

public: // functions
  ZimFileServer(int serverPort, std::string libraryFilePath);
  ZimFileServer(int serverPort, const FilePathCollection& zimpaths, std::string indexTemplateString = "");
  ~ZimFileServer();

  Response GET(const char* path, const Headers& headers = Headers())
  {
    return client->Get(path, headers);
  }

  Response HEAD(const char* path, const Headers& headers = Headers())
  {
    return client->Head(path, headers);
  }

private:
  void run(int serverPort, std::string indexTemplateString = "");

private: // data
  kiwix::Library library;
  kiwix::Manager manager;
  std::unique_ptr<kiwix::HumanReadableNameMapper> nameMapper;
  std::unique_ptr<kiwix::Server> server;
  std::unique_ptr<httplib::Client> client;
};

ZimFileServer::ZimFileServer(int serverPort, std::string libraryFilePath)
: manager(&this->library)
{
  if ( kiwix::isRelativePath(libraryFilePath) )
    libraryFilePath = kiwix::computeAbsolutePath(kiwix::getCurrentDirectory(), libraryFilePath);
  manager.readFile(libraryFilePath, true, true);
  run(serverPort);
}

ZimFileServer::ZimFileServer(int serverPort, const FilePathCollection& zimpaths, std::string indexTemplateString)
: manager(&this->library)
{
  for ( const auto& zimpath : zimpaths ) {
    if (!manager.addBookFromPath(zimpath, zimpath, "", false))
      throw std::runtime_error("Unable to add the ZIM file '" + zimpath + "'");
  }
  run(serverPort, indexTemplateString);
}

void ZimFileServer::run(int serverPort, std::string indexTemplateString)
{
  const std::string address = "127.0.0.1";
  nameMapper.reset(new kiwix::HumanReadableNameMapper(library, false));
  server.reset(new kiwix::Server(&library, nameMapper.get()));
  server->setRoot("ROOT");
  server->setAddress(address);
  server->setPort(serverPort);
  server->setNbThreads(2);
  server->setVerbose(false);
  if (!indexTemplateString.empty()) {
    server->setIndexTemplateString(indexTemplateString);
  }

  if ( !server->start() )
    throw std::runtime_error("ZimFileServer failed to start");

  client.reset(new httplib::Client(address, serverPort));
}

ZimFileServer::~ZimFileServer()
{
  server->stop();
}

class ServerTest : public ::testing::Test
{
protected:
  std::unique_ptr<ZimFileServer>   zfs1_;

  const int PORT = 8001;
  const ZimFileServer::FilePathCollection ZIMFILES {
    "./test/zimfile.zim",
    "./test/corner_cases.zim"
  };

protected:
  void SetUp() override {
    zfs1_.reset(new ZimFileServer(PORT, ZIMFILES));
  }

  void TearDown() override {
    zfs1_.reset();
  }
};

const bool WITH_ETAG = true;
const bool NO_ETAG = false;

struct Resource
{
  bool etag_expected;
  const char* url;
};

std::ostream& operator<<(std::ostream& out, const Resource& r)
{
  out << "url: " << r.url;
  return out;
}

typedef std::vector<Resource> ResourceCollection;

const ResourceCollection resources200Compressible{
  { WITH_ETAG, "/ROOT/" },

  { WITH_ETAG, "/ROOT/skin/jquery-ui/jquery-ui.structure.min.css" },
  { WITH_ETAG, "/ROOT/skin/jquery-ui/jquery-ui.min.js" },
  { WITH_ETAG, "/ROOT/skin/jquery-ui/external/jquery/jquery.js" },
  { WITH_ETAG, "/ROOT/skin/jquery-ui/jquery-ui.theme.min.css" },
  { WITH_ETAG, "/ROOT/skin/jquery-ui/jquery-ui.min.css" },
  { WITH_ETAG, "/ROOT/skin/taskbar.js" },
  { WITH_ETAG, "/ROOT/skin/taskbar.css" },
  { WITH_ETAG, "/ROOT/skin/block_external.js" },

  { NO_ETAG,   "/ROOT/catalog/root.xml" },
  { NO_ETAG,   "/ROOT/catalog/searchdescription.xml" },
  { NO_ETAG,   "/ROOT/catalog/search" },

  { NO_ETAG,   "/ROOT/search?content=zimfile&pattern=a" },

  { NO_ETAG,   "/ROOT/suggest?content=zimfile" },
  { NO_ETAG,   "/ROOT/suggest?content=zimfile&term=ray" },

  { NO_ETAG,   "/ROOT/catch/external?source=www.example.com" },

  { WITH_ETAG, "/ROOT/zimfile/A/index" },
  { WITH_ETAG, "/ROOT/zimfile/A/Ray_Charles" },

  { WITH_ETAG, "/ROOT/raw/zimfile/content/A/index" },
  { WITH_ETAG, "/ROOT/raw/zimfile/content/A/Ray_Charles" },
};

const ResourceCollection resources200Uncompressible{
  { WITH_ETAG, "/ROOT/skin/jquery-ui/images/animated-overlay.gif" },
  { WITH_ETAG, "/ROOT/skin/caret.png" },

  { WITH_ETAG, "/ROOT/raw/zimfile/meta/Title" },
  { WITH_ETAG, "/ROOT/raw/zimfile/meta/Description" },
  { WITH_ETAG, "/ROOT/raw/zimfile/meta/Language" },
  { WITH_ETAG, "/ROOT/raw/zimfile/meta/Name" },
  { WITH_ETAG, "/ROOT/raw/zimfile/meta/Tags" },
  { WITH_ETAG, "/ROOT/raw/zimfile/meta/Date" },
  { WITH_ETAG, "/ROOT/raw/zimfile/meta/Creator" },
  { WITH_ETAG, "/ROOT/raw/zimfile/meta/Publisher" },

  { NO_ETAG, "/ROOT/catalog/v2/illustration/zimfile?size=48" },

  { WITH_ETAG, "/ROOT/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg" },

  { WITH_ETAG, "/ROOT/corner_cases/A/empty.html" },
  { WITH_ETAG, "/ROOT/corner_cases/-/empty.css" },
  { WITH_ETAG, "/ROOT/corner_cases/-/empty.js" },

  // The title and creator are too small to be compressed
  { WITH_ETAG, "/ROOT/raw/zimfile/meta/Creator" },
  { WITH_ETAG, "/ROOT/raw/zimfile/meta/Title" },
};

ResourceCollection all200Resources()
{
  return concat(resources200Compressible, resources200Uncompressible);
}

TEST(indexTemplateStringTest, emptyIndexTemplate) {
  const int PORT = 8001;
  const ZimFileServer::FilePathCollection ZIMFILES {
    "./test/zimfile.zim",
    "./test/corner_cases.zim"
  };

  ZimFileServer zfs(PORT, ZIMFILES, "");
  EXPECT_EQ(200, zfs.GET("/ROOT/")->status);
}

TEST(indexTemplateStringTest, indexTemplateCheck) {
  const int PORT = 8001;
  const ZimFileServer::FilePathCollection ZIMFILES {
    "./test/zimfile.zim",
    "./test/corner_cases.zim"
  };

  ZimFileServer zfs(PORT, ZIMFILES, "<!DOCTYPE html><head>"
      "<title>Welcome to kiwix library</title>"
    "</head>"
  "</html>");
  EXPECT_EQ("<!DOCTYPE html><head>"
    "<title>Welcome to kiwix library</title>"
    "<link type=\"root\" href=\"/ROOT\">"
    "</head>"
  "</html>", zfs.GET("/ROOT/")->body);
}

TEST_F(ServerTest, 200)
{
  for ( const Resource& res : all200Resources() )
    EXPECT_EQ(200, zfs1_->GET(res.url)->status) << "res.url: " << res.url;
}

TEST_F(ServerTest, CompressibleContentIsCompressedIfAcceptable)
{
  for ( const Resource& res : resources200Compressible ) {
    const auto x = zfs1_->GET(res.url, { {"Accept-Encoding", "deflate"} });
    EXPECT_EQ(200, x->status) << res;
    EXPECT_EQ("deflate", x->get_header_value("Content-Encoding")) << res;
    EXPECT_EQ("Accept-Encoding", x->get_header_value("Vary")) << res;
  }
}

TEST_F(ServerTest, UncompressibleContentIsNotCompressed)
{
  for ( const Resource& res : resources200Uncompressible ) {
    const auto x = zfs1_->GET(res.url, { {"Accept-Encoding", "deflate"} });
    EXPECT_EQ(200, x->status) << res;
    EXPECT_EQ("", x->get_header_value("Content-Encoding")) << res;
  }
}

const char* urls400[] = {
  "/ROOT/search",
  "/ROOT/search?content=zimfile",
  "/ROOT/search?content=non-existing-book&pattern=asdfqwerty",
  "/ROOT/search?content=non-existing-book&pattern=asd<qwerty",
  "/ROOT/search?pattern"
};


TEST_F(ServerTest, 400)
{
  for (const char* url: urls400 ) {
    EXPECT_EQ(400, zfs1_->GET(url)->status) << "url: " << url;
  }
}

const char* urls404[] = {
  "/",
  "/zimfile",
  "/ROOT/non-existent-item",
  "/ROOT/skin/non-existent-skin-resource",
  "/ROOT/catalog",
  "/ROOT/catalog/non-existent-item",
  "/ROOT/catalogBLABLABLA/root.xml",
  "/ROOT/catalog/v2/illustration/zimfile?size=96",
  "/ROOT/meta",
  "/ROOT/meta?content=zimfile",
  "/ROOT/meta?content=zimfile&name=non-existent-item",
  "/ROOT/meta?content=non-existent-book&name=title",
  "/ROOT/random",
  "/ROOT/random?content=non-existent-book",
  "/ROOT/suggest",
  "/ROOT/suggest?content=non-existent-book&term=abcd",
  "/ROOT/catch/external",
  "/ROOT/zimfile/A/non-existent-article",

  "/ROOT/raw/non-existent-book/meta/Title",
  "/ROOT/raw/zimfile/wrong-kind/Foo",

  // zimfile has no Favicon nor Illustration_48x48@1 meta item
  "/ROOT/raw/zimfile/meta/Favicon",
  "/ROOT/raw/zimfile/meta/Illustration_48x48@1",
};

TEST_F(ServerTest, 404)
{
  for ( const char* url : urls404 ) {
    EXPECT_EQ(404, zfs1_->GET(url)->status) << "url: " << url;
  }
}

namespace TestingOfHtmlResponses
{

struct ExpectedResponseData
{
  const std::string expectedPageTitle;
  const std::string expectedCssUrl;
  const std::string bookName;
  const std::string bookTitle;
  const std::string expectedBody;
};

enum ExpectedResponseDataType
{
  expected_page_title,
  expected_css_url,
  book_name,
  book_title,
  expected_body
};

// Operator overloading is used as a means of defining a mini-DSL for
// defining test data in a concise way (see usage in
// TEST_F(ServerTest, 404WithBodyTesting))
ExpectedResponseData operator==(ExpectedResponseDataType t, std::string s)
{
  switch (t)
  {
    case expected_page_title: return ExpectedResponseData{s, "", "", "", ""};
    case expected_css_url:    return ExpectedResponseData{"", s, "", "", ""};
    case book_name:           return ExpectedResponseData{"", "", s, "", ""};
    case book_title:          return ExpectedResponseData{"", "", "", s, ""};
    case expected_body:       return ExpectedResponseData{"", "", "", "", s};
    default: assert(false); return ExpectedResponseData{};
  }
}

std::string selectNonEmpty(const std::string& a, const std::string& b)
{
  if ( a.empty() ) return b;

  assert(b.empty());
  return a;
}

ExpectedResponseData operator&&(const ExpectedResponseData& a,
                                const ExpectedResponseData& b)
{
  return ExpectedResponseData{
    selectNonEmpty(a.expectedPageTitle, b.expectedPageTitle),
    selectNonEmpty(a.expectedCssUrl, b.expectedCssUrl),
    selectNonEmpty(a.bookName, b.bookName),
    selectNonEmpty(a.bookTitle, b.bookTitle),
    selectNonEmpty(a.expectedBody, b.expectedBody)
  };
}

class TestContentIn404HtmlResponse : public ExpectedResponseData
{
public:
  TestContentIn404HtmlResponse(const std::string& url,
                               const ExpectedResponseData& erd)
    : ExpectedResponseData(erd)
    , url(url)
  {}
  virtual ~TestContentIn404HtmlResponse() = default;

  const std::string url;

  std::string expectedResponse() const;

private:
  virtual std::string pageTitle() const;
  std::string pageCssLink() const;
  std::string hiddenBookNameInput() const;
  std::string searchPatternInput() const;
  std::string taskbarLinks() const;
};

std::string TestContentIn404HtmlResponse::expectedResponse() const
{
  const std::string frag[] =  {
    R"FRAG(<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta content="text/html;charset=UTF-8" http-equiv="content-type" />
    <title>)FRAG",

    R"FRAG(</title>
)FRAG",

    R"FRAG(  <link type="root" href="/ROOT"><link type="text/css" href="/ROOT/skin/jquery-ui/jquery-ui.min.css" rel="Stylesheet" />
<link type="text/css" href="/ROOT/skin/jquery-ui/jquery-ui.theme.min.css" rel="Stylesheet" />
<link type="text/css" href="/ROOT/skin/taskbar.css" rel="Stylesheet" />
<script type="text/javascript" src="/ROOT/skin/jquery-ui/external/jquery/jquery.js" defer></script>
<script type="text/javascript" src="/ROOT/skin/jquery-ui/jquery-ui.min.js" defer></script>
<script type="text/javascript" src="/ROOT/skin/taskbar.js" defer></script>
</head>
  <body><span class="kiwix">
  <span id="kiwixtoolbar" class="ui-widget-header">
    <div class="kiwix_centered">
      <div class="kiwix_searchform">
        <form class="kiwixsearch" method="GET" action="/ROOT/search" id="kiwixsearchform">
          )FRAG",

  R"FRAG(
          <label for="kiwixsearchbox">&#x1f50d;</label>
)FRAG",

  R"FRAG(        </form>
      </div>
        <input type="checkbox" id="kiwix_button_show_toggle">
        <label for="kiwix_button_show_toggle"><img src="/ROOT/skin/caret.png" alt=""></label>
        <div class="kiwix_button_cont">
            <a id="kiwix_serve_taskbar_library_button" title="Go to welcome page" aria-label="Go to welcome page" href="/ROOT/"><button>&#x1f3e0;</button></a>
          )FRAG",

  R"FRAG(
        </div>
    </div>
  </span>
</span>
)FRAG",

  R"FRAG(  </body>
</html>
)FRAG"
  };

  return frag[0]
       + pageTitle()
       + frag[1]
       + pageCssLink()
       + frag[2]
       + hiddenBookNameInput()
       + frag[3]
       + searchPatternInput()
       + frag[4]
       + taskbarLinks()
       + frag[5]
       + removeEOLWhitespaceMarkers(expectedBody)
       + frag[6];
}

std::string TestContentIn404HtmlResponse::pageTitle() const
{
  return expectedPageTitle.empty()
       ? "Content not found"
       : expectedPageTitle;
}

std::string TestContentIn404HtmlResponse::pageCssLink() const
{
  if ( expectedCssUrl.empty() )
    return "";

  return R"(    <link type="text/css" href=")"
       + expectedCssUrl
       + R"(" rel="Stylesheet" />
)";
}

std::string TestContentIn404HtmlResponse::hiddenBookNameInput() const
{
  return bookName.empty()
       ? ""
       : R"(<input type="hidden" name="content" value=")" + bookName + R"(" />)";
}

std::string TestContentIn404HtmlResponse::searchPatternInput() const
{
  return R"(          <input autocomplete="off" class="ui-autocomplete-input" id="kiwixsearchbox" name="pattern" type="text" title="Search ')"
       + bookTitle
       + R"('" aria-label="Search ')"
       + bookTitle
       + R"('">
)";
}

std::string TestContentIn404HtmlResponse::taskbarLinks() const
{
  if ( bookName.empty() )
    return "";

  return R"(<a id="kiwix_serve_taskbar_home_button" title="Go to the main page of ')"
       + bookTitle
       + R"('" aria-label="Go to the main page of ')"
       + bookTitle
       + R"('" href="/ROOT/)"
       + bookName
       + R"(/"><button>)"
       + bookTitle
       + R"(</button></a>
          <a id="kiwix_serve_taskbar_random_button" title="Go to a randomly selected page" aria-label="Go to a randomly selected page"
            href="/ROOT/random?content=)"
       + bookName
       + R"("><button>&#x1F3B2;</button></a>)";
}

class TestContentIn400HtmlResponse : public TestContentIn404HtmlResponse
{
public:
  TestContentIn400HtmlResponse(const std::string& url,
                               const ExpectedResponseData& erd)
    : TestContentIn404HtmlResponse(url, erd)
  {}

private:
  std::string pageTitle() const;
};

std::string TestContentIn400HtmlResponse::pageTitle() const {
  return expectedPageTitle.empty()
     ? "Invalid request"
     : expectedPageTitle;
}


} // namespace TestingOfHtmlResponses

TEST_F(ServerTest, 404WithBodyTesting)
{
  using namespace TestingOfHtmlResponses;
  const std::vector<TestContentIn404HtmlResponse> testData{
    { /* url */ "/ROOT/random?content=non-existent-book",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      No such book: non-existent-book
    </p>
)"  },

    { /* url */ "/ROOT/suggest?content=no-such-book&term=whatever",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      No such book: no-such-book
    </p>
)"  },

    { /* url */ "/ROOT/catalog/",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT/catalog/" was not found on this server.
    </p>
)"  },

    { /* url */ "/ROOT/catalog/invalid_endpoint",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT/catalog/invalid_endpoint" was not found on this server.
    </p>
)"  },

    { /* url */ "/ROOT/invalid-book/whatever",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT/invalid-book/whatever" was not found on this server.
    </p>
    <p>
      Make a full text search for <a href="/ROOT/search?pattern=whatever">whatever</a>
    </p>
)"  },

    { /* url */ "/ROOT/zimfile/invalid-article",
      book_name=="zimfile" &&
      book_title=="Ray Charles" &&
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT/zimfile/invalid-article" was not found on this server.
    </p>
    <p>
      Make a full text search for <a href="/ROOT/search?content=zimfile&pattern=invalid-article">invalid-article</a>
    </p>
)"  },

    { /* url */ R"(/ROOT/"><svg onload=alert(1)>)",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT/&quot;&gt;&lt;svg onload=alert(1)&gt;" was not found on this server.
    </p>
    <p>
      Make a full text search for <a href="/ROOT/search?pattern=%22%3E%3Csvg%20onload%3Dalert(1)%3E">&quot;&gt;&lt;svg onload=alert(1)&gt;</a>
    </p>
)"  },

    { /* url */ R"(/ROOT/zimfile/"><svg onload=alert(1)>)",
      book_name=="zimfile" &&
      book_title=="Ray Charles" &&
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT/zimfile/&quot;&gt;&lt;svg onload=alert(1)&gt;" was not found on this server.
    </p>
    <p>
      Make a full text search for <a href="/ROOT/search?content=zimfile&pattern=%22%3E%3Csvg%20onload%3Dalert(1)%3E">&quot;&gt;&lt;svg onload=alert(1)&gt;</a>
    </p>
)"  },

    { /* url */ "/ROOT/raw/no-such-book/meta/Title",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT/raw/no-such-book/meta/Title" was not found on this server.
    </p>
    <p>
      No such book: no-such-book
    </p>
)"  },

    { /* url */ "/ROOT/raw/zimfile/XYZ",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT/raw/zimfile/XYZ" was not found on this server.
    </p>
    <p>
      XYZ is not a valid request for raw content.
    </p>
)"  },

    { /* url */ "/ROOT/raw/zimfile/meta/invalid-metadata",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT/raw/zimfile/meta/invalid-metadata" was not found on this server.
    </p>
    <p>
      Cannot find meta entry invalid-metadata
    </p>
)"  },

    { /* url */ "/ROOT/raw/zimfile/content/invalid-article",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT/raw/zimfile/content/invalid-article" was not found on this server.
    </p>
    <p>
      Cannot find content entry invalid-article
    </p>
)"  },
  };

  for ( const auto& t : testData ) {
    const TestContext ctx{ {"url", t.url} };
    const auto r = zfs1_->GET(t.url.c_str());
    EXPECT_EQ(r->status, 404) << ctx;
    EXPECT_EQ(r->body, t.expectedResponse()) << ctx;
  }
}

TEST_F(ServerTest, 400WithBodyTesting)
{
  using namespace TestingOfHtmlResponses;
  const std::vector<TestContentIn400HtmlResponse> testData{
    { /* url */ "/ROOT/search",
      expected_body== R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT/search" is not a valid request.
    </p>
    <p>
      No query provided.
    </p>
)"  },
    { /* url */ "/ROOT/search?content=zimfile",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT/search?content=zimfile" is not a valid request.
    </p>
    <p>
      No query provided.
    </p>
)"  },
    { /* url */ "/ROOT/search?content=non-existing-book&pattern=asdfqwerty",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT/search?content=non-existing-book&pattern=asdfqwerty" is not a valid request.
    </p>
    <p>
      The requested book doesn't exist.
    </p>
)"  },
    { /* url */ "/ROOT/search?content=non-existing-book&pattern=a\"<script foo>",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT/search?content=non-existing-book&pattern=a"&lt;script foo&gt;" is not a valid request.
    </p>
    <p>
      The requested book doesn't exist.
    </p>
)"  },
    // There is a flaw in our way to handle query string, we cannot differenciate
    // between `pattern` and `pattern=`
    { /* url */ "/ROOT/search?pattern",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT/search?pattern=" is not a valid request.
    </p>
    <p>
      No query provided.
    </p>
)"  },
  };

  for ( const auto& t : testData ) {
    const TestContext ctx{ {"url", t.url} };
    const auto r = zfs1_->GET(t.url.c_str());
    EXPECT_EQ(r->status, 400) << ctx;
    EXPECT_EQ(r->body, t.expectedResponse()) << ctx;
  }
}

TEST_F(ServerTest, RandomPageRedirectsToAnExistingArticle)
{
  auto g = zfs1_->GET("/ROOT/random?content=zimfile");
  ASSERT_EQ(302, g->status);
  ASSERT_TRUE(g->has_header("Location"));
  ASSERT_TRUE(g->get_header_value("Location").find("/zimfile/A/") != std::string::npos);
}

TEST_F(ServerTest, BookMainPageIsRedirectedToArticleIndex)
{
  auto g = zfs1_->GET("/ROOT/zimfile");
  ASSERT_EQ(302, g->status);
  ASSERT_TRUE(g->has_header("Location"));
  ASSERT_EQ("/ROOT/zimfile/A/index", g->get_header_value("Location"));
}


TEST_F(ServerTest, RawEntry)
{
  auto p = zfs1_->GET("/ROOT/raw/zimfile/meta/Title");
  EXPECT_EQ(200, p->status);
  EXPECT_EQ(p->body, std::string("Ray Charles"));

  p = zfs1_->GET("/ROOT/raw/zimfile/meta/Creator");
  EXPECT_EQ(200, p->status);
  EXPECT_EQ(p->body, std::string("Wikipedia"));

  // The raw content of Ray_Charles returned by the server is
  // the same as the one in the zim file.
  auto archive = zim::Archive("./test/zimfile.zim");
  auto entry = archive.getEntryByPath("A/Ray_Charles");
  p = zfs1_->GET("/ROOT/raw/zimfile/content/A/Ray_Charles");
  EXPECT_EQ(200, p->status);
  EXPECT_EQ(std::string(p->body), std::string(entry.getItem(true).getData()));

  // ... but the "normal" content is not
  p = zfs1_->GET("/ROOT/zimfile/A/Ray_Charles");
  EXPECT_EQ(200, p->status);
  EXPECT_NE(std::string(p->body), std::string(entry.getItem(true).getData()));
  EXPECT_TRUE(p->body.find("taskbar") != std::string::npos);
}

TEST_F(ServerTest, HeadMethodIsSupported)
{
  for ( const Resource& res : all200Resources() ) {
    EXPECT_EQ(200, zfs1_->HEAD(res.url)->status) << res;
  }
}

TEST_F(ServerTest, TheResponseToHeadRequestHasNoBody)
{
  for ( const Resource& res : all200Resources() ) {
    EXPECT_TRUE(zfs1_->HEAD(res.url)->body.empty()) << res;
  }
}

TEST_F(ServerTest, HeadersAreTheSameInResponsesToHeadAndGetRequests)
{
  for ( const Resource& res : all200Resources() ) {
    httplib::Headers g = zfs1_->GET(res.url)->headers;
    httplib::Headers h = zfs1_->HEAD(res.url)->headers;
    EXPECT_EQ(invariantHeaders(g), invariantHeaders(h)) << res;
  }
}

TEST_F(ServerTest, ETagHeaderIsSetAsNeeded)
{
  for ( const Resource& res : all200Resources() ) {
    const auto responseToGet = zfs1_->GET(res.url);
    EXPECT_EQ(res.etag_expected, responseToGet->has_header("ETag")) << res;
    if ( res.etag_expected ) {
      EXPECT_TRUE(is_valid_etag(responseToGet->get_header_value("ETag")));
    }
  }
}

TEST_F(ServerTest, ETagIsTheSameInResponsesToDifferentRequestsOfTheSameURL)
{
  for ( const Resource& res : all200Resources() ) {
    const auto h1 = zfs1_->HEAD(res.url);
    const auto h2 = zfs1_->HEAD(res.url);
    EXPECT_EQ(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
  }
}

TEST_F(ServerTest, ETagIsTheSameAcrossHeadAndGet)
{
  for ( const Resource& res : all200Resources() ) {
    const auto g = zfs1_->GET(res.url);
    const auto h = zfs1_->HEAD(res.url);
    EXPECT_EQ(h->get_header_value("ETag"), g->get_header_value("ETag"));
  }
}

TEST_F(ServerTest, DifferentServerInstancesProduceDifferentETags)
{
  ZimFileServer zfs2(PORT + 1, ZIMFILES);
  for ( const Resource& res : all200Resources() ) {
    if ( !res.etag_expected ) continue;
    const auto h1 = zfs1_->HEAD(res.url);
    const auto h2 = zfs2.HEAD(res.url);
    EXPECT_NE(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
  }
}

TEST_F(ServerTest, CompressionInfluencesETag)
{
  for ( const Resource& res : resources200Compressible ) {
    if ( ! res.etag_expected ) continue;
    const auto g1 = zfs1_->GET(res.url);
    const auto g2 = zfs1_->GET(res.url, { {"Accept-Encoding", ""} } );
    const auto g3 = zfs1_->GET(res.url, { {"Accept-Encoding", "deflate"} } );
    const auto etag = g1->get_header_value("ETag");
    EXPECT_EQ(etag, g2->get_header_value("ETag"));
    EXPECT_NE(etag, g3->get_header_value("ETag"));
  }
}

TEST_F(ServerTest, ETagOfUncompressibleContentIsNotAffectedByAcceptEncoding)
{
  for ( const Resource& res : resources200Uncompressible ) {
    if ( ! res.etag_expected ) continue;
    const auto g1 = zfs1_->GET(res.url);
    const auto g2 = zfs1_->GET(res.url, { {"Accept-Encoding", ""} } );
    const auto g3 = zfs1_->GET(res.url, { {"Accept-Encoding", "deflate"} } );
    const auto etag = g1->get_header_value("ETag");
    EXPECT_EQ(etag, g2->get_header_value("ETag")) << res;
    EXPECT_EQ(etag, g3->get_header_value("ETag")) << res;
  }
}

// Pick from the response those headers that are required to be present in the
// 304 (Not Modified) response if they would be set in the 200 (OK) response.
// NOTE: The "Date" header (which should belong to that list as required
// NOTE: by RFC 7232) is not included (since the result of this function
// NOTE: will be used to check the equality of headers from the 200 and 304
// NOTE: responses).
Headers special304Headers(const httplib::Response& r)
{
  Headers result;
  std::copy_if(
    r.headers.begin(), r.headers.end(),
    std::inserter(result, result.end()),
    [](const Headers::value_type& x) {
       return x.first == "Cache-Control"
           || x.first == "Content-Location"
           || x.first == "ETag"
           || x.first == "Expires"
           || x.first == "Vary";
  });
  return result;
}

// make a list of three etags with the given one in the middle
std::string make_etag_list(const std::string& etag)
{
  return "\"x" + etag.substr(1) + ", "
       + etag + ", "
       + etag.substr(0, etag.size()-2) + "\"";
}

TEST_F(ServerTest, IfNoneMatchRequestsWithMatchingETagResultIn304Responses)
{
  const char* const encodings[] = { "", "deflate" };
  for ( const Resource& res : all200Resources() ) {
    for ( const char* enc: encodings ) {
      if ( ! res.etag_expected ) continue;
      const TestContext ctx{ {"url", res.url}, {"encoding", enc} };

      const auto g = zfs1_->GET(res.url, { {"Accept-Encoding", enc} });
      const auto etag = g->get_header_value("ETag");

      const std::string etags = make_etag_list(etag);
      const Headers headers{{"If-None-Match", etags}, {"Accept-Encoding", enc}};
      const auto g2 = zfs1_->GET(res.url, headers );
      const auto h = zfs1_->HEAD(res.url, headers );
      EXPECT_EQ(304, h->status) << ctx;
      EXPECT_EQ(304, g2->status) << ctx;
      EXPECT_EQ(special304Headers(*g), special304Headers(*g2)) << ctx;
      EXPECT_EQ(special304Headers(*g2), special304Headers(*h)) << ctx;
      EXPECT_TRUE(g2->body.empty()) << ctx;
    }
  }
}

TEST_F(ServerTest, IfNoneMatchRequestsWithMismatchingETagResultIn200Responses)
{
  for ( const Resource& res : all200Resources() ) {
    const auto g = zfs1_->GET(res.url);
    const auto etag = g->get_header_value("ETag");
    const auto etag2 = etag.substr(0, etag.size() - 1) + "x\"";
    const auto h = zfs1_->HEAD(res.url, { {"If-None-Match", etag2} } );
    const auto g2 = zfs1_->GET(res.url, { {"If-None-Match", etag2} } );
    EXPECT_EQ(200, h->status);
    EXPECT_EQ(200, g2->status);
  }
}

TEST_F(ServerTest, ValidSingleRangeByteRangeRequestsAreHandledProperly)
{
  const char url[] = "/ROOT/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";
  const auto full = zfs1_->GET(url);
  EXPECT_FALSE(full->has_header("Content-Range"));
  EXPECT_EQ("bytes", full->get_header_value("Accept-Ranges"));

  {
    const auto p = zfs1_->GET(url, { {"Range", "bytes=0-100000"} } );
    EXPECT_EQ(206, p->status);
    EXPECT_EQ(full->body, p->body);
    EXPECT_EQ("bytes 0-20076/20077", p->get_header_value("Content-Range"));
    EXPECT_EQ("bytes", p->get_header_value("Accept-Ranges"));
  }

  {
    const auto p = zfs1_->GET(url, { {"Range", "bytes=0-10"} } );
    EXPECT_EQ(206, p->status);
    EXPECT_EQ("bytes 0-10/20077", p->get_header_value("Content-Range"));
    EXPECT_EQ(11U, p->body.size());
    EXPECT_EQ(full->body.substr(0, 11), p->body);
    EXPECT_EQ("bytes", p->get_header_value("Accept-Ranges"));
  }

  {
    const auto p = zfs1_->GET(url, { {"Range", "bytes=123-456"} } );
    EXPECT_EQ(206, p->status);
    EXPECT_EQ("bytes 123-456/20077", p->get_header_value("Content-Range"));
    EXPECT_EQ(334U, p->body.size());
    EXPECT_EQ(full->body.substr(123, 334), p->body);
    EXPECT_EQ("bytes", p->get_header_value("Accept-Ranges"));
  }

  {
    const auto p = zfs1_->GET(url, { {"Range", "bytes=20000-"} } );
    EXPECT_EQ(206, p->status);
    EXPECT_EQ(full->body.substr(20000), p->body);
    EXPECT_EQ("bytes 20000-20076/20077", p->get_header_value("Content-Range"));
    EXPECT_EQ("bytes", p->get_header_value("Accept-Ranges"));
  }

  {
    const auto p = zfs1_->GET(url, { {"Range", "bytes=-100"} } );
    EXPECT_EQ(206, p->status);
    EXPECT_EQ(full->body.substr(19977), p->body);
    EXPECT_EQ("bytes 19977-20076/20077", p->get_header_value("Content-Range"));
    EXPECT_EQ("bytes", p->get_header_value("Accept-Ranges"));
  }
}

TEST_F(ServerTest, InvalidAndMultiRangeByteRangeRequestsResultIn416Responses)
{
  const char url[] = "/ROOT/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";

  const char* invalidRanges[] = {
    "0-10", "bytes=", "bytes=123", "bytes=-10-20", "bytes=10-20xxx",
    "bytes=10-0", // reversed range
    "bytes=10-20, 30-40", // multi-range
    "bytes=1000000-", "bytes=30000-30100" // unsatisfiable ranges
  };

  for( const char* range : invalidRanges )
  {
    const TestContext ctx{ {"Range", range} };
    const auto p = zfs1_->GET(url, { {"Range", range } } );
    EXPECT_EQ(416, p->status) << ctx;
    EXPECT_TRUE(p->body.empty()) << ctx;
    EXPECT_EQ("bytes */20077", p->get_header_value("Content-Range")) << ctx;
  }
}

TEST_F(ServerTest, ValidByteRangeRequestsOfZeroSizedEntriesResultIn416Responses)
{
  const char url[] = "/ROOT/corner_cases/-/empty.js";

  const char* ranges[] = {
    "bytes=0-",
    "bytes=-100"
  };

  for( const char* range : ranges )
  {
    const TestContext ctx{ {"Range", range} };
    const auto p = zfs1_->GET(url, { {"Range", range } } );
    EXPECT_EQ(416, p->status) << ctx;
    EXPECT_TRUE(p->body.empty()) << ctx;
    EXPECT_EQ("bytes */0", p->get_header_value("Content-Range")) << ctx;
  }
}

TEST_F(ServerTest, RangeHasPrecedenceOverCompression)
{
  const char url[] = "/ROOT/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";

  const Headers onlyRange{ {"Range", "bytes=123-456"} };
  Headers rangeAndCompression(onlyRange);
  rangeAndCompression.insert({"Accept-Encoding", "deflate"});

  const auto p1 = zfs1_->GET(url, onlyRange);
  const auto p2 = zfs1_->GET(url, rangeAndCompression);
  EXPECT_EQ(p1->status, p2->status);
  EXPECT_EQ(invariantHeaders(p1->headers), invariantHeaders(p2->headers));
  EXPECT_EQ(p1->body, p2->body);
}

TEST_F(ServerTest, RangeHeaderIsCaseInsensitive)
{
  const char url[] = "/ROOT/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";
  const auto r0 = zfs1_->GET(url, { {"Range", "bytes=100-200"} } );

  const char* header_variations[] = { "RANGE", "range", "rAnGe", "RaNgE" };
  for ( const char* header : header_variations ) {
    const auto r = zfs1_->GET(url, { {header, "bytes=100-200"} } );
    EXPECT_EQ(206, r->status);
    EXPECT_EQ("bytes 100-200/20077", r->get_header_value("Content-Range"));
    EXPECT_EQ(r0->body, r->body);
  }
}

TEST_F(ServerTest, suggestions)
{
  typedef std::pair<std::string, std::string> UrlAndExpectedResponse;
  const std::vector<UrlAndExpectedResponse> testData{
    { /* url: */ "/ROOT/suggest?content=zimfile&term=thing",
R"EXPECTEDRESPONSE([
  {
    "value" : "Doing His Thing",
    "label" : "Doing His &lt;b&gt;Thing&lt;/b&gt;",
    "kind" : "path"
      , "path" : "A/Doing_His_Thing"
  },
  {
    "value" : "We Didn&apos;t See a Thing",
    "label" : "We Didn&apos;t See a &lt;b&gt;Thing&lt;/b&gt;",
    "kind" : "path"
      , "path" : "A/We_Didn&apos;t_See_a_Thing"
  },
  {
    "value" : "thing ",
    "label" : "containing &apos;thing&apos;...",
    "kind" : "pattern"
    //EOLWHITESPACEMARKER
  }
]
)EXPECTEDRESPONSE"
    },
    { /* url: */ "/ROOT/suggest?content=zimfile&term=old%20sun",
R"EXPECTEDRESPONSE([
  {
    "value" : "That Lucky Old Sun",
    "label" : "That Lucky &lt;b&gt;Old&lt;/b&gt; &lt;b&gt;Sun&lt;/b&gt;",
    "kind" : "path"
      , "path" : "A/That_Lucky_Old_Sun"
  },
  {
    "value" : "old sun ",
    "label" : "containing &apos;old sun&apos;...",
    "kind" : "pattern"
    //EOLWHITESPACEMARKER
  }
]
)EXPECTEDRESPONSE"
    },
    { /* url: */ "/ROOT/suggest?content=zimfile&term=abracadabra",
R"EXPECTEDRESPONSE([
  {
    "value" : "abracadabra ",
    "label" : "containing &apos;abracadabra&apos;...",
    "kind" : "pattern"
    //EOLWHITESPACEMARKER
  }
]
)EXPECTEDRESPONSE"
    },
    { // Test handling of & (%26 when url-encoded) in the search string
      /* url: */ "/ROOT/suggest?content=zimfile&term=A%26B",
R"EXPECTEDRESPONSE([
  {
    "value" : "A&amp;B ",
    "label" : "containing &apos;A&amp;B&apos;...",
    "kind" : "pattern"
    //EOLWHITESPACEMARKER
  }
]
)EXPECTEDRESPONSE"
    },
  };

  for ( const auto& urlAndExpectedResponse : testData ) {
    const std::string url = urlAndExpectedResponse.first;
    const std::string expectedResponse = urlAndExpectedResponse.second;
    const TestContext ctx{ {"url", url} };
    const auto r = zfs1_->GET(url.c_str());
    EXPECT_EQ(r->status, 200) << ctx;
    EXPECT_EQ(r->body, removeEOLWhitespaceMarkers(expectedResponse)) << ctx;
  }
}

TEST_F(ServerTest, suggestions_in_range)
{
  /**
   * Attempt to get 50 suggestions in steps of 5
   * The suggestions are returned in the json format
   * [{sugg1}, {sugg2}, ... , {suggN}, {suggest ft search}]
   * Assuming the number of suggestions = (occurance of "{" - 1)
   */
  {
    int suggCount = 0;
    for (int i = 0; i < 10; i++) {
      std::string url = "/ROOT/suggest?content=zimfile&term=ray&start=" + std::to_string(i*5) + "&count=5";
      const auto r = zfs1_->GET(url.c_str());
      std::string body = r->body;
      int currCount = std::count(body.begin(), body.end(), '{') - 1;
      ASSERT_EQ(currCount, 5);
      suggCount += currCount;
    }
    ASSERT_EQ(suggCount, 50);
  }

  // Attempt to get 10 suggestions in steps of 5 even though there are only 8
  {
    std::string url = "/ROOT/suggest?content=zimfile&term=song+for+you&start=0&count=5";
    const auto r1 = zfs1_->GET(url.c_str());
    std::string body = r1->body;
    int currCount = std::count(body.begin(), body.end(), '{') - 1;
    ASSERT_EQ(currCount, 5);

    url = "/ROOT/suggest?content=zimfile&term=song+for+you&start=5&count=5";
    const auto r2 = zfs1_->GET(url.c_str());
    body = r2->body;
    currCount = std::count(body.begin(), body.end(), '{') - 1;
    ASSERT_EQ(currCount, 3);
  }

  // Attempt to get 10 suggestions even though there is only 1
  {
    std::string url = "/ROOT/suggest?content=zimfile&term=strong&start=0&count=5";
    const auto r = zfs1_->GET(url.c_str());
    std::string body = r->body;
    int currCount = std::count(body.begin(), body.end(), '{') - 1;
    ASSERT_EQ(currCount, 1);
  }

  // No Suggestion
  {
    std::string url = "/ROOT/suggest?content=zimfile&term=oops&start=0&count=5";
    const auto r = zfs1_->GET(url.c_str());
    std::string body = r->body;
    int currCount = std::count(body.begin(), body.end(), '{') - 1;
    ASSERT_EQ(currCount, 0);
  }

  // Out of bound value
  {
    std::string url = "/ROOT/suggest?content=zimfile&term=ray&start=-2&count=-1";
    const auto r = zfs1_->GET(url.c_str());
    std::string body = r->body;
    int currCount = std::count(body.begin(), body.end(), '{') - 1;
    ASSERT_EQ(currCount, 0);
  }
}

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
