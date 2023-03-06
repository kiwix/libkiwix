
#define CPPHTTPLIB_ZLIB_SUPPORT 1
#include "./httplib.h"
#include "gtest/gtest.h"

#define SERVER_PORT 8001
#include "server_testing_tools.h"

#include "../src/tools/stringTools.h"


const std::string ROOT_PREFIX("/ROOT%23%3F");

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

enum ResourceKind
{
  ZIM_CONTENT,
  STATIC_CONTENT,
  DYNAMIC_CONTENT,
};

struct Resource
{
  ResourceKind kind;
  const char* url;

  bool etag_expected() const { return kind != STATIC_CONTENT; }
};

std::ostream& operator<<(std::ostream& out, const Resource& r)
{
  out << "url: " << r.url;
  return out;
}

typedef std::vector<Resource> ResourceCollection;

const ResourceCollection resources200Compressible{
  { DYNAMIC_CONTENT, "/ROOT%23%3F/" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/viewer" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/viewer?cacheid=whatever" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/autoComplete.min.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/autoComplete.min.js?cacheid=1191aaaf" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/css/autoComplete.css" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/css/autoComplete.css?cacheid=08951e06" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/i18n.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/i18n.js?cacheid=2cf0f8c5" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/index.css" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/index.css?cacheid=be514520" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/index.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/index.js?cacheid=cafa3d61" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/iso6391To3.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/iso6391To3.js?cacheid=ecde2bb3" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/isotope.pkgd.min.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/isotope.pkgd.min.js?cacheid=2e48d392" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/mustache.min.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/mustache.min.js?cacheid=bd23c4fb" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/taskbar.css" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/taskbar.css?cacheid=8fc2cc83" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/viewer.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/viewer.js?cacheid=b9a574d4" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/fonts/Poppins.ttf" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/fonts/Poppins.ttf?cacheid=af705837" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/fonts/Roboto.ttf" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/fonts/Roboto.ttf?cacheid=84d10248" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/i18n/test.json" },
  // TODO: implement cache management of i18n resources
  //{ STATIC_CONTENT, "/ROOT%23%3F/skin/i18n/test.json?cacheid=unknown" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/search" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/v2/root.xml" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/v2/entries" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/v2/partial_entries" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/search?content=zimfile&pattern=a" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/suggest?content=zimfile&term=ray" },

  { ZIM_CONTENT,     "/ROOT%23%3F/content/zimfile/A/index" },
  { ZIM_CONTENT,     "/ROOT%23%3F/content/zimfile/A/Ray_Charles" },

  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/content/A/index" },
  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/content/A/Ray_Charles" },
};

const ResourceCollection resources200Uncompressible{
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/bittorrent.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/bittorrent.png?cacheid=4f5c6882" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/blank.html" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/blank.html?cacheid=6b1fa032" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/caret.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/caret.png?cacheid=22b942b4" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/download.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/download.png?cacheid=a39aa502" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/android-chrome-192x192.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/android-chrome-192x192.png?cacheid=bfac158b" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/android-chrome-512x512.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/android-chrome-512x512.png?cacheid=380c3653" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/apple-touch-icon.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/apple-touch-icon.png?cacheid=f86f8df3" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/browserconfig.xml" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/browserconfig.xml?cacheid=f29a7c4a" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/favicon-16x16.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/favicon-16x16.png?cacheid=a986fedc" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/favicon-32x32.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/favicon-32x32.png?cacheid=79ded625" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/favicon.ico" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/favicon.ico?cacheid=92663314" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/mstile-144x144.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/mstile-144x144.png?cacheid=c25a7641" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/mstile-150x150.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/mstile-150x150.png?cacheid=6fa6f467" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/mstile-310x150.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/mstile-310x150.png?cacheid=e0ed9032" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/mstile-310x310.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/mstile-310x310.png?cacheid=26b20530" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/mstile-70x70.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/mstile-70x70.png?cacheid=64ffd9dc" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/safari-pinned-tab.svg" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/safari-pinned-tab.svg?cacheid=8d487e95" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/favicon/site.webmanifest" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/favicon/site.webmanifest?cacheid=bc396efb" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/hash.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/hash.png?cacheid=f836e872" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/magnet.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/magnet.png?cacheid=73b6bddf" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/search-icon.svg" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/search-icon.svg?cacheid=b10ae7ed" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/search_results.css" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/search_results.css?cacheid=76d39c84" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/languages.js" },
  { STATIC_CONTENT, "/ROOT%23%3F/skin/languages.js?cacheid=b00b12db" },

  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/meta/Title" },
  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/meta/Description" },
  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/meta/Language" },
  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/meta/Name" },
  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/meta/Tags" },
  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/meta/Date" },
  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/meta/Creator" },
  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/meta/Publisher" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/root.xml" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/searchdescription.xml" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/v2/categories" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/v2/languages" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/v2/searchdescription.xml" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/v2/illustration/6f1d19d0-633f-087b-fb55-7ac324ff9baf?size=48" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/catch/external?source=www.example.com" },

  { ZIM_CONTENT,     "/ROOT%23%3F/content/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg" },

  { ZIM_CONTENT,     "/ROOT%23%3F/content/corner_cases%23%26/empty.html" },
  { ZIM_CONTENT,     "/ROOT%23%3F/content/corner_cases%23%26/empty.css" },
  { ZIM_CONTENT,     "/ROOT%23%3F/content/corner_cases%23%26/empty.js" },


  // The following url's responses are too small to be compressed
  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/root.xml" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/searchdescription.xml" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/suggest?content=zimfile" },
  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/meta/Creator" },
  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/meta/Title" },
};

ResourceCollection all200Resources()
{
  return concat(resources200Compressible, resources200Uncompressible);
}

TEST(indexTemplateStringTest, emptyIndexTemplate) {
  const int PORT = 8001;
  const ZimFileServer::FilePathCollection ZIMFILES {
    "./test/zimfile.zim",
    "./test/corner_cases#&.zim"
  };

  ZimFileServer zfs(PORT, ZimFileServer::DEFAULT_OPTIONS, ZIMFILES, "");
  EXPECT_EQ(200, zfs.GET("/ROOT%23%3F/")->status);
}

TEST(indexTemplateStringTest, indexTemplateCheck) {
  const int PORT = 8001;
  const ZimFileServer::FilePathCollection ZIMFILES {
    "./test/zimfile.zim",
    "./test/corner_cases#&.zim"
  };

  ZimFileServer zfs(PORT, ZimFileServer::DEFAULT_OPTIONS, ZIMFILES, "<!DOCTYPE html><head>"
      "<title>Welcome to kiwix library</title>"
    "</head>"
  "</html>");
  EXPECT_EQ("<!DOCTYPE html><head>"
    "<title>Welcome to kiwix library</title>"
    "</head>"
  "</html>", zfs.GET("/ROOT%23%3F/")->body);
}

TEST_F(ServerTest, 200)
{
  for ( const Resource& res : all200Resources() )
    EXPECT_EQ(200, zfs1_->GET(res.url)->status) << "res.url: " << res.url;
}

TEST_F(ServerTest, 200_IdNameMapper)
{
  EXPECT_EQ(404, zfs1_->GET("/ROOT%23%3F/content/6f1d19d0-633f-087b-fb55-7ac324ff9baf/A/index")->status);
  EXPECT_EQ(200, zfs1_->GET("/ROOT%23%3F/content/zimfile/A/index")->status);
  resetServer(ZimFileServer::NO_NAME_MAPPER);
  EXPECT_EQ(200, zfs1_->GET("/ROOT%23%3F/content/6f1d19d0-633f-087b-fb55-7ac324ff9baf/A/index")->status);
  EXPECT_EQ(404, zfs1_->GET("/ROOT%23%3F/content/zimfile/A/index")->status);
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
      /* url */ "/ROOT%23%3F/",
R"EXPECTEDRESULT(      href="/ROOT%23%3F/skin/index.css?cacheid=be514520"
    <link rel="apple-touch-icon" sizes="180x180" href="/ROOT%23%3F/skin/favicon/apple-touch-icon.png?cacheid=f86f8df3">
    <link rel="icon" type="image/png" sizes="32x32" href="/ROOT%23%3F/skin/favicon/favicon-32x32.png?cacheid=79ded625">
    <link rel="icon" type="image/png" sizes="16x16" href="/ROOT%23%3F/skin/favicon/favicon-16x16.png?cacheid=a986fedc">
    <link rel="manifest" href="/ROOT%23%3F/skin/favicon/site.webmanifest?cacheid=bc396efb">
    <link rel="mask-icon" href="/ROOT%23%3F/skin/favicon/safari-pinned-tab.svg?cacheid=8d487e95" color="#5bbad5">
    <link rel="shortcut icon" href="/ROOT%23%3F/skin/favicon/favicon.ico?cacheid=92663314">
    <meta name="msapplication-config" content="/ROOT%23%3F/skin/favicon/browserconfig.xml?cacheid=f29a7c4a">
        src: url("/ROOT%23%3F/skin/fonts/Poppins.ttf?cacheid=af705837") format("truetype");
          src: url("/ROOT%23%3F/skin/fonts/Roboto.ttf?cacheid=84d10248") format("truetype");
    <script type="module" src="/ROOT%23%3F/skin/i18n.js?cacheid=2cf0f8c5" defer></script>
    <script type="text/javascript" src="/ROOT%23%3F/skin/languages.js?cacheid=b00b12db" defer></script>
    <script src="/ROOT%23%3F/skin/isotope.pkgd.min.js?cacheid=2e48d392" defer></script>
    <script src="/ROOT%23%3F/skin/iso6391To3.js?cacheid=ecde2bb3"></script>
    <script type="text/javascript" src="/ROOT%23%3F/skin/index.js?cacheid=cafa3d61" defer></script>
        <img src="/ROOT%23%3F/skin/feed.png?cacheid=56a672b1"
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT%23%3F/skin/index.css",
R"EXPECTEDRESULT(    background-image: url('../skin/search-icon.svg?cacheid=b10ae7ed');
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT%23%3F/skin/index.js",
R"EXPECTEDRESULT(                                <img src="${root}/skin/download.png?cacheid=a39aa502" alt="${$t("direct-download-alt-text")}" />
                                <img src="${root}/skin/hash.png?cacheid=f836e872" alt="${$t("hash-download-alt-text")}" />
                                <img src="${root}/skin/magnet.png?cacheid=73b6bddf" alt="${$t("magnet-alt-text")}" />
                                <img src="${root}/skin/bittorrent.png?cacheid=4f5c6882" alt="${$t("torrent-download-alt-text")}" />
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT%23%3F/viewer",
R"EXPECTEDRESULT(    <link type="text/css" href="./skin/taskbar.css?cacheid=8fc2cc83" rel="Stylesheet" />
    <link type="text/css" href="./skin/css/autoComplete.css?cacheid=08951e06" rel="Stylesheet" />
    <script type="module" src="./skin/i18n.js?cacheid=2cf0f8c5" defer></script>
    <script type="text/javascript" src="./skin/languages.js?cacheid=b00b12db" defer></script>
    <script type="text/javascript" src="./skin/viewer.js?cacheid=b9a574d4" defer></script>
    <script type="text/javascript" src="./skin/autoComplete.min.js?cacheid=1191aaaf"></script>
      const blankPageUrl = root + "/skin/blank.html?cacheid=6b1fa032";
          <label for="kiwix_button_show_toggle"><img src="./skin/caret.png?cacheid=22b942b4" alt=""></label>
            src="./skin/blank.html?cacheid=6b1fa032" title="ZIM content" width="100%"
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT%23%3F/content/zimfile/A/index",
      ""
    },
    {
      // Searching in a ZIM file without a full-text index returns
      // a page rendered from static/templates/no_search_result_html
      /* url */ "/ROOT%23%3F/search?content=poor&pattern=whatever",
R"EXPECTEDRESULT(    <link type="text/css" href="/ROOT%23%3F/skin/search_results.css?cacheid=76d39c84" rel="Stylesheet" />
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
  "/ROOT%23%3F/search",
  "/ROOT%23%3F/search?content=zimfile",
  "/ROOT%23%3F/search?content=non-existing-book&pattern=asdfqwerty",
  "/ROOT%23%3F/search?content=non-existing-book&pattern=asd<qwerty",
  "/ROOT%23%3F/search?books.name=non-exsitent-book&pattern=asd<qwerty",
  "/ROOT%23%3F/search?books.id=non-exsitent-id&pattern=asd<qwerty",
  "/ROOT%23%3F/search?books.filter.lang=unk&pattern=asd<qwerty",
  "/ROOT%23%3F/search?pattern=foo",
  "/ROOT%23%3F/search?pattern"
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
  "/ROOT",
  "/ROOT%23%",
  "/ROOT%23%3",
  "/ROOT%23%3Fxyz",
  "/ROOT%23%3F/skin/non-existent-skin-resource",
  "/ROOT%23%3F/skin/autoComplete.min.js?cacheid=wrongcacheid",
  "/ROOT%23%3F/catalog",
  "/ROOT%23%3F/catalog/",
  "/ROOT%23%3F/catalog/non-existent-item",
  "/ROOT%23%3F/catalog/v2/illustration/zimfile?size=48",
  "/ROOT%23%3F/catalog/v2/illustration/6f1d19d0-633f-087b-fb55-7ac324ff9baf?size=96",
  "/ROOT%23%3F/random",
  "/ROOT%23%3F/random?content=non-existent-book",
  "/ROOT%23%3F/random/",
  "/ROOT%23%3F/random/number",
  "/ROOT%23%3F/suggest",
  "/ROOT%23%3F/suggest?content=non-existent-book&term=abcd",
  "/ROOT%23%3F/suggest/",
  "/ROOT%23%3F/suggest/fr",
  "/ROOT%23%3F/search/",
  "/ROOT%23%3F/search/anythingotherthansearchdescription.xml",
  "/ROOT%23%3F/catch/",
  "/ROOT%23%3F/catch/external", // missing ?source=URL
  "/ROOT%23%3F/catch/external?source=",
  "/ROOT%23%3F/catch/anythingotherthanexternal",
  "/ROOT%23%3F/content/zimfile/A/non-existent-article",

  "/ROOT%23%3F/raw/non-existent-book/meta/Title",
  "/ROOT%23%3F/raw/zimfile/wrong-kind/Foo",

  // zimfile has no Favicon nor Illustration_48x48@1 meta item
  "/ROOT%23%3F/raw/zimfile/meta/Favicon",
  "/ROOT%23%3F/raw/zimfile/meta/Illustration_48x48@1",
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

std::string getCacheControlHeader(const httplib::Response& r)
{
  return getHeaderValue(r.headers, "Cache-Control");
}

TEST_F(CustomizedServerTest, NewResourcesCanBeAdded)
{
  // ServerTest.404 verifies that "/ROOT%23%3F/non-existent-item" doesn't exist
  const auto r = zfs1_->GET("/ROOT%23%3F/non-existent-item");
  EXPECT_EQ(r->status, 200);
  EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), "text/plain");
  EXPECT_EQ(r->body, "Hello world!\n");
}

TEST_F(CustomizedServerTest, ContentOfAnyServableUrlCanBeOverriden)
{
  {
    const auto r = zfs1_->GET("/ROOT%23%3F/");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), "text/html");
    EXPECT_EQ(r->body, "<html><head></head><body>Welcome</body></html>\n");
  }

  {
    const auto r = zfs1_->GET("/ROOT%23%3F/skin/index.css");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), "application/json");
    EXPECT_EQ(r->body, "Hello world!\n");
  }

  {
    const auto r = zfs1_->GET("/ROOT%23%3F/zimfile/A/Ray_Charles");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), "ray/charles");
    EXPECT_EQ(r->body, "<html><head></head><body>Welcome</body></html>\n");
  }

  {
    const auto r = zfs1_->GET("/ROOT%23%3F/content/zimfile/A/Ray_Charles");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), "charles/ray");
    EXPECT_EQ(r->body, "<html><head></head><body>Welcome</body></html>\n");
  }

  {
    const auto r = zfs1_->GET("/ROOT%23%3F/search?pattern=la+femme");
    EXPECT_EQ(r->status, 200);
    EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), "text/html");
    EXPECT_EQ(r->body, "Hello world!\n");
  }
}

TEST_F(ServerTest, MimeTypes)
{
  struct TestData {
    const char* const url;
    const char* const mimeType;
  };

  const TestData testData[] = {
    { "/",                                 "text/html; charset=utf-8" },
    { "/viewer",                           "text/html" },
    { "/skin/blank.html",                  "text/html" },
    { "/skin/index.css",                   "text/css" },
    { "/skin/index.js",                    "application/javascript" },
    { "/catalog/v2/searchdescription.xml", "application/opensearchdescription+xml" },
    { "/catalog/v2/root.xml",              "application/atom+xml;profile=opds-catalog;kind=navigation" },
    { "/skin/search-icon.svg",             "image/svg+xml" },
    { "/skin/bittorrent.png",              "image/png" },
    { "/skin/favicon/favicon.ico",         "image/x-icon" },
    { "/skin/i18n/en.json",                "application/json" },
    { "/skin/fonts/Roboto.ttf",            "application/font-ttf" },
    { "/suggest?content=zimfile&term=ray", "application/json; charset=utf-8" },
  };

  for ( const auto& t : testData ) {
    const std::string url= ROOT_PREFIX + t.url;
    const TestContext ctx{ {"url", url} };
    const auto r = zfs1_->GET(url.c_str());
    EXPECT_EQ(getHeaderValue(r->headers, "Content-Type"), t.mimeType) << ctx;
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
  virtual std::string pageTitle() const;
  std::string pageCssLink() const;
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
  </head>
  <body>)FRAG",

  R"FRAG(  </body>
</html>
)FRAG"
  };

  return frag[0]
       + pageTitle()
       + frag[1]
       + pageCssLink()
       + frag[2]
       + expectedBody
       + frag[3];
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
    { /* url */ "/ROOT%23%3F/random?content=non-existent-book",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      No such book: non-existent-book
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/random?content=non-existent-book&userlang=test",
      expected_page_title=="[I18N TESTING] Not Found - Try Again" &&
      expected_body==R"(
    <h1>[I18N TESTING] Content not found, but at least the server is alive</h1>
    <p>
      [I18N TESTING] No such book: non-existent-book. Sorry.
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/suggest?content=no-such-book&term=whatever",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      No such book: no-such-book
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/catalog/",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/catalog/" was not found on this server.
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/catalog/?userlang=test",
      expected_page_title=="[I18N TESTING] Not Found - Try Again" &&
      expected_body==R"(
    <h1>[I18N TESTING] Content not found, but at least the server is alive</h1>
    <p>
      [I18N TESTING] URL not found: /ROOT%23%3F/catalog/
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/catalog/invalid_endpoint",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/catalog/invalid_endpoint" was not found on this server.
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/catalog/invalid_endpoint?userlang=test",
      expected_page_title=="[I18N TESTING] Not Found - Try Again" &&
      expected_body==R"(
    <h1>[I18N TESTING] Content not found, but at least the server is alive</h1>
    <p>
      [I18N TESTING] URL not found: /ROOT%23%3F/catalog/invalid_endpoint
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/content/invalid-book/whatever",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/content/invalid-book/whatever" was not found on this server.
    </p>
    <p>
      Make a full text search for <a href="/ROOT%23%3F/search?pattern=whatever">whatever</a>
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/content/zimfile/invalid-article",
      book_name=="zimfile" &&
      book_title=="Ray Charles" &&
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/content/zimfile/invalid-article" was not found on this server.
    </p>
    <p>
      Make a full text search for <a href="/ROOT%23%3F/search?content=zimfile&pattern=invalid-article">invalid-article</a>
    </p>
)"  },

    { /* url */ R"(/ROOT%23%3F/content/"><svg onload=alert(1)>)",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/content/&quot;&gt;&lt;svg onload%3Dalert(1)&gt;" was not found on this server.
    </p>
    <p>
      Make a full text search for <a href="/ROOT%23%3F/search?pattern=%22%3E%3Csvg%20onload%3Dalert(1)%3E">&quot;&gt;&lt;svg onload=alert(1)&gt;</a>
    </p>
)"  },

    { /* url */ R"(/ROOT%23%3F/content/zimfile/"><svg onload=alert(1)>)",
      book_name=="zimfile" &&
      book_title=="Ray Charles" &&
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/content/zimfile/&quot;&gt;&lt;svg onload%3Dalert(1)&gt;" was not found on this server.
    </p>
    <p>
      Make a full text search for <a href="/ROOT%23%3F/search?content=zimfile&pattern=%22%3E%3Csvg%20onload%3Dalert(1)%3E">&quot;&gt;&lt;svg onload=alert(1)&gt;</a>
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/content/zimfile/invalid-article?userlang=test",
      expected_page_title=="[I18N TESTING] Not Found - Try Again" &&
      book_name=="zimfile" &&
      book_title=="Ray Charles" &&
      expected_body==R"(
    <h1>[I18N TESTING] Content not found, but at least the server is alive</h1>
    <p>
      [I18N TESTING] URL not found: /ROOT%23%3F/content/zimfile/invalid-article
    </p>
    <p>
      [I18N TESTING] Make a full text search for <a href="/ROOT%23%3F/search?content=zimfile&pattern=invalid-article">invalid-article</a>
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/raw/no-such-book/meta/Title",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/raw/no-such-book/meta/Title" was not found on this server.
    </p>
    <p>
      No such book: no-such-book
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/raw/zimfile/XYZ",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/raw/zimfile/XYZ" was not found on this server.
    </p>
    <p>
      XYZ is not a valid request for raw content.
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/raw/zimfile/meta/invalid-metadata",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/raw/zimfile/meta/invalid-metadata" was not found on this server.
    </p>
    <p>
      Cannot find meta entry invalid-metadata
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/raw/zimfile/content/invalid-article",
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/raw/zimfile/content/invalid-article" was not found on this server.
    </p>
    <p>
      Cannot find content entry invalid-article
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/search?content=poor&pattern=whatever",
      expected_page_title=="Fulltext search unavailable" &&
      expected_css_url=="/ROOT%23%3F/skin/search_results.css?cacheid=76d39c84" &&
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
    { /* url */ "/ROOT%23%3F/search",
      expected_body== R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT%23%3F/search" is not a valid request.
    </p>
    <p>
      Too many books requested (4) where limit is 3
    </p>
)"  },
    { /* url */ "/ROOT%23%3F/search?content=zimfile",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT%23%3F/search?content=zimfile" is not a valid request.
    </p>
    <p>
      No query provided.
    </p>
)"  },
    { /* url */ "/ROOT%23%3F/search?content=non-existing-book&pattern=asdfqwerty",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT%23%3F/search?content=non-existing-book&pattern=asdfqwerty" is not a valid request.
    </p>
    <p>
      No such book: non-existing-book
    </p>
)"  },
    { /* url */ "/ROOT%23%3F/search?content=non-existing-book&pattern=a\"<script foo>",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT%23%3F/search?content=non-existing-book&pattern=a%22%3Cscript%20foo%3E" is not a valid request.
    </p>
    <p>
      No such book: non-existing-book
    </p>
)"  },
    // There is a flaw in our way to handle query string, we cannot differenciate
    // between `pattern` and `pattern=`
    { /* url */ "/ROOT%23%3F/search?books.filter.lang=eng&pattern",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT%23%3F/search?books.filter.lang=eng&pattern" is not a valid request.
    </p>
    <p>
      No query provided.
    </p>
)"  },
    { /* url */ "/ROOT%23%3F/search?pattern=foo",
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT%23%3F/search?pattern=foo" is not a valid request.
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
    { /* url */ "/ROOT%23%3F/search?format=xml",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT%23%3F/search?format=xml" is not a valid request.</detail>
<detail>Too many books requested (4) where limit is 3</detail>
)"  },
    { /* url */ "/ROOT%23%3F/search?format=xml&content=zimfile",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT%23%3F/search?format=xml&content=zimfile" is not a valid request.</detail>
<detail>No query provided.</detail>
)"  },
    { /* url */ "/ROOT%23%3F/search?format=xml&content=non-existing-book&pattern=asdfqwerty",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT%23%3F/search?format=xml&content=non-existing-book&pattern=asdfqwerty" is not a valid request.</detail>
<detail>No such book: non-existing-book</detail>
)"  },
    { /* url */ "/ROOT%23%3F/search?format=xml&content=non-existing-book&pattern=a\"<script foo>",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT%23%3F/search?format=xml&content=non-existing-book&pattern=a%22%3Cscript%20foo%3E" is not a valid request.</detail>
<detail>No such book: non-existing-book</detail>
)"  },
    // There is a flaw in our way to handle query string, we cannot differenciate
    // between `pattern` and `pattern=`
    { /* url */ "/ROOT%23%3F/search?format=xml&books.filter.lang=eng&pattern",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT%23%3F/search?format=xml&books.filter.lang=eng&pattern" is not a valid request.</detail>
<detail>No query provided.</detail>
)"  },
    { /* url */ "/ROOT%23%3F/search?format=xml&pattern=foo",
      /* HTTP status code */ 400,
      /* expected response XML */ R"(
<error>Invalid request</error>
<detail>The requested URL "/ROOT%23%3F/search?format=xml&pattern=foo" is not a valid request.</detail>
<detail>Too many books requested (4) where limit is 3</detail>
)"  },
    { /* url */ "/ROOT%23%3F/search?format=xml&content=poor&pattern=whatever",
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

  {
  const auto r = zfs1_->GET("/ROOT%23%3F/content/poor/A/redirect_loop.html");
  EXPECT_EQ(r->status, 500);
  EXPECT_EQ(r->body, expectedBody);
  }
}

TEST_F(ServerTest, UserLanguageList)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/skin/languages.js");
  EXPECT_EQ(r->body,
R"EXPECTEDRESPONSE(const uiLanguages = [
  {
    "الإنجليزية": "ar"
  },
  {
    "বাংলা": "bn"
  },
  {
    "Čeština": "cs"
  },
  {
    "Deutsch": "de"
  },
  {
    "English": "en"
  },
  {
    "français": "fr"
  },
  {
    "עברית": "he"
  },
  {
    "Հայերեն": "hy"
  },
  {
    "italiano": "it"
  },
  {
    "日本語": "ja"
  },
  {
    "한국어": "ko"
  },
  {
    "kurdî": "ku-latn"
  },
  {
    "Lëtzebuergesch": "lb"
  },
  {
    "македонски": "mk"
  },
  {
    "ߒߞߏ": "nqo"
  },
  {
    "Polski": "pl"
  },
  {
    "русский": "ru"
  },
  {
    "Sardu": "sc"
  },
  {
    "slovenčina": "sk"
  },
  {
    "slovenščina": "sl"
  },
  {
    "Svenska": "sv"
  },
  {
    "Türkçe": "tr"
  },
  {
    "英语": "zh-hans"
  },
  {
    "繁體中文": "zh-hant"
  }
])EXPECTEDRESPONSE");
}

TEST_F(ServerTest, UserLanguageControl)
{
  struct TestData
  {
    const std::string description;
    const std::string url;
    const std::string acceptLanguageHeader;
    const char* const requestCookie; // Cookie: header of the request
    const char* const responseSetCookie; // Set-Cookie: header of the response
    const std::string expectedH1;

    operator TestContext() const
    {
      TestContext ctx{
          {"description", description},
          {"url", url},
          {"acceptLanguageHeader", acceptLanguageHeader},
      };

      if ( requestCookie ) {
        ctx.push_back({"requestCookie", requestCookie});
      }

      return ctx;
    }
  };

  const char* const NO_COOKIE = nullptr;

  const TestData testData[] = {
    {
      "Default user language is English",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "",
      /*Request Cookie:*/       NO_COOKIE,
      /*Response Set-Cookie:*/  "userlang=en;Path=/ROOT%23%3F;Max-Age=31536000",
      /* expected <h1> */ "Not Found"
    },
    {
      "userlang URL query parameter is respected",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article?userlang=en",
      /*Accept-Language:*/ "",
      /*Request Cookie:*/       NO_COOKIE,
      /*Response Set-Cookie:*/  "userlang=en;Path=/ROOT%23%3F;Max-Age=31536000",
      /* expected <h1> */ "Not Found"
    },
    {
      "userlang URL query parameter is respected",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article?userlang=test",
      /*Accept-Language:*/ "",
      /*Request Cookie:*/       NO_COOKIE,
      /*Response Set-Cookie:*/  "userlang=test;Path=/ROOT%23%3F;Max-Age=31536000",
      /* expected <h1> */ "[I18N TESTING] Content not found, but at least the server is alive"
    },
    {
      "'Accept-Language: *' is handled",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "*",
      /*Request Cookie:*/       NO_COOKIE,
      /*Response Set-Cookie:*/  "userlang=en;Path=/ROOT%23%3F;Max-Age=31536000",
      /* expected <h1> */ "Not Found"
    },
    {
      "Accept-Language: header is respected",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "test",
      /*Request Cookie:*/       NO_COOKIE,
      /*Response Set-Cookie:*/  "userlang=test;Path=/ROOT%23%3F;Max-Age=31536000",
      /* expected <h1> */ "[I18N TESTING] Content not found, but at least the server is alive"
    },
    {
      "userlang cookie is respected",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "",
      /*Request Cookie:*/       "userlang=test",
      /*Response Set-Cookie:*/  NO_COOKIE,
      /* expected <h1> */ "[I18N TESTING] Content not found, but at least the server is alive"
    },
    {
      "userlang cookie is correctly parsed",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "",
      /*Request Cookie:*/       "anothercookie=123; userlang=test",
      /*Response Set-Cookie:*/  NO_COOKIE,
      /* expected <h1> */ "[I18N TESTING] Content not found, but at least the server is alive"
    },
    {
      "userlang cookie is correctly parsed",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "",
      /*Request Cookie:*/       "userlang=test; anothercookie=abc",
      /*Response Set-Cookie:*/  NO_COOKIE,
      /* expected <h1> */ "[I18N TESTING] Content not found, but at least the server is alive"
    },
    {
      "userlang cookie is correctly parsed",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "",
      /*Request Cookie:*/       "cookie1=abc; userlang=test; cookie2=xyz",
      /*Response Set-Cookie:*/  NO_COOKIE,
      /* expected <h1> */ "[I18N TESTING] Content not found, but at least the server is alive"
    },
    {
      "Multiple userlang cookies are not a problem",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "",
      /*Request Cookie:*/       "cookie1=abc; userlang=en; userlang=test; cookie2=xyz",
      /*Response Set-Cookie:*/  NO_COOKIE,
      /* expected <h1> */ "[I18N TESTING] Content not found, but at least the server is alive"
    },
    {
      "userlang query parameter takes precedence over Accept-Language",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article?userlang=en",
      /*Accept-Language:*/ "test",
      /*Request Cookie:*/       NO_COOKIE,
      /*Response Set-Cookie:*/  "userlang=en;Path=/ROOT%23%3F;Max-Age=31536000",
      /* expected <h1> */ "Not Found"
    },
    {
      "userlang query parameter takes precedence over its cookie counterpart",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article?userlang=en",
      /*Accept-Language:*/ "",
      /*Request Cookie:*/       "userlang=test",
      /*Response Set-Cookie:*/  "userlang=en;Path=/ROOT%23%3F;Max-Age=31536000",
      /* expected <h1> */ "Not Found"
    },
    {
      "userlang in cookies takes precedence over Accept-Language",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "test",
      /*Request Cookie:*/       "userlang=en",
      /*Response Set-Cookie:*/  NO_COOKIE,
      /* expected <h1> */ "Not Found"
    },
    {
      "Most suitable language is selected from the Accept-Language header",
      // In case of a comma separated list of languages (optionally weighted
      // with quality values) the most suitable language is selected.
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "test;q=0.9, en;q=0.2",
      /*Request Cookie:*/       NO_COOKIE,
      /*Response Set-Cookie:*/  "userlang=test;Path=/ROOT%23%3F;Max-Age=31536000",
      /* expected <h1> */ "[I18N TESTING] Content not found, but at least the server is alive"
    },
    {
      "Most suitable language is selected from the Accept-Language header",
      // In case of a comma separated list of languages (optionally weighted
      // with quality values) the most suitable language is selected.
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "test;q=0.2, en;q=0.9",
      /*Request Cookie:*/       NO_COOKIE,
      /*Response Set-Cookie:*/  "userlang=en;Path=/ROOT%23%3F;Max-Age=31536000",
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
    if ( t.requestCookie ) {
      headers.insert({"Cookie", t.requestCookie});
    }
    const auto r = zfs1_->GET(t.url.c_str(), headers);
    if ( t.responseSetCookie ) {
      ASSERT_TRUE(r->has_header("Set-Cookie")) << t;
      EXPECT_EQ(t.responseSetCookie, getHeaderValue(r->headers, "Set-Cookie")) << t;
    } else {
      EXPECT_FALSE(r->has_header("Set-Cookie"));
    }
    std::regex_search(r->body, h1Match, h1Regex);
    const std::string h1(h1Match[1]);
    EXPECT_EQ(h1, t.expectedH1) << t;
  }
}

TEST_F(ServerTest, SlashlessRootURLIsRedirectedToSlashfulURL)
{
  const std::pair<const char*, const char*> test_data[] = {
    // URL                            redirect
    { "/ROOT%23%3F",                  "/ROOT%23%3F/" },
    { "/ROOT%23%3F?abcd=123&xyz=890", "/ROOT%23%3F/?abcd=123&xyz=890" }
  };

  for ( const auto& t : test_data )
  {
    const TestContext ctx{ {"url", t.first} };
    const auto g = zfs1_->GET(t.first);
    ASSERT_EQ(302, g->status) << ctx;
    ASSERT_TRUE(g->has_header("Location")) << ctx;
    ASSERT_EQ(g->get_header_value("Location"), t.second) << ctx;
    ASSERT_EQ(getCacheControlHeader(*g), "max-age=0, must-revalidate") << ctx;
    ASSERT_FALSE(g->has_header("ETag")) << ctx;
  }
}

TEST_F(ServerTest, EmptyRootIsNotRedirected)
{
  ZimFileServer::Cfg serverCfg;
  serverCfg.root = "";

  resetServer(serverCfg);

  ASSERT_EQ(200, zfs1_->GET("/")->status);
}

TEST_F(ServerTest, RandomPageRedirectsToAnExistingArticle)
{
  auto g = zfs1_->GET("/ROOT%23%3F/random?content=zimfile");
  ASSERT_EQ(302, g->status);
  ASSERT_TRUE(g->has_header("Location"));
  ASSERT_TRUE(kiwix::startsWith(g->get_header_value("Location"), "/ROOT%23%3F/content/zimfile/A/"));
  ASSERT_EQ(getCacheControlHeader(*g), "max-age=0, must-revalidate");
  ASSERT_FALSE(g->has_header("ETag"));
}

TEST_F(ServerTest, RandomPageRedirectionsAreUriEncoded)
{
  // The purpose of this unit test is to validate that the URL to which the
  // /random endpoint redirects is in URI-encoded form. Due to the difficulty
  // of checking non-deterministic functionality, it is implemented by running
  // /random sufficiently many times so that all articles are returned with
  // close to 100% probability (this is done on an artificial small ZIM file
  // with at least one article with a special symbol in its name). Such an
  // approach effectively tests more than the original goal was, so this test
  // may over time evolve into something more general whereupon it must be
  // renamed.

  auto g = zfs1_->GET("/ROOT%23%3F/random?content=corner_cases%23%26");

  typedef std::set<std::string> StringSet;
  const StringSet frontArticles{
      {"/ROOT%23%3F/content/corner_cases%23%26/c%23.html"},

      // empty.html is missing because of a subtle bug
      // in libzim::randomNumber(max) which returns a value equal to
      // it argument with much lower probability than other numbers
      // {"/ROOT%23%3F/content/corner_cases%23%26/empty.html"}
  };

  StringSet randomPageUrls;
  auto n = 10 * frontArticles.size();
  while ( --n && randomPageUrls.size() != frontArticles.size() ) {
    ASSERT_EQ(302, g->status);
    ASSERT_TRUE(g->has_header("Location"));
    randomPageUrls.insert(g->get_header_value("Location"));
  }

  ASSERT_EQ(frontArticles, randomPageUrls);
}

TEST_F(ServerTest, NonEndpointUrlsAreRedirectedToContentUrls)
{
  const std::string paths[] = {
    "/zimfile",
    "/zimfile/A/index",
    "/some/path",
    "/non-existent-item",

    // Validate that paths starting with a string matching an endpoint name
    // are not misinterpreted as endpoint URLs
    "/catalogarithm",
    "/catalogistics/root.xml",
    "/catch22",
    "/catchy/song",
    "/contention",
    "/contents/is/not/the/same/as/content",
    "/randomize",
    "/randomestic/animals",
    "/rawman",
    "/rawkward/url",
    "/searcheology",
    "/searchitecture/history",
    "/skinhead",
    "/skineffect/formula",
    "/suggesture",
    "/suggestonia/tallinn",

    // The /meta endpoint has been replaced with /raw/<book>/meta
    "/meta",

    // Make sure that the query is preserved in the redirect URL
    "/does?P=NP"

    // Make sure that reserved URI encoded symbols stay URI encoded
    "/C%23",              // # --> /C#
    "/100%25_guarantee",  // % --> /100%_guarantee
    "/AT%26T",            // & --> /AT&T
    "/Quo_vadis%3F",      // ? --> /Quo_vadis?
    "/1%2B1%3D2",         // +,= --> 1+1=2

    // Make sure that URI-encoded query stays URI-encoded
    "/encode?string=%23%25%26%2B%3D%3F",

    // There seems to be a bug in httplib client - the '+' symbols
    // in query parameters are URI encoded. Therefore using %20 (a URI-encoded
    // space) instead.
    //"/route?from=current+location&to=girlfriend%238",
    "/route?from=current%20location&to=girlfriend%238",
  };

  for ( const std::string& p : paths )
  {
    auto g = zfs1_->GET(("/ROOT%23%3F" + p).c_str());
    const TestContext ctx{ { "path", p } };
    ASSERT_EQ(302, g->status) << ctx;
    ASSERT_TRUE(g->has_header("Location")) << ctx;
    ASSERT_EQ("/ROOT%23%3F/content" + p, g->get_header_value("Location")) << ctx;
    ASSERT_EQ(getCacheControlHeader(*g), "max-age=0, must-revalidate");
    ASSERT_FALSE(g->has_header("ETag"));
  }
}

TEST_F(ServerTest, RedirectionsToURLsWithSpecialSymbols)
{
  auto g = zfs1_->GET("/ROOT%23%3F/content/corner_cases%23%26/c_sharp.html");
  ASSERT_EQ(302, g->status);
  ASSERT_TRUE(g->has_header("Location"));
  ASSERT_EQ(g->get_header_value("Location"), "/ROOT%23%3F/content/corner_cases%23%26/c%23.html");
  ASSERT_EQ(getCacheControlHeader(*g), "max-age=0, must-revalidate");
  ASSERT_FALSE(g->has_header("ETag"));
}


TEST_F(ServerTest, BookMainPageIsRedirectedToArticleIndex)
{
  {
  auto g = zfs1_->GET("/ROOT%23%3F/content/zimfile");
  ASSERT_EQ(302, g->status);
  ASSERT_TRUE(g->has_header("Location"));
  ASSERT_EQ("/ROOT%23%3F/content/zimfile/A/index", g->get_header_value("Location"));
  }
}


TEST_F(ServerTest, RawEntry)
{
  auto p = zfs1_->GET("/ROOT%23%3F/raw/zimfile/meta/Title");
  EXPECT_EQ(200, p->status);
  EXPECT_EQ(p->body, std::string("Ray Charles"));

  p = zfs1_->GET("/ROOT%23%3F/raw/zimfile/meta/Creator");
  EXPECT_EQ(200, p->status);
  EXPECT_EQ(p->body, std::string("Wikipedia"));

  // The raw content of Ray_Charles returned by the server is
  // the same as the one in the zim file.
  auto archive = zim::Archive("./test/zimfile.zim");
  auto entry = archive.getEntryByPath("A/Ray_Charles");
  p = zfs1_->GET("/ROOT%23%3F/raw/zimfile/content/A/Ray_Charles");
  EXPECT_EQ(200, p->status);
  EXPECT_EQ(std::string(p->body), std::string(entry.getItem(true).getData()));

  /* Now normal content is not decorated in any way, either
  // ... but the "normal" content is not
  p = zfs1_->GET("/ROOT%23%3F/content/zimfile/A/Ray_Charles");
  EXPECT_EQ(200, p->status);
  EXPECT_NE(std::string(p->body), std::string(entry.getItem(true).getData()));
  EXPECT_TRUE(p->body.find("<link type=\"root\" href=\"/ROOT%23%3F\">") != std::string::npos);
  */
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

TEST_F(ServerTest, CacheControlOfZimContent)
{
  for ( const Resource& res : all200Resources() ) {
    if ( res.kind == ZIM_CONTENT ) {
      const auto g = zfs1_->GET(res.url);
      EXPECT_EQ(getCacheControlHeader(*g), "max-age=3600, must-revalidate") << res;
      EXPECT_TRUE(g->has_header("ETag")) << res;
    }
  }
}

TEST_F(ServerTest, CacheControlOfStaticContent)
{
  for ( const Resource& res : all200Resources() ) {
    if ( res.kind == STATIC_CONTENT ) {
      const auto g = zfs1_->GET(res.url);
      EXPECT_EQ(getCacheControlHeader(*g), "max-age=31536000, immutable") << res;
      EXPECT_FALSE(g->has_header("ETag")) << res;
    }
  }
}

TEST_F(ServerTest, CacheControlOfDynamicContent)
{
  for ( const Resource& res : all200Resources() ) {
    if ( res.kind == DYNAMIC_CONTENT ) {
      const auto g = zfs1_->GET(res.url);
      EXPECT_EQ(getCacheControlHeader(*g), "max-age=0, must-revalidate") << res;
      EXPECT_TRUE(g->has_header("ETag")) << res;
    }
  }
}

TEST_F(ServerTest, ETagHeaderIsSetAsNeeded)
{
  for ( const Resource& res : all200Resources() ) {
    const auto responseToGet = zfs1_->GET(res.url);
    EXPECT_EQ(res.etag_expected(), responseToGet->has_header("ETag")) << res;
    if ( res.etag_expected() ) {
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

TEST_F(ServerTest, DifferentServerInstancesProduceDifferentETagsForDynamicContent)
{
  ZimFileServer zfs2(SERVER_PORT + 1, ZimFileServer::DEFAULT_OPTIONS, ZIMFILES);
  for ( const Resource& res : all200Resources() ) {
    if ( res.kind != DYNAMIC_CONTENT ) continue;
    const auto h1 = zfs1_->HEAD(res.url);
    const auto h2 = zfs2.HEAD(res.url);
    EXPECT_NE(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
  }
}

TEST_F(ServerTest, DifferentServerInstancesProduceIdenticalETagsForZimContent)
{
  ZimFileServer zfs2(SERVER_PORT + 1, ZimFileServer::DEFAULT_OPTIONS, ZIMFILES);
  for ( const Resource& res : all200Resources() ) {
    if ( res.kind != ZIM_CONTENT ) continue;
    const auto h1 = zfs1_->HEAD(res.url);
    const auto h2 = zfs2.HEAD(res.url);
    EXPECT_EQ(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
  }
}

TEST_F(ServerTest, CompressionInfluencesETag)
{
  for ( const Resource& res : resources200Compressible ) {
    if ( ! res.etag_expected() ) continue;
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
    if ( ! res.etag_expected() ) continue;
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
      if ( ! res.etag_expected() ) continue;
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
    EXPECT_EQ(200, h->status) << res;
    EXPECT_EQ(200, g2->status) << res;
  }
}

TEST_F(ServerTest, ValidSingleRangeByteRangeRequestsAreHandledProperly)
{
  const char url[] = "/ROOT%23%3F/content/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";
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
  const char url[] = "/ROOT%23%3F/content/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";

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
  const char url[] = "/ROOT%23%3F/content/corner_cases%23%26/empty.js";

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
  const char url[] = "/ROOT%23%3F/content/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";

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
  const char url[] = "/ROOT%23%3F/content/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";
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
    { /* url: */ "/ROOT%23%3F/suggest?content=zimfile&term=thing",
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
    { /* url: */ "/ROOT%23%3F/suggest?content=zimfile&term=old%20sun",
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
    { /* url: */ "/ROOT%23%3F/suggest?content=zimfile&term=öld%20suñ",
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
    { /* url: */ "/ROOT%23%3F/suggest?content=zimfile&term=abracadabra",
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
      /* url: */ "/ROOT%23%3F/suggest?content=zimfile&term=A%26B",
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
    { /* url: */ "/ROOT%23%3F/suggest?content=zimfile&term=abracadabra&userlang=test",
R"EXPECTEDRESPONSE([
  {
    "value" : "abracadabra ",
    "label" : "[I18N TESTING] cOnTaInInG &apos;abracadabra&apos;...",
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
      std::string url = "/ROOT%23%3F/suggest?content=zimfile&term=ray&start=" + std::to_string(i*5) + "&count=5";
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
    std::string url = "/ROOT%23%3F/suggest?content=zimfile&term=song+for+you&start=0&count=5";
    const auto r1 = zfs1_->GET(url.c_str());
    std::string body = r1->body;
    int currCount = std::count(body.begin(), body.end(), '{') - 1;
    ASSERT_EQ(currCount, 5);

    url = "/ROOT%23%3F/suggest?content=zimfile&term=song+for+you&start=5&count=5";
    const auto r2 = zfs1_->GET(url.c_str());
    body = r2->body;
    currCount = std::count(body.begin(), body.end(), '{') - 1;
    ASSERT_EQ(currCount, 3);
  }

  // Attempt to get 10 suggestions even though there is only 1
  {
    std::string url = "/ROOT%23%3F/suggest?content=zimfile&term=strong&start=0&count=5";
    const auto r = zfs1_->GET(url.c_str());
    std::string body = r->body;
    int currCount = std::count(body.begin(), body.end(), '{') - 1;
    ASSERT_EQ(currCount, 1);
  }

  // No Suggestion
  {
    std::string url = "/ROOT%23%3F/suggest?content=zimfile&term=oops&start=0&count=5";
    const auto r = zfs1_->GET(url.c_str());
    std::string body = r->body;
    int currCount = std::count(body.begin(), body.end(), '{') - 1;
    ASSERT_EQ(currCount, 0);
  }

  // Out of bound value
  {
    std::string url = "/ROOT%23%3F/suggest?content=zimfile&term=ray&start=-2&count=-1";
    const auto r = zfs1_->GET(url.c_str());
    std::string body = r->body;
    int currCount = std::count(body.begin(), body.end(), '{') - 1;
    ASSERT_EQ(currCount, 0);
  }
}

TEST_F(ServerTest, viewerSettings)
{
  const auto JS_CONTENT_TYPE = "application/javascript; charset=utf-8";
  {
    resetServer(ZimFileServer::NO_TASKBAR_NO_LINK_BLOCKING);
    const auto r = zfs1_->GET("/ROOT%23%3F/viewer_settings.js");
    ASSERT_EQ(r->status, 200);
    ASSERT_EQ(getHeaderValue(r->headers, "Content-Type"), JS_CONTENT_TYPE);
    ASSERT_EQ(r->body,
R"(const viewerSettings = {
  toolbarEnabled:       false,
  linkBlockingEnabled:  false,
  libraryButtonEnabled: false
}
)");
  }

  {
    resetServer(ZimFileServer::BLOCK_EXTERNAL_LINKS);
    ASSERT_EQ(zfs1_->GET("/ROOT%23%3F/viewer_settings.js")->body,
R"(const viewerSettings = {
  toolbarEnabled:       false,
  linkBlockingEnabled:  true,
  libraryButtonEnabled: false
}
)");
  }

  {
    resetServer(ZimFileServer::WITH_TASKBAR);
    ASSERT_EQ(zfs1_->GET("/ROOT%23%3F/viewer_settings.js")->body,
R"(const viewerSettings = {
  toolbarEnabled:       true,
  linkBlockingEnabled:  false,
  libraryButtonEnabled: false
}
)");
  }

  {
    resetServer(ZimFileServer::WITH_TASKBAR_AND_LIBRARY_BUTTON);
    ASSERT_EQ(zfs1_->GET("/ROOT%23%3F/viewer_settings.js")->body,
R"(const viewerSettings = {
  toolbarEnabled:       true,
  linkBlockingEnabled:  false,
  libraryButtonEnabled: true
}
)");
  }
}
