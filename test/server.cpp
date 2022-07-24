
#define CPPHTTPLIB_ZLIB_SUPPORT 1
#include "./httplib.h"
#include "gtest/gtest.h"

#define SERVER_PORT 8001
#include "server_testing_tools.h"


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

  { NO_ETAG,   "/ROOT/catalog/search" },

  { NO_ETAG,   "/ROOT/search?content=zimfile&pattern=a" },

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

  // The following url's responses are too small to be compressed
  { NO_ETAG,   "/ROOT/catalog/root.xml" },
  { NO_ETAG,   "/ROOT/catalog/searchdescription.xml" },
  { NO_ETAG,   "/ROOT/suggest?content=zimfile" },
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

  ZimFileServer zfs(PORT, /*withTaskbar=*/true, ZIMFILES, "");
  EXPECT_EQ(200, zfs.GET("/ROOT/")->status);
}

TEST(indexTemplateStringTest, indexTemplateCheck) {
  const int PORT = 8001;
  const ZimFileServer::FilePathCollection ZIMFILES {
    "./test/zimfile.zim",
    "./test/corner_cases.zim"
  };

  ZimFileServer zfs(PORT, /*withTaskbar=*/true, ZIMFILES, "<!DOCTYPE html><head>"
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
    const auto x = zfs1_->GET(res.url, { {"Accept-Encoding", "gzip"} });
    EXPECT_EQ(200, x->status) << res;
    EXPECT_EQ("gzip", x->get_header_value("Content-Encoding")) << res;
    EXPECT_EQ("Accept-Encoding", x->get_header_value("Vary")) << res;
  }
}

TEST_F(ServerTest, UncompressibleContentIsNotCompressed)
{
  for ( const Resource& res : resources200Uncompressible ) {
    const auto x = zfs1_->GET(res.url, { {"Accept-Encoding", "gzip"} });
    EXPECT_EQ(200, x->status) << res;
    EXPECT_EQ("", x->get_header_value("Content-Encoding")) << res;
  }
}


// Selects from text only the lines containing the specified (fixed string)
// pattern
std::string fgrep(const std::string& pattern, const std::string& text)
{
  std::istringstream iss(text);
  std::string line;
  std::string result;
  while ( getline(iss, line) ) {
    if ( line.find(pattern) != std::string::npos ) {
      result += line + "\n";
    }
  }
  return result;
}

TEST_F(ServerTest, CacheIdsOfStaticResources)
{
  typedef std::pair<std::string, std::string> UrlAndExpectedResult;
  const std::vector<UrlAndExpectedResult> testData{
    {
      /* url */ "/ROOT/",
R"EXPECTEDRESULT(      href="/ROOT/skin/index.css?cacheid=56e818cd"
        src: url("/ROOT/skin/fonts/Poppins.ttf?cacheid=af705837") format("truetype");
          src: url("/ROOT/skin/fonts/Roboto.ttf?cacheid=84d10248") format("truetype");
    <script src="/ROOT/skin/isotope.pkgd.min.js?cacheid=2e48d392" defer></script>
    <script src="/ROOT/skin/iso6391To3.js?cacheid=ecde2bb3"></script>
    <script type="text/javascript" src="/ROOT/skin/index.js?cacheid=76440e7a" defer></script>
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT/skin/index.js",
R"EXPECTEDRESULT(                                <img src="../skin/download.png?cacheid=a39aa502" alt="direct download" />
                                <img src="../skin/hash.png?cacheid=f836e872" alt="download hash" />
                                <img src="../skin/magnet.png?cacheid=73b6bddf" alt="download magnet" />
                                <img src="../skin/bittorrent.png?cacheid=4f5c6882" alt="download torrent" />
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT/zimfile/A/index",
R"EXPECTEDRESULT(<link type="root" href="/ROOT"><link type="text/css" href="/ROOT/skin/jquery-ui/jquery-ui.min.css?cacheid=e1de77b3" rel="Stylesheet" />
<link type="text/css" href="/ROOT/skin/jquery-ui/jquery-ui.theme.min.css?cacheid=2a5841f9" rel="Stylesheet" />
<link type="text/css" href="/ROOT/skin/taskbar.css?cacheid=26082885" rel="Stylesheet" />
<link type="text/css" href="/ROOT/skin/css/autoComplete.css?cacheid=08951e06" rel="Stylesheet" />
<script type="text/javascript" src="/ROOT/skin/jquery-ui/external/jquery/jquery.js?cacheid=1d85f0f3" defer></script>
<script type="text/javascript" src="/ROOT/skin/jquery-ui/jquery-ui.min.js?cacheid=d927c2ff" defer></script>
<script type="text/javascript" src="/ROOT/skin/taskbar.js?cacheid=1aec4a68" defer></script>
<script type="text/javascript" src="/ROOT/skin/autoComplete.min.js?cacheid=1191aaaf"></script>
        <label for="kiwix_button_show_toggle"><img src="/ROOT/skin/caret.png?cacheid=22b942b4" alt=""></label>
)EXPECTEDRESULT"
    },
    {
      // Searching in a ZIM file without a full-text index returns
      // a page rendered from static/templates/no_search_result_html
      /* url */ "/ROOT/search?content=poor&pattern=whatever",
R"EXPECTEDRESULT(    <link type="text/css" href="/ROOT/skin/search_results.css?cacheid=76d39c84" rel="Stylesheet" />
  <link type="root" href="/ROOT"><link type="text/css" href="/ROOT/skin/jquery-ui/jquery-ui.min.css?cacheid=e1de77b3" rel="Stylesheet" />
<link type="text/css" href="/ROOT/skin/jquery-ui/jquery-ui.theme.min.css?cacheid=2a5841f9" rel="Stylesheet" />
<link type="text/css" href="/ROOT/skin/taskbar.css?cacheid=26082885" rel="Stylesheet" />
<link type="text/css" href="/ROOT/skin/css/autoComplete.css?cacheid=08951e06" rel="Stylesheet" />
<script type="text/javascript" src="/ROOT/skin/jquery-ui/external/jquery/jquery.js?cacheid=1d85f0f3" defer></script>
<script type="text/javascript" src="/ROOT/skin/jquery-ui/jquery-ui.min.js?cacheid=d927c2ff" defer></script>
<script type="text/javascript" src="/ROOT/skin/taskbar.js?cacheid=1aec4a68" defer></script>
<script type="text/javascript" src="/ROOT/skin/autoComplete.min.js?cacheid=1191aaaf"></script>
        <label for="kiwix_button_show_toggle"><img src="/ROOT/skin/caret.png?cacheid=22b942b4" alt=""></label>
)EXPECTEDRESULT"
    },
  };

  for ( const auto& urlAndExpectedResult : testData ) {
    const std::string url = urlAndExpectedResult.first;
    const std::string expectedResult = urlAndExpectedResult.second;
    const TestContext ctx{ {"url", url} };
    const auto r = zfs1_->GET(url.c_str());
    EXPECT_EQ(r->body.find("KIWIXCACHEID"), std::string::npos) << ctx;
    EXPECT_EQ(fgrep("/skin/", r->body), expectedResult) << ctx;
  }
}

const char* urls400[] = {
  "/ROOT/search",
  "/ROOT/search?content=zimfile",
  "/ROOT/search?content=non-existing-book&pattern=asdfqwerty",
  "/ROOT/search?content=non-existing-book&pattern=asd<qwerty",
  "/ROOT/search?books.name=non-exsitent-book&pattern=asd<qwerty",
  "/ROOT/search?books.id=non-exsitent-id&pattern=asd<qwerty",
  "/ROOT/search?books.filter.lang=unk&pattern=asd<qwerty",
  "/ROOT/search?pattern=foo",
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

struct CustomizedServerTest : ServerTest
{
  void SetUp()
  {
    setenv("KIWIX_SERVE_CUSTOMIZED_RESOURCES", "./test/customized_resources.txt", 1);
    ServerTest::SetUp();
  }
};

typedef std::vector<std::string> StringCollection;

std::string getHeaderValue(const Headers& headers, const std::string& name)
{
  const auto er = headers.equal_range(name);
  const auto n = std::distance(er.first, er.second);
  if (n == 0)
    throw std::runtime_error("Missing header: " + name);
  if (n > 1)
    throw std::runtime_error("Multiple occurrences of header: " + name);
  return er.first->second;
}

TEST_F(CustomizedServerTest, NewResourcesCanBeAdded)
{
  // ServerTest.404 verifies that "/ROOT/non-existent-item" doesn't exist
  const auto r = zfs1_->GET("/ROOT/non-existent-item");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), "text/plain");
  EXPECT_EQ(r->body, "Hello world!\n");
}

TEST_F(CustomizedServerTest, ContentOfAnyServableUrlCanBeOverriden)
{
  {
    const auto r = zfs1_->GET("/ROOT/");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), "text/html");
    EXPECT_EQ(r->body, "<html><head></head><body>Welcome</body></html>\n");
  }

  {
    const auto r = zfs1_->GET("/ROOT/skin/index.css");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), "application/json");
    EXPECT_EQ(r->body, "Hello world!\n");
  }

  {
    const auto r = zfs1_->GET("/ROOT/zimfile/A/Ray_Charles");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), "ray/charles");
    EXPECT_EQ(r->body, "<html><head></head><body>Welcome</body></html>\n");
  }

  {
    const auto r = zfs1_->GET("/ROOT/search?pattern=la+femme");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), "text/html");
    EXPECT_EQ(r->body, "Hello world!\n");
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
// TEST_F(ServerTest, Http404HtmlError))
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
  bool isTranslatedVersion() const;
  virtual std::string pageTitle() const;
  std::string pageCssLink() const;
  std::string hiddenBookNameInput() const;
  std::string searchPatternInput() const;
  std::string taskbarLinks() const;
  std::string goToWelcomePageText() const;
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

    R"FRAG(
  <link type="root" href="/ROOT"><link type="text/css" href="/ROOT/skin/jquery-ui/jquery-ui.min.css?cacheid=e1de77b3" rel="Stylesheet" />
<link type="text/css" href="/ROOT/skin/jquery-ui/jquery-ui.theme.min.css?cacheid=2a5841f9" rel="Stylesheet" />
<link type="text/css" href="/ROOT/skin/taskbar.css?cacheid=26082885" rel="Stylesheet" />
<link type="text/css" href="/ROOT/skin/css/autoComplete.css?cacheid=08951e06" rel="Stylesheet" />
<script type="text/javascript" src="/ROOT/skin/jquery-ui/external/jquery/jquery.js?cacheid=1d85f0f3" defer></script>
<script type="text/javascript" src="/ROOT/skin/jquery-ui/jquery-ui.min.js?cacheid=d927c2ff" defer></script>
<script type="text/javascript" src="/ROOT/skin/taskbar.js?cacheid=1aec4a68" defer></script>
<script type="text/javascript" src="/ROOT/skin/autoComplete.min.js?cacheid=1191aaaf"></script>
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
        <label for="kiwix_button_show_toggle"><img src="/ROOT/skin/caret.png?cacheid=22b942b4" alt=""></label>
        <div class="kiwix_button_cont">
            <a id="kiwix_serve_taskbar_library_button" title=")FRAG",

  R"FRAG(" aria-label=")FRAG",

  R"FRAG(" href="/ROOT/"><button>&#x1f3e0;</button></a>
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
       + goToWelcomePageText()
       + frag[5]
       + goToWelcomePageText()
       + frag[6]
       + taskbarLinks()
       + frag[7]
       + expectedBody
       + frag[8];
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
       + R"(" rel="Stylesheet" />)";
}

std::string TestContentIn404HtmlResponse::hiddenBookNameInput() const
{
  return bookName.empty()
       ? ""
       : R"(<input type="hidden" name="content" value=")" + bookName + R"(" />)";
}

std::string TestContentIn404HtmlResponse::searchPatternInput() const
{
  const std::string searchboxTooltip = isTranslatedVersion()
                                    ? "Որոնել '" + bookTitle + "'֊ում"
                                    : "Search '" + bookTitle + "'";
  return R"(          <input autocomplete="off" id="kiwixsearchbox" name="pattern" type="text" size="50" title=")"
       + searchboxTooltip
       + R"(" aria-label=")"
       + searchboxTooltip
       + R"(">
)";
}

std::string TestContentIn404HtmlResponse::taskbarLinks() const
{
  if ( bookName.empty() )
    return "";

  const auto goToMainPageOfBook = isTranslatedVersion()
                                ? "Դեպի '" + bookTitle + "'֊ի գլխավոր էջը"
                                : "Go to the main page of '" + bookTitle + "'";

  const std::string goToRandomPage = isTranslatedVersion()
                                   ? "Բացել պատահական էջ"
                                   : "Go to a randomly selected page";

  return R"(<a id="kiwix_serve_taskbar_home_button" title=")"
       + goToMainPageOfBook
       + R"(" aria-label=")"
       + goToMainPageOfBook
       + R"(" href="/ROOT/)"
       + bookName
       + R"(/"><button>)"
       + bookTitle
       + R"(</button></a>
          <a id="kiwix_serve_taskbar_random_button" title=")"
       + goToRandomPage
       + R"(" aria-label=")"
       + goToRandomPage
       + R"("
            href="/ROOT/random?content=)"
       + bookName
       + R"("><button>&#x1F3B2;</button></a>)";
}

bool TestContentIn404HtmlResponse::isTranslatedVersion() const
{
  return url.find("userlang=hy") != std::string::npos;
}

std::string TestContentIn404HtmlResponse::goToWelcomePageText() const
{
  return isTranslatedVersion()
       ? "Գրադարանի էջ"
       : "Go to welcome page";
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

TEST_F(ServerTest, Http404HtmlError)
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

    { /* url */ "/ROOT/random?content=non-existent-book&userlang=hy",
      expected_page_title=="Սխալ հասցե" &&
      expected_body==R"(
    <h1>Սխալ հասցե</h1>
    <p>
      Գիրքը բացակայում է՝ non-existent-book
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

    { /* url */ "/ROOT/catalog/?userlang=hy",
      expected_page_title=="Սխալ հասցե" &&
      expected_body==R"(
    <h1>Սխալ հասցե</h1>
    <p>
      Սխալ հասցե՝ /ROOT/catalog/
    </p>
)"  },

    { /* url */ "/ROOT/catalog/invalid_endpoint",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT/catalog/invalid_endpoint" was not found on this server.
    </p>
)"  },

    { /* url */ "/ROOT/catalog/invalid_endpoint?userlang=hy",
      expected_page_title=="Սխալ հասցե" &&
      expected_body==R"(
    <h1>Սխալ հասցե</h1>
    <p>
      Սխալ հասցե՝ /ROOT/catalog/invalid_endpoint
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

    { /* url */ "/ROOT/zimfile/invalid-article?userlang=hy",
      expected_page_title=="Սխալ հասցե" &&
      book_name=="zimfile" &&
      book_title=="Ray Charles" &&
      expected_body==R"(
    <h1>Սխալ հասցե</h1>
    <p>
      Սխալ հասցե՝ /ROOT/zimfile/invalid-article
    </p>
    <p>
      Որոնել <a href="/ROOT/search?content=zimfile&pattern=invalid-article">invalid-article</a>
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

    { /* url */ "/ROOT/search?content=poor&pattern=whatever",
      expected_page_title=="Fulltext search unavailable" &&
      expected_css_url=="/ROOT/skin/search_results.css?cacheid=76d39c84" &&
      book_name=="poor" &&
      book_title=="poor" &&
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The fulltext search engine is not available for this content.
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

TEST_F(ServerTest, Http400HtmlError)
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
      Too many books requested (4) where limit is 3
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
      No such book: non-existing-book
    </p>
)"  },
    { /* url */ "/ROOT/search?content=non-existing-book&pattern=a\"<script foo>",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT/search?content=non-existing-book&pattern=a"&lt;script foo&gt;" is not a valid request.
    </p>
    <p>
      No such book: non-existing-book
    </p>
)"  },
    // There is a flaw in our way to handle query string, we cannot differenciate
    // between `pattern` and `pattern=`
    { /* url */ "/ROOT/search?books.filter.lang=eng&pattern",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT/search?books.filter.lang=eng&pattern=" is not a valid request.
    </p>
    <p>
      No query provided.
    </p>
)"  },
    { /* url */ "/ROOT/search?pattern=foo",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT/search?pattern=foo" is not a valid request.
    </p>
    <p>
      Too many books requested (4) where limit is 3
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

TEST_F(ServerTest, HttpXmlError)
{
  struct TestData
  {
    std::string url;
    int expectedStatusCode;
    std::string expectedXml;

    std::string fullExpectedXml() const
    {
      return R"(<?xml version="1.0" encoding="UTF-8">)" + expectedXml;
    }

    TestContext ctx() const
    {
      return TestContext{ {"url", url} };
    }
  };

  const std::vector<TestData> testData{
    { /* url */ "/ROOT/search?format=xml",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT/search?format=xml" is not a valid request.</detail>
<detail>Too many books requested (4) where limit is 3</detail>
)"  },
    { /* url */ "/ROOT/search?format=xml&content=zimfile",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT/search?content=zimfile&format=xml" is not a valid request.</detail>
<detail>No query provided.</detail>
)"  },
    { /* url */ "/ROOT/search?format=xml&content=non-existing-book&pattern=asdfqwerty",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT/search?content=non-existing-book&format=xml&pattern=asdfqwerty" is not a valid request.</detail>
<detail>No such book: non-existing-book</detail>
)"  },
    { /* url */ "/ROOT/search?format=xml&content=non-existing-book&pattern=a\"<script foo>",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT/search?content=non-existing-book&format=xml&pattern=a"&lt;script foo&gt;" is not a valid request.</detail>
<detail>No such book: non-existing-book</detail>
)"  },
    // There is a flaw in our way to handle query string, we cannot differenciate
    // between `pattern` and `pattern=`
    { /* url */ "/ROOT/search?format=xml&books.filter.lang=eng&pattern",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT/search?books.filter.lang=eng&format=xml&pattern=" is not a valid request.</detail>
<detail>No query provided.</detail>
)"  },
    { /* url */ "/ROOT/search?format=xml&pattern=foo",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT/search?format=xml&pattern=foo" is not a valid request.</detail>
<detail>Too many books requested (4) where limit is 3</detail>
)"  },
    { /* url */ "/ROOT/search?format=xml&content=poor&pattern=whatever",
      /* HTTP status code */ 404,
      /* expected response XML */ R"(
<error>Fulltext search unavailable</error>
<detail>The fulltext search engine is not available for this content.</detail>
)"  },
  };

  for ( const auto& t : testData ) {
    const auto r = zfs1_->GET(t.url.c_str());
    EXPECT_EQ(r->status, t.expectedStatusCode) << t.ctx();
    EXPECT_EQ(r->body, t.fullExpectedXml()) << t.ctx();
  }
}

TEST_F(ServerTest, 500)
{
  const std::string expectedBody = R"(<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta content="text/html;charset=UTF-8" http-equiv="content-type" />
    <title>Internal Server Error</title>

  </head>
  <body>
    <h1>Internal Server Error</h1>
    <p>
      An internal server error occured. We are sorry about that :/
    </p>
    <p>
      Entry redirect_loop.html is a redirect entry.
    </p>
  </body>
</html>
)";

  const auto r = zfs1_->GET("/ROOT/poor/A/redirect_loop.html");
  EXPECT_EQ(r->status, 500);
  EXPECT_EQ(r->body, expectedBody);
}

TEST_F(ServerTest, UserLanguageControl)
{
  struct TestData
  {
    const std::string url;
    const std::string acceptLanguageHeader;
    const std::string expectedH1;

    operator TestContext() const
    {
      return TestContext{
          {"url", url},
          {"acceptLanguageHeader", acceptLanguageHeader},
      };
    }
  };

  const TestData testData[] = {
    {
      /*url*/ "/ROOT/zimfile/invalid-article",
      /*Accept-Language:*/ "",
      /* expected <h1> */ "Not Found"
    },
    {
      /*url*/ "/ROOT/zimfile/invalid-article?userlang=en",
      /*Accept-Language:*/ "",
      /* expected <h1> */ "Not Found"
    },
    {
      /*url*/ "/ROOT/zimfile/invalid-article?userlang=hy",
      /*Accept-Language:*/ "",
      /* expected <h1> */ "Սխալ հասցե"
    },
    {
      /*url*/ "/ROOT/zimfile/invalid-article",
      /*Accept-Language:*/ "*",
      /* expected <h1> */ "Not Found"
    },
    {
      /*url*/ "/ROOT/zimfile/invalid-article",
      /*Accept-Language:*/ "hy",
      /* expected <h1> */ "Սխալ հասցե"
    },
    {
      // userlang query parameter takes precedence over Accept-Language
      /*url*/ "/ROOT/zimfile/invalid-article?userlang=en",
      /*Accept-Language:*/ "hy",
      /* expected <h1> */ "Not Found"
    },
    {
      // The value of the Accept-Language header is not currently parsed.
      // In case of a comma separated list of languages (optionally weighted
      // with quality values) the default (en) language is used instead.
      /*url*/ "/ROOT/zimfile/invalid-article",
      /*Accept-Language:*/ "hy;q=0.9, en;q=0.2",
      /* expected <h1> */ "Not Found"
    },
  };

  const std::regex h1Regex("<h1>(.+)</h1>");
  for ( const auto& t : testData ) {
    std::smatch h1Match;
    Headers headers;
    if ( !t.acceptLanguageHeader.empty() ) {
      headers.insert({"Accept-Language", t.acceptLanguageHeader});
    }
    const auto r = zfs1_->GET(t.url.c_str(), headers);
    std::regex_search(r->body, h1Match, h1Regex);
    const std::string h1(h1Match[1]);
    EXPECT_EQ(h1, t.expectedH1) << t;
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
  ZimFileServer zfs2(SERVER_PORT + 1, /*withTaskbar=*/true, ZIMFILES);
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
    const auto g3 = zfs1_->GET(res.url, { {"Accept-Encoding", "gzip"} } );
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
    const auto g3 = zfs1_->GET(res.url, { {"Accept-Encoding", "gzip"} } );
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
  const char* const encodings[] = { "", "gzip" };
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
  rangeAndCompression.insert({"Accept-Encoding", "gzip"});

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
    { /* url: */ "/ROOT/suggest?content=zimfile&term=öld%20suñ",
R"EXPECTEDRESPONSE([
  {
    "value" : "That Lucky Old Sun",
    "label" : "That Lucky &lt;b&gt;Old&lt;/b&gt; &lt;b&gt;Sun&lt;/b&gt;",
    "kind" : "path"
      , "path" : "A/That_Lucky_Old_Sun"
  },
  {
    "value" : "öld suñ ",
    "label" : "containing &apos;öld suñ&apos;...",
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
    { /* url: */ "/ROOT/suggest?content=zimfile&term=abracadabra&userlang=hy",
R"EXPECTEDRESPONSE([
  {
    "value" : "abracadabra ",
    "label" : "որոնել &apos;abracadabra&apos;...",
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
