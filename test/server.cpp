
#define CPPHTTPLIB_ZLIB_SUPPORT 1
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

std::string replace(std::string s, std::string pattern, std::string replacement)
{
  return std::regex_replace(s, std::regex(pattern), replacement);
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
  ZimFileServer(int serverPort,
                bool withTaskbar,
                const FilePathCollection& zimpaths,
                std::string indexTemplateString = "");
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
  const bool withTaskbar = true;
};

ZimFileServer::ZimFileServer(int serverPort, std::string libraryFilePath)
: manager(&this->library)
{
  if ( kiwix::isRelativePath(libraryFilePath) )
    libraryFilePath = kiwix::computeAbsolutePath(kiwix::getCurrentDirectory(), libraryFilePath);
  manager.readFile(libraryFilePath, true, true);
  run(serverPort);
}

ZimFileServer::ZimFileServer(int serverPort,
                             bool _withTaskbar,
                             const FilePathCollection& zimpaths,
                             std::string indexTemplateString)
: manager(&this->library)
, withTaskbar(_withTaskbar)
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
  server->setTaskbar(withTaskbar, withTaskbar);
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
    "./test/poor.zim",
    "./test/corner_cases.zim"
  };

protected:
  void SetUp() override {
    zfs1_.reset(new ZimFileServer(PORT, /*withTaskbar=*/true, ZIMFILES));
  }

  void TearDown() override {
    zfs1_.reset();
  }
};

class TaskbarlessServerTest : public ServerTest
{
protected:
  void SetUp() override {
    zfs1_.reset(new ZimFileServer(PORT, /*withTaskbar=*/false, ZIMFILES));
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
R"EXPECTEDRESULT(      src="/ROOT/skin/jquery-ui/external/jquery/jquery.js?cacheid=1d85f0f3"
      src="/ROOT/skin/jquery-ui/jquery-ui.min.js?cacheid=d927c2ff"
      href="/ROOT/skin/jquery-ui/jquery-ui.min.css?cacheid=e1de77b3"
      href="/ROOT/skin/jquery-ui/jquery-ui.theme.min.css?cacheid=2a5841f9"
      href="/ROOT/skin/index.css?cacheid=1aca980a"
        src: url("/ROOT/skin/fonts/Poppins.ttf?cacheid=af705837") format("truetype");
          src: url("/ROOT/skin/fonts/Roboto.ttf?cacheid=84d10248") format("truetype");
    <script src="/ROOT/skin/isotope.pkgd.min.js?cacheid=2e48d392" defer></script>
    <script src="/ROOT/skin/iso6391To3.js?cacheid=ecde2bb3"></script>
    <script type="text/javascript" src="/ROOT/skin/index.js?cacheid=31ffa1f7" defer></script>
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
<link type="text/css" href="/ROOT/skin/taskbar.css?cacheid=49365e9c" rel="Stylesheet" />
<script type="text/javascript" src="/ROOT/skin/jquery-ui/external/jquery/jquery.js?cacheid=1d85f0f3" defer></script>
<script type="text/javascript" src="/ROOT/skin/jquery-ui/jquery-ui.min.js?cacheid=d927c2ff" defer></script>
<script type="text/javascript" src="/ROOT/skin/taskbar.js?cacheid=5982280c" defer></script>
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
<link type="text/css" href="/ROOT/skin/taskbar.css?cacheid=49365e9c" rel="Stylesheet" />
<script type="text/javascript" src="/ROOT/skin/jquery-ui/external/jquery/jquery.js?cacheid=1d85f0f3" defer></script>
<script type="text/javascript" src="/ROOT/skin/jquery-ui/jquery-ui.min.js?cacheid=d927c2ff" defer></script>
<script type="text/javascript" src="/ROOT/skin/taskbar.js?cacheid=5982280c" defer></script>
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
<link type="text/css" href="/ROOT/skin/taskbar.css?cacheid=49365e9c" rel="Stylesheet" />
<script type="text/javascript" src="/ROOT/skin/jquery-ui/external/jquery/jquery.js?cacheid=1d85f0f3" defer></script>
<script type="text/javascript" src="/ROOT/skin/jquery-ui/jquery-ui.min.js?cacheid=d927c2ff" defer></script>
<script type="text/javascript" src="/ROOT/skin/taskbar.js?cacheid=5982280c" defer></script>
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

  return R"(          <input autocomplete="off" class="ui-autocomplete-input" id="kiwixsearchbox" name="pattern" type="text" title=")"
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

std::string makeSearchResultsHtml(const std::string& pattern,
                                  const std::string& header,
                                  const std::string& results,
                                  const std::string& footer)
{
  const char SEARCHRESULTS_HTML_TEMPLATE[] = R"HTML(<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta content="text/html; charset=utf-8" http-equiv="content-type" />

    //EOLWHITESPACEMARKER
    <style type="text/css">
      body{
      color: #000000;
      font: small/normal Arial,Helvetica,Sans-Serif;
      margin-top: 0.5em;
      font-size: 90%;
      }

      a{
      color: #04c;
      }

      a:visited {
      color: #639
      }

      a:hover {
      text-decoration: underline
      }

      .header {
      font-size: 120%;
      }

      ul {
      margin:0;
      padding:0
      }

      .results {
      font-size: 110%;
      }

      .results li {
      list-style-type:none;
      margin-top: 0.5em;
      }

      .results a {
      font-size: 110%;
      text-decoration: underline
      }

      cite {
      font-style:normal;
      word-wrap:break-word;
      display: block;
      font-size: 100%;
      }

      .informations {
      color: #388222;
      font-size: 100%;
      }

      .book-title {
      color: #662200;
      font-size: 100%;
      }

      .footer {
      padding: 0;
      margin-top: 1em;
      width: 100%;
      float: left
      }

      .footer a, .footer span {
      display: block;
      padding: .3em .7em;
      margin: 0 .38em 0 0;
      text-align:center;
      text-decoration: none;
      }

      .footer a:hover {
      background: #ededed;
      }

      .footer ul, .footer li {
      list-style:none;
      margin: 0;
      padding: 0;
      }

      .footer li {
      float: left;
      }

      .selected {
      background: #ededed;
      }

    </style>
    <title>Search: %PATTERN%</title>
  <link type="root" href="/ROOT"></head>
  <body bgcolor="white">
    <div class="header">
      %HEADER%
    </div>

    <div class="results">
      <ul>%RESULTS%
      </ul>
    </div>

    <div class="footer">%FOOTER%
    </div>
  </body>
</html>
)HTML";

  std::string html = removeEOLWhitespaceMarkers(SEARCHRESULTS_HTML_TEMPLATE);
  html = replace(html, "%PATTERN%", pattern);
  html = replace(html, "%HEADER%", header);
  html = replace(html, "%RESULTS%", results);
  html = replace(html, "%FOOTER%", footer);
  return html;
}

const std::vector<std::string> LARGE_SEARCH_RESULTS = {
R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Genius_+_Soul_=_Jazz">
              Genius + Soul = Jazz
            </a>
              <cite>...Grammy Hall of Fame in 2011. It was re-issued in the UK, first in 1989 on the Castle Communications "Essential Records" label, and by Rhino Records in 1997 on a single CD together with Charles' 1970 My Kind of <b>Jazz</b>. In 2010, Concord Records released a deluxe edition comprising digitally remastered versions of Genius + Soul = <b>Jazz</b>, My Kind of <b>Jazz</b>, <b>Jazz</b> Number II, and My Kind of <b>Jazz</b> Part 3. Professional ratings Review scores Source Rating Allmusic link Warr.org link Encyclopedia of Popular Music...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">242 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Jazz_Number_II">
              Jazz Number II
            </a>
              <cite><b>Jazz</b> Number II <b>Jazz</b> Number II is a 1973 album by Ray Charles. It is a collection of <b>jazz</b>/soul instrumentals featuring Charles on piano backed by his Big Band. Professional ratings Review scores Source Rating Allmusic link <b>Jazz</b> Number II Studio album by Ray Charles Released January 1973 Recorded 1971-72 Studio Charles’ Tangerine/RPM Studios, Los Angeles, CA Genre Soul, <b>jazz</b> Length 39:02 Label Tangerine Producer Ray Charles Ray Charles chronology Through the Eyes of Love (1972) <b>Jazz</b> Number II......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">87 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/My_Kind_of_Jazz_Part_3">
              My Kind of Jazz Part 3
            </a>
              <cite>My Kind of <b>Jazz</b> Part 3 My Kind of <b>Jazz</b> Part 3 is a 1975 album by Ray Charles released by Crossover Records. Concord Records re-issued the contents in digital form in 2009. Professional ratings Review scores Source Rating Allmusic link My Kind of <b>Jazz</b> Part 3 Studio album by Ray Charles Released October 1975 Recorded 1975 in Los Angeles, CA Genre Soul, <b>jazz</b> Length 38:13 Label Crossover Producer Ray Charles Ray Charles chronology Renaissance (1975) My Kind of <b>Jazz</b> Part 3 (1975) Live In Japan (1975)...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">88 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/My_Kind_of_Jazz">
              My Kind of Jazz
            </a>
              <cite>My Kind of <b>Jazz</b> My Kind of <b>Jazz</b> Studio album by Ray Charles Released April 1970 Recorded January 1-10, 1970 in Los Angeles, CA Genre <b>jazz</b> Length 30:20 Label Tangerine Producer Quincy Jones Ray Charles chronology Doing His Thing (1969) My Kind of <b>Jazz</b> (1970) Love Country Style (1970) Professional ratings Review scores Source Rating Allmusic link My Kind of <b>Jazz</b> is a 1970 album by Ray Charles....</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">69 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Hank_Crawford">
              Hank Crawford
            </a>
              <cite>...bop, <b>jazz</b>-funk, soul <b>jazz</b> alto saxophonist, arranger and songwriter. Crawford was musical director for Ray Charles before embarking on a solo career releasing many well-regarded albums on Atlantic, CTI and Milestone. Hank Crawford Background information Birth name Bennie Ross Crawford, Jr Born (1934-12-21)December 21, 1934 Memphis, Tennessee, U.S. Died January 29, 2009(2009-01-29) (aged 74) Memphis, Tennessee, U.S. Genres R&amp;B, Hard bop, <b>Jazz</b>-funk, Soul <b>jazz</b> Occupation(s) Saxophonist, Songwriter......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">102 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Catchin&apos;_Some_Rays:_The_Music_of_Ray_Charles">
              Catchin&apos; Some Rays: The Music of Ray Charles
            </a>
              <cite>...<b>jazz</b> singer Roseanna Vitro, released in August 1997 on the Telarc <b>Jazz</b> label. Catchin' Some Rays: The Music of Ray Charles Studio album by Roseanna Vitro Released August 1997 Recorded March 26, 1997 at Sound on Sound, NYC April 4,1997 at Quad Recording Studios, NYC Genre Vocal <b>jazz</b> Length 61:00 Label Telarc <b>Jazz</b> CD-83419 Producer Paul Wickliffe Roseanna Vitro chronology Passion Dance (1996) Catchin' Some Rays: The Music of Ray Charles (1997) The Time of My Life: Roseanna Vitro Sings the Songs of......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">118 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/That&apos;s_What_I_Say:_John_Scofield_Plays_the_Music_of_Ray_Charles">
              That&apos;s What I Say: John Scofield Plays the Music of Ray Charles
            </a>
              <cite>That's What I Say: John Scofield Plays the Music of Ray Charles Studio album by John Scofield Released June 7, 2005 (2005-06-07) Recorded December 2004 Studio Avatar Studios, New York City Genre <b>Jazz</b> Length 65:21 Label Verve Producer Steve Jordan John Scofield chronology EnRoute: John Scofield Trio LIVE (2004) That's What I Say: John Scofield Plays the Music of Ray Charles (2005) Out Louder (2006) Professional ratings Review scores Source Rating Allmusic All About <b>Jazz</b> All About <b>Jazz</b>...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">109 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Tribute_to_Uncle_Ray">
              Tribute to Uncle Ray
            </a>
              <cite>...singles as Motown struggled to find a sound to fit Wonder, who was just 12 when this album was released. Tribute to Uncle Ray Studio album by Little Stevie Wonder Released October 1962 Recorded 1962 Studio Studio A, Hitsville USA, Detroit Genre Soul, <b>jazz</b> Label Tamla Producer Henry Cosby, Clarence Paul Stevie Wonder chronology The <b>Jazz</b> Soul of Little Stevie (1962) Tribute to Uncle Ray (1962) Recorded Live: The 12 Year Old Genius (1963) Professional ratings Review scores Source Rating Allmusic...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">165 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/The_Best_of_Ray_Charles">
              The Best of Ray Charles
            </a>
              <cite>The Best of Ray Charles The Best of Ray Charles is a compilation album released in 1970 on the Atlantic <b>Jazz</b> label, featuring previously released instrumental (non-vocal) tracks recorded by Ray Charles between November 1956 and November 1958. The Best of Ray Charles Greatest hits album by Ray Charles Released 1970 Genre R&amp;B, <b>Jazz</b> Length 34:06 Label Atlantic The instrumental, "Rockhouse" would later be covered, as "Ray's Rockhouse" (1985), by The Manhattan Transfer with lyrics by Jon Hendricks....</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">79 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/The_Genius_Hits_the_Road">
              The Genius Hits the Road
            </a>
              <cite>...a hit single, "Georgia on My Mind". The Genius Hits the Road Studio album by Ray Charles Released September 1960 Recorded March 25 and 29, 1960 in New York City Genre R&amp;B, blues, <b>jazz</b> Length 33:37 Label ABC-Paramount 335 Producer Sid Feller Ray Charles chronology Genius + Soul = <b>Jazz</b> (1961) The Genius Hits the Road (1960) Dedicated to You (1961) Singles from The Genius Hits the Road "Georgia on My Mind" Released: September 1960 Professional ratings Review scores Source Rating Allmusic Warr.org...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">127 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Ray_Charles_at_Newport">
              Ray Charles at Newport
            </a>
              <cite>...unissued song from the 1958 Newport concert, "Swanee River Rock". Professional ratings Review scores Source Rating Allmusic link Discogs link Ray Charles at Newport Live album by Ray Charles Released November 1958 Recorded July 5, 1958 Venue Newport <b>Jazz</b> Festival, Newport, Rhode Island Genre R&amp;B Length 40:28 Label Atlantic Producer Tom Dowd (engineer) Ray Charles chronology The Great Ray Charles (1957) Ray Charles at Newport (1958) Yes Indeed! (1958) Re-issue cover 1987 re-issue/compilation...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">152 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Here_We_Go_Again:_Celebrating_the_Genius_of_Ray_Charles">
              Here We Go Again: Celebrating the Genius of Ray Charles
            </a>
              <cite>...and <b>jazz</b> trumpeter Wynton Marsalis. It was recorded during concerts at the Rose Theater in New York City, on February 9 and 10, 2009. The album received mixed reviews, in which the instrumentation of Marsalis' orchestra was praised by the critics. Here We Go Again: Celebrating the Genius of Ray Charles Live album by Willie Nelson and Wynton Marsalis Released March 29, 2011 (2011-03-29) Recorded February 9 –10 2009 Venue Rose Theater, New York Genre <b>Jazz</b>, country Length 61:49 Label Blue Note......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">167 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Confession_Blues">
              Confession Blues
            </a>
              <cite>...<b>jazz</b> Length 2:31 Label Down Beat Records Songwriter(s) R. C. Robinson (Ray Charles) Charles moved to Seattle in 1948, where he formed The McSon Trio with guitarist G. D. "Gossie" McKee and bass player Milton S. Garret. In late 1948, Jack Lauderdale of Down Beat Records heard Charles play at the Seattle <b>jazz</b> club, The Rocking Chair. The next day, Lauderdale took Charles and his trio to a Seattle recording studio where they recorded "Confession Blues" and "I Love You, I Love You". In February......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">284 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Genius_Loves_Company">
              Genius Loves Company
            </a>
              <cite>...<b>jazz</b> and pop standards performed by Charles and several guest musicians, such as Natalie Cole, Elton John, James Taylor, Norah Jones, B.B. King, Gladys Knight, Diana Krall, Van Morrison, Willie Nelson and Bonnie Raitt. Genius Loves Company was the last album recorded and completed by Charles before his death in June 2004. Genius Loves Company Studio album by Ray Charles Released August 31, 2004 Recorded June 2003–March 2004 Genre Rhythm and blues, soul, country, blues, <b>jazz</b>, pop Length 54:03......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">325 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Love_Country_Style">
              Love Country Style
            </a>
              <cite>Love Country Style Love Country Style is a studio album by Ray Charles released in June 1970 on Charles' Tangerine Records label. Love Country Style Studio album by Ray Charles Released June 1970 Genre R&amp;B Length 35:25 Label ABC/Tangerine Producer Joe Adams Ray Charles chronology My Kind of <b>Jazz</b> (1970) Love Country Style (1970) Volcanic Action of My Soul (1971) Professional ratings Review scores Source Rating Allmusic Christgau's Record Guide B...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">72 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Doing_His_Thing">
              Doing His Thing
            </a>
              <cite>Doing His Thing Doing His Thing is a 1969 studio album by Ray Charles, released by Tangerine Records. The cover artwork was by Lafayette Chew. Doing His Thing Studio album by Ray Charles Released May 1969 Recorded RPM Studios, Los Angeles, California Genre R&amp;B, soul Length 32:33 Label ABC/Tangerine Producer Joe Adams Ray Charles chronology I'm All Yours Baby (1969) Doing His Thing (1969) My Kind of <b>Jazz</b> (1970)...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">70 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/The_Inspiration_I_Feel">
              The Inspiration I Feel
            </a>
              <cite>The Inspiration I Feel The Inspiration I Feel is an album by flautist Herbie Mann featuring tunes associated with Ray Charles recorded in 1968 and released on the Atlantic label. The Inspiration I Feel Studio album by Herbie Mann Released 1968 Recorded May 6 &amp; 7, 1968 New York City Genre <b>Jazz</b> Length 34:28 Label Atlantic SD 1513 Producer Nesuhi Ertegun, Joel Dorn Herbie Mann chronology Windows Opened (1968) The Inspiration I Feel (1968) Memphis Underground (1968)...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">78 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Milt_Turner">
              Milt Turner
            </a>
              <cite>...Turner After graduating from Pearl High School, he attended Tennessee State University, where he coincided with Hank Crawford, who he later recommended to join him in Ray Charles' band when he took over from William Peeples in the late 1950s. Milton Turner (1930-1993) was a <b>jazz</b> drummer. In 1962, he was a member of Phineas Newborn's trio with Leroy Vinnegar, on whose solo albums he would later appear, and in the early 1960s, Turner also recorded with Teddy Edwards. He never recorded as a leader....</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">87 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Rare_Genius">
              Rare Genius
            </a>
              <cite>...studio recordings and demos made in the 1970s, 1980s and 1990s together with some contemporary instrumental and backing vocal parts. Rare Genius: The Undiscovered Masters Remix album by Ray Charles Released 2010 Genre Soul Length 41:36 Label Concord Producer Ray Charles, John Burk Ray Charles chronology Ray Sings, Basie Swings (2006) Rare Genius: The Undiscovered Masters (2010) Professional ratings Review scores Source Rating Allmusic (link) PopMatters (link) All About <b>Jazz</b> (link) favorable...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">91 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Tangerine_Records_(1962)">
              Tangerine Records (1962)
            </a>
              <cite>...in 1962. ABC-Paramount Records promoted and distributed it. Early singles labels were orange and later became black, red and white. Many of the later recordings are now sought after in "Northern Soul" circles. In 1973 Charles left ABC, closed Tangerine and started Crossover Records. Ray Charles Enterprises owns the catalog. Tangerine Records Parent company ABC-Paramount Records Founded 1962 Founder Ray Charles Defunct 1973 Distributor(s) ABC-Paramount Records Genre R&amp;B, soul music, <b>jazz</b> music...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">87 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Ray_Sings,_Basie_Swings">
              Ray Sings, Basie Swings
            </a>
              <cite>...instrumental tracks by the contemporary Count Basie Orchestra. Professional ratings Review scores Source Rating AllMusic Ray Sings, Basie Swings Compilation album by Ray Charles, Count Basie Orchestra Released October 3, 2006 (2006-10-03) Recorded Mid-1970s, February - May 2006 Studio Los Angeles Genre Soul, <b>jazz</b>, Swing Label Concord/Hear Music Producer Gregg Field Ray Charles chronology Genius &amp; Friends (2005) Ray Sings, Basie Swings (2006) Rare Genius: The Undiscovered Masters (2010)...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">91 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/I_Remember_Brother_Ray">
              I Remember Brother Ray
            </a>
              <cite>...tribute to his bandleader and mentor Ray Charles, which was recorded in 2004 and released on the HighNote label the following year. I Remember Brother Ray Studio album by David "Fathead" Newman Released January 11, 2005 Recorded August 14, 2004 Studio Van Gelder Studio, Englewood Cliffs, NJ Genre <b>Jazz</b> Length 50:39 Label HighNote HCD 7135 Producer David "Fathead" Newman, Houston Person David "Fathead" Newman chronology Song for the New Man (2004) I Remember Brother Ray (2005) Cityscape (2006)...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">96 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Light_Out_of_Darkness_(A_Tribute_to_Ray_Charles)">
              Light Out of Darkness (A Tribute to Ray Charles)
            </a>
              <cite>...to Ray Charles) is a 1993 studio album by Shirley Horn, recorded in tribute to Ray Charles. Light Out of Darkness (A Tribute to Ray Charles) Studio album by Shirley Horn Released 1993 Recorded April 30 and May 1–3, 1993, Clinton Recording Studios, New York City Genre Vocal <b>jazz</b> Length 62:53 Label Verve Producer Shirley Horn, Sheila Mathis, Richard Seidel, Lynn Butterer Shirley Horn chronology Here's to Life (1992) Light Out of Darkness (A Tribute to Ray Charles) (1993) I Love You, Paris (1994)...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">100 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Soul_Meeting">
              Soul Meeting
            </a>
              <cite>...Brothers, on a 2 CD compilation together with other 'bonus' tracks from the same recording sessions. Professional ratings Review scores Source Rating Down Beat (Original Lp release) AllMusic link Soul Meeting Studio album by Ray Charles, Milt Jackson Released 1961 Recorded April 10, 1958 Genre R&amp;B, <b>jazz</b> Length 37:43 Label Atlantic Producer Tom Dowd Ray Charles chronology The Genius Sings the Blues (1961) Soul Meeting (1961) The Genius After Hours (1961) Alternative cover compilation CD re-issue...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">114 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Ray_Charles_in_Concert">
              Ray Charles in Concert
            </a>
              <cite>...between 1958 and 1975. In Concert Compilation album by Ray Charles Released 2003 Recorded Newport <b>Jazz</b> Festival (1958 July 5), Herndon Stadium Atlanta (1959 May 19), Sportpalast Berlin (1962 March 6), Shrine Auditorium Los Angeles (1964 September 20), Tokyo (1975 November 27) and Yokohama (1975 November 30) Genre R&amp;B, soul Length 2 hours Label Rhino Handmade Producer Nesuhi Ertegun (Newport), Zenas Sears (Atlanta), Norman Granz (Berlin), Sid Feller (Los Angeles) and Ray Charles (Tokyo / Yokohama)...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">118 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Sticks_and_Stones_(Titus_Turner_song)">
              Sticks and Stones (Titus Turner song)
            </a>
              <cite>...in a 1960 version by Ray Charles, who added the Latin drum part. It was his first R&amp;B hit with ABC-Paramount, followed in 1961 with "Hit The Road Jack". The song was also covered by Jerry Lee Lewis, The Zombies, Wanda Jackson and The Kingsmen, as well as Joe Cocker on Mad Dogs and Englishmen, and Elvis Costello in 1994 on the extended play version of Kojak Variety. In 1997, <b>jazz</b> singer Roseanna Vitro included the tune in her tribute to Charles, Catchin’ Some Rays: The Music of Ray Charles....</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">113 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Do_the_Twist!_with_Ray_Charles">
              Do the Twist! with Ray Charles
            </a>
              <cite>...the Twist! with Ray Charles Greatest hits album by Ray Charles Released 1961 Recorded 1954-1960 Genre R&amp;B, Soul, <b>Jazz</b> Length 32:39 Label Atlantic Ray Charles chronology The Genius Sings the Blues (1961) Do the Twist! with Ray Charles (1961) Soul Meeting (1961) Professional ratings Review scores Source Rating Allmusic (link) The Rolling Stone Record Guide In 1963, the album got a new cover and was renamed The Greatest Ray Charles. Track listing and catalog number (Atlantic 8054) remained the same....</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">120 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/The_Great_Ray_Charles">
              The Great Ray Charles
            </a>
              <cite>...<b>jazz</b> album. Later CD re-issues often include as a bonus, six of eight tracks from The Genius After Hours. The original cover was by Marvin Israel. Professional ratings Review scores Source Rating Allmusic The Great Ray Charles Studio album by Ray Charles Released August 1957 Recorded April 30 - November 26, 1956 in New York City Genre Bebop Length 37:37 Label Atlantic Producer Ahmet Ertegün, Jerry Wexler Ray Charles chronology Ray Charles (or, Hallelujah I Love Her So) (1957) The Great Ray......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">127 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Ray_Charles_Live">
              Ray Charles Live
            </a>
              <cite>...<b>Jazz</b> Festival in 1958 and at Herndon Stadium in Atlanta in 1959, respectively). Later CD re-issues of this compilation include an additional, previously unreleased, track from the 1958 Newport concert, "Swanee River Rock." Live Live album by Ray Charles Released 1973 Recorded July 5, 1958 / May 28, 1959 Genre Soul, R&amp;B Length 71:55 Label Atlantic 503 Producer Nesuhi Ertegün / Zenas Sears Ray Charles chronology From the Pages of My Mind (1986) Live (1973) Just Between Us (1988) Professional......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">133 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Blue_Funk_(Ray_Charles_song)">
              Soul Brothers
            </a>
              <cite>...on the original LP releases. Soul Brothers Studio album by Ray Charles, Milt Jackson Released June 1958 Recorded September 12, 1957 (Tracks 1-2) and April 10, 1958 (Tracks 3-7), in New York City Genre R&amp;B, <b>jazz</b> Length 38:42 Label Atlantic, Studio One Producer Nesuhi Ertegun Ray Charles chronology Yes Indeed! (1958) Soul Brothers (1958) What'd I Say (1959) alternate release cover compilation CD / re-issue Professional ratings Review scores Source Rating AllMusic Down Beat (Original Lp release)...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">135 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Soul_Brothers">
              Soul Brothers
            </a>
              <cite>...on the original LP releases. Soul Brothers Studio album by Ray Charles, Milt Jackson Released June 1958 Recorded September 12, 1957 (Tracks 1-2) and April 10, 1958 (Tracks 3-7), in New York City Genre R&amp;B, <b>jazz</b> Length 38:42 Label Atlantic, Studio One Producer Nesuhi Ertegun Ray Charles chronology Yes Indeed! (1958) Soul Brothers (1958) What'd I Say (1959) alternate release cover compilation CD / re-issue Professional ratings Review scores Source Rating AllMusic Down Beat (Original Lp release)...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">135 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Ray_Charles_and_Betty_Carter">
              Ray Charles and Betty Carter
            </a>
              <cite>...Betty Carter Released August 1961 Recorded August 23, 1960 - June 14, 1961 Genre <b>Jazz</b> Length 41:38 Label ABC Producer Sid Feller Ray Charles chronology Dedicated to You (1961) Ray Charles and Betty Carter (1961) The Genius Sings the Blues (1961) Betty Carter chronology The Modern Sound of Betty Carter (1960) Ray Charles and Betty Carter (1961) 'Round Midnight (1962) Alternative cover / re-issue 1998 Rhino CD re-issue with Dedicated to You Professional ratings Review scores Source Rating Allmusic...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">158 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Ingredients_in_a_Recipe_for_Soul">
              Ingredients in a Recipe for Soul
            </a>
              <cite>...<b>jazz</b> Label ABC 465 Producer Sid Feller Ray Charles chronology Modern Sounds in Country and Western Music, Vol. 2 (1962) Ingredients in a Recipe for Soul (1963) Sweet &amp; Sour Tears (1964) Alternative cover 1997 Rhino CD re-issue with Have a Smile with Me In 1990, the album was released on compact disc by DCC with four bonus tracks. In 1997, it was packaged together with 1964's Have a Smile with Me on a two-for-one CD reissue on Rhino with historical liner notes. Professional ratings Review scores......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">162 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/The_Genius_Sings_the_Blues">
              The Genius Sings the Blues
            </a>
              <cite>...<b>jazz</b>, and southern R&amp;B. The photo for the album cover was taken by renowned photographer Lee Friedlander. The Genius Sings the Blues was reissued in 2003 by Rhino Entertainment with liner notes by Billy Taylor. The Genius Sings the Blues Compilation album by Ray Charles Released October 1961 Recorded 1952–1960 Genre Rhythm and blues, piano blues, soul Length 34:19 Label Atlantic SD-8052 Producer Ahmet Ertegün, Jerry Wexler Ray Charles chronology Ray Charles and Betty Carter (1961) The Genius......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">162 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/The_Genius_of_Ray_Charles">
              The Genius of Ray Charles
            </a>
              <cite>...<b>jazz</b>, gospel, and blues, for swinging pop with big band arrangements. It comprises a first half of big band songs and a second half of string-backed ballads. The Genius of Ray Charles sold fewer than 500,000 copies and charted at number 17 on the Billboard 200. "Let the Good Times Roll" and "Don't Let the Sun Catch You Cryin'" were released as singles in 1959. The Genius of Ray Charles Studio album by Ray Charles Released October 1959 Recorded May 6 and June 23, 1959 at 6 West Recording in New......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">172 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Ray_Charles_in_Person">
              Ray Charles in Person
            </a>
              <cite>...Soul = <b>Jazz</b> (1961) Re-issue cover 1987 re-issue / compilation Professional ratings Review scores Source Rating Allmusic The album was recorded by the concert sponsor, radio station WAOK. The station's lead disk jockey, Zenas "Daddy" Sears, recorded the album from the audience using a single microphone. The album is noted for its technical excellence in balancing band, singer, and audience, and also for its documentation of the jazzy R&amp;B Ray Charles sound prior to his great crossover success....</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">176 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Don&apos;t_Let_the_Sun_Catch_You_Cryin&apos;">
              Don&apos;t Let the Sun Catch You Cryin&apos;
            </a>
              <cite>...R&amp;B Sides" and No. 95 on the Billboard Hot 100. It was also recorded by Jackie DeShannon on her 1965 album This is Jackie De Shannon, Paul McCartney on his 1990 live album Tripping the Live Fantastic, Jex Saarelaht and Kate Ceberano on their album Open the Door - Live at Mietta's (1992) and <b>jazz</b> singer Roseanna Vitro on her 1997 album Catchin’ Some Rays: The Music of Ray Charles. Karin Krog and Steve Kuhn include it on their 2005 album, Together Again. Steve Alaimo released a version in 1963...</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">185 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/I_Don&apos;t_Need_No_Doctor">
              I Don&apos;t Need No Doctor
            </a>
              <cite>...<b>jazz</b> guitar player John Scofield recorded a version for his album That's What I Say: John Scofield Plays the Music of Ray Charles in 2005, featuring the blues guitarist John Mayer on additional guitar and vocals. Mayer covered the song again with his band during his tour in summer 2007. A recorded live version from a Los Angeles show during that tour is available on Mayer's CD/DVD release Where the Light Is. A Ray Charles tribute album also provided the impetus for <b>jazz</b> singer Roseanna Vitro's......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">558 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/If_You_Go_Away">
              If You Go Away
            </a>
              <cite>...<b>Jazz</b> Length 3:49 Label Epic Records Songwriter(s) Jacques Brel, Rod McKuen Producer(s) Bob Morgan Damita Jo singles chronology "Gotta Travel On" (1965) "If You Go Away" (1966) "Walk Away" (1967) Damita Jo reached #10 on the Adult Contemporary chart and #68 on the Billboard Hot 100 in 1966 for her version of the song. Terry Jacks recorded a version of the song which was released as a single in 1974 and reached #29 on the Adult Contemporary chart, #68 on the Billboard Hot 100, and went to #8 in......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">204 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Anthology_(Ray_Charles_album)">
              Anthology (Ray Charles album)
            </a>
              <cite>...<b>jazz</b> piano blues Length 67:25 (original), 66:18 (re-release) Label Rhino Producer Ray Charles Steve Hoffman Richard Foos Ray Charles chronology Just Between Us (1988) Anthology (1988) Would You Believe? (1990) Posthumous cover Professional ratings Review scores Source Rating AllMusic Charles, who retained the master rights (currently controlled by his estate since his June 2004 passing) to his ABC-Paramount recordings, supervised a remixing of the 20 songs on this compilation especially for this......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">265 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Ray_Charles">
              Ray Charles
            </a>
              <cite>...<b>jazz</b> rock and roll Occupation(s) musician singer songwriter composer Instruments Vocals piano Years active 1947–2004 Labels Atlantic ABC Tangerine Warner Bros. Swing Time Concord Columbia Flashback Associated acts The Raelettes USA for Africa Billy Joel Gladys Knight Website raycharles.com Charles pioneered the soul music genre during the 1950s by combining blues, rhythm and blues, and gospel styles into the music he recorded for Atlantic. He contributed to the integration of country music......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">416 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/The_Pages_of_My_Mind">
              Ray Charles
            </a>
              <cite>...<b>jazz</b> rock and roll Occupation(s) musician singer songwriter composer Instruments Vocals piano Years active 1947–2004 Labels Atlantic ABC Tangerine Warner Bros. Swing Time Concord Columbia Flashback Associated acts The Raelettes USA for Africa Billy Joel Gladys Knight Website raycharles.com Charles pioneered the soul music genre during the 1950s by combining blues, rhythm and blues, and gospel styles into the music he recorded for Atlantic. He contributed to the integration of country music......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">416 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Here_We_Go_Again_(Ray_Charles_song)">
              Here We Go Again (Ray Charles song)
            </a>
              <cite>...was first covered in an instrumental <b>jazz</b> format, and many of the more recent covers have been sung as duets, such as one with Willie Nelson and Norah Jones with Wynton Marsalis accompanying. The song was released on their 2011 tribute album Here We Go Again: Celebrating the Genius of Ray Charles. The song lent its name to Red Steagall's 2007 album as well. Cover versions have appeared on compilation albums by a number of artists, even some who did not release "Here We Go Again" as a single....</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">417 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Modern_Sounds_in_Country_and_Western_Music">
              Modern Sounds in Country and Western Music
            </a>
              <cite>...<b>jazz</b>. Charles produced the album with Sid Feller, who helped the singer select songs to record, and performed alongside saxophonist Hank Crawford, a string section conducted by Marty Paich, and a big band arranged by Gil Fuller and Gerald Wilson. Modern Sounds in Country and Western Music was an immediate critical and commercial success. The album and its four hit singles brought Charles greater mainstream notice and recognition in the pop market, as well as airplay on both R&amp;B and country radio......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">424 words</div>
)SEARCHRESULT"
};

TEST_F(TaskbarlessServerTest, searchResults)
{
  struct TestData
  {
    struct PaginationEntry
    {
      std::string label;
      size_t start;
      bool selected;
    };

    std::string pattern;
    size_t resultsPerPage;
    size_t totalResultCount;
    size_t firstResultIndex;
    std::vector<std::string> results;
    std::vector<PaginationEntry> pagination;

    static std::string makeUrl(const std::string pattern, int start, size_t resultsPerPage)
    {
      std::string url = "/ROOT/search?pattern=" + pattern + "&content=zimfile";

      if ( start >= 0 ) {
        url += "&start=" + to_string(start);
      }

      if ( resultsPerPage != 0 ) {
        url += "&pageLength=" + to_string(resultsPerPage);
      }

      return url;
    }

    std::string url() const
    {
      return makeUrl(pattern, -1, resultsPerPage);
    }

    std::string expectedHeader() const
    {
      if ( totalResultCount == 0 ) {
        return "\n        No results were found for <b>\"" + pattern + "\"</b>";
      }

      std::string header = R"(  Results
        <b>
          FIRSTRESULT-LASTRESULT
        </b> of <b>
          RESULTCOUNT
        </b> for <b>
          "PATTERN"
        </b>
      )";

      const size_t lastResultIndex = firstResultIndex + results.size() - 1;
      header = replace(header, "FIRSTRESULT", to_string(firstResultIndex));
      header = replace(header, "LASTRESULT",  to_string(lastResultIndex));
      header = replace(header, "RESULTCOUNT", to_string(totalResultCount));
      header = replace(header, "PATTERN",     pattern);
      return header;
    }

    std::string expectedResultsString() const
    {
      if ( results.empty() ) {
        return "\n        ";
      }

      std::string s;
      for ( const auto& r : results ) {
        s += "\n          <li>";
        s += r;
        s += "          </li>";
      }
      return s;
    }

    std::string expectedFooter() const
    {
      if ( pagination.empty() ) {
        return "\n      ";
      }

      std::ostringstream oss;
      oss << "\n        <ul>\n";
      for ( const auto& p : pagination ) {
        const auto url = makeUrl(pattern, p.start, resultsPerPage);
        oss << "            <li>\n";
        oss << "              <a ";
        if ( p.selected ) {
          oss << "class=\"selected\"";
        }
        oss << "\n                 href=\"" << url << "\">\n";
        oss << "                " << p.label << "\n";
        oss << "              </a>\n";
        oss << "            </li>\n";
      }
      oss << "        </ul>";
      return oss.str();
    }

    std::string expectedHtml() const
    {
      return makeSearchResultsHtml(
               pattern,
               expectedHeader(),
               expectedResultsString(),
               expectedFooter()
      );
    }

    TestContext testContext() const
    {
      return TestContext{ { "url", url() } };
    }
  };

  const TestData testData[] = {
    {
      /* pattern */          "velomanyunkan",
      /* resultsPerPage */   0,
      /* totalResultCount */ 0,
      /* firstResultIndex */ 0,
      /* results */          {},
      /* pagination */       {}
    },

    {
      /* pattern */          "razaf",
      /* resultsPerPage */   0,
      /* totalResultCount */ 1,
      /* firstResultIndex */ 1,
      /* results */ {
R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/We_Gonna_Move_to_the_Outskirts_of_Town">
              We Gonna Move to the Outskirts of Town
            </a>
              <cite>...to the Outskirts of Town "We Gonna Move to the Outskirts of Town" is a country blues song recorded September 3, 1936 by Casey Bill Weldon (voice and guitar). The song has been covered by many other musicians, most often under the title "I'm Gonna Move to the Outskirts of Town", and sometimes simply Outskirts of Town. All recordings seem to credit Weldon as songwriter, often as Weldon or as Will Weldon or as William Weldon. Some cover versions give credit also to Andy <b>Razaf</b> and/or to Roy Jacobs....</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">93 words</div>
)SEARCHRESULT"
      },
      /* pagination */ {}
    },

    {
      /* pattern */          "beatles",
      /* resultsPerPage */   0,
      /* totalResultCount */ 2,
      /* firstResultIndex */ 1,
      /* results */ {
R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/Eleanor_Rigby">
              Eleanor Rigby
            </a>
              <cite>...a mainly rock and roll- and pop-oriented act to a more experimental, studio-based band. With a double string quartet arrangement by George Martin and striking lyrics about loneliness, "Eleanor Rigby" broke sharply with popular music conventions, both musically and lyrically. Richie Unterberger of AllMusic cites the band's "singing about the neglected concerns and fates of the elderly" on the song as "just one example of why the Beatles' appeal reached so far beyond the traditional rock audience"....</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">201 words</div>
)SEARCHRESULT",

R"SEARCHRESULT(
            <a href="/ROOT/zimfile/A/True_to_Life_(Ray_Charles_album)">
              True to Life (Ray Charles album)
            </a>
              <cite>...Been Going On?", and The Beatles' "Let It Be". The album was arranged by Larry Muhoberac, Roger Newman, Sid Feller and Ray Charles. True to Life Studio album by Ray Charles Released October 1977 Recorded 1977; R.P.M International Studios, Los Angeles, California Genre R&amp;B, soul Length 38:44 Label Atlantic Producer Ray Charles Ray Charles chronology Porgy and Bess (1976) True to Life (1977) Love &amp; Peace (1978) Professional ratings Review scores Source Rating Allmusic Christgau's Record Guide A......</cite>
              <div class="book-title">from Ray Charles</div>
              <div class="informations">134 words</div>
)SEARCHRESULT"
      },
      /* pagination */ {}
    },

    {
      /* pattern */          "jazz",
      /* resultsPerPage */   100,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 1,
      /* results */ LARGE_SEARCH_RESULTS,
      /* pagination */ {}
    },

    {
      /* pattern */          "jazz",
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 1,
      /* results */ {
        LARGE_SEARCH_RESULTS[0],
        LARGE_SEARCH_RESULTS[1],
        LARGE_SEARCH_RESULTS[2],
        LARGE_SEARCH_RESULTS[3],
        LARGE_SEARCH_RESULTS[4],
      },

      /* pagination */ {
        { "◀", 0,  false },
        { "1", 0,  true  },
        { "2", 5,  false },
        { "3", 10, false },
        { "4", 15, false },
        { "5", 20, false },
        { "▶", 40, false },
      }
    },
  };

  for ( const auto& t : testData ) {
    const auto r = zfs1_->GET(t.url().c_str());
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(r->body, t.expectedHtml()) << t.testContext();
  }
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
  ZimFileServer zfs2(PORT + 1, /*withTaskbar=*/true, ZIMFILES);
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
