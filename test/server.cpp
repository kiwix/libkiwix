
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

  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/autoComplete/autoComplete.min.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/autoComplete/autoComplete.min.js?cacheid=1191aaaf" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/autoComplete/css/autoComplete.css" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/autoComplete/css/autoComplete.css?cacheid=f2d376c4" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/error.css" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/error.css?cacheid=b3fa90cf" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/i18n.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/i18n.js?cacheid=e9a10ac1" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/index.css" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/index.css?cacheid=ae79e41a" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/index.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/index.js?cacheid=4e232c58" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/iso6391To3.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/iso6391To3.js?cacheid=ecde2bb3" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/isotope.pkgd.min.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/isotope.pkgd.min.js?cacheid=2e48d392" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/kiwix.css" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/kiwix.css?cacheid=b4e29e64" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/mustache.min.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/mustache.min.js?cacheid=bd23c4fb" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/taskbar.css" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/taskbar.css?cacheid=42e90cb9" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/viewer.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/viewer.js?cacheid=3208c3ed" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/fonts/Poppins.ttf" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/fonts/Poppins.ttf?cacheid=af705837" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/fonts/Roboto.ttf" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/fonts/Roboto.ttf?cacheid=84d10248" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/i18n/test.json" },
  // TODO: implement cache management of i18n resources
  //{ STATIC_CONTENT, "/ROOT%23%3F/skin/i18n/test.json?cacheid=unknown" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/languages.js" },
  { STATIC_CONTENT, "/ROOT%23%3F/skin/languages.js?cacheid=08955948" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/search" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/v2/root.xml" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/v2/entries" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/catalog/v2/partial_entries" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/catch/external?source=www.example.com" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/search?content=zimfile&pattern=a" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/suggest?content=zimfile&term=ray" },

  { ZIM_CONTENT,     "/ROOT%23%3F/content/zimfile/A/index" },
  { ZIM_CONTENT,     "/ROOT%23%3F/content/zimfile/A/Ray_Charles" },

  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/content/A/index" },
  { ZIM_CONTENT,     "/ROOT%23%3F/raw/zimfile/content/A/Ray_Charles" },

  { DYNAMIC_CONTENT, "/ROOT%23%3F/nojs"},
};

const ResourceCollection resources200Uncompressible{
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/404.svg" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/404.svg?cacheid=b6d648af" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/500.svg" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/500.svg?cacheid=32eb0f20" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/bittorrent.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/bittorrent.png?cacheid=4f5c6882" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/blank.html" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/blank.html?cacheid=6b1fa032" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/blocklink.svg" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/blocklink.svg?cacheid=bd56b116" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/caret.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/caret.png?cacheid=22b942b4" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/download.png" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/download.png?cacheid=a39aa502" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/download-white.svg" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/download-white.svg?cacheid=079ab989"},
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
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/polyfills.js" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/polyfills.js?cacheid=a0e0343d" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/search-icon.svg" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/search-icon.svg?cacheid=b10ae7ed" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/search_results.css" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/search_results.css?cacheid=76d39c84" },
  { DYNAMIC_CONTENT, "/ROOT%23%3F/skin/print.css" },
  { STATIC_CONTENT,  "/ROOT%23%3F/skin/print.css?cacheid=65b1c1d2" },

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

  { DYNAMIC_CONTENT, "/ROOT%23%3F/nojs/download/zimfile"},
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
R"EXPECTEDRESULT(      href="/ROOT%23%3F/skin/kiwix.css?cacheid=b4e29e64"
      href="/ROOT%23%3F/skin/index.css?cacheid=ae79e41a"
    <link rel="apple-touch-icon" sizes="180x180" href="/ROOT%23%3F/skin/favicon/apple-touch-icon.png?cacheid=f86f8df3">
    <link rel="icon" type="image/png" sizes="32x32" href="/ROOT%23%3F/skin/favicon/favicon-32x32.png?cacheid=79ded625">
    <link rel="icon" type="image/png" sizes="16x16" href="/ROOT%23%3F/skin/favicon/favicon-16x16.png?cacheid=a986fedc">
    <link rel="manifest" href="/ROOT%23%3F/skin/favicon/site.webmanifest?cacheid=bc396efb">
    <link rel="mask-icon" href="/ROOT%23%3F/skin/favicon/safari-pinned-tab.svg?cacheid=8d487e95" color="#5bbad5">
    <link rel="shortcut icon" href="/ROOT%23%3F/skin/favicon/favicon.ico?cacheid=92663314">
    <meta name="msapplication-config" content="/ROOT%23%3F/skin/favicon/browserconfig.xml?cacheid=f29a7c4a">
    <script type="text/javascript" src="./skin/polyfills.js?cacheid=a0e0343d"></script>
    <script type="module" src="/ROOT%23%3F/skin/i18n.js?cacheid=e9a10ac1" defer></script>
    <script type="text/javascript" src="/ROOT%23%3F/skin/languages.js?cacheid=08955948" defer></script>
    <script src="/ROOT%23%3F/skin/isotope.pkgd.min.js?cacheid=2e48d392" defer></script>
    <script src="/ROOT%23%3F/skin/iso6391To3.js?cacheid=ecde2bb3"></script>
    <script type="text/javascript" src="/ROOT%23%3F/skin/index.js?cacheid=4e232c58" defer></script>
        <img src="/ROOT%23%3F/skin/feed.svg?cacheid=055b333f"
        <img src="/ROOT%23%3F/skin/langSelector.svg?cacheid=00b59961"
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT%23%3F/skin/kiwix.css",
R"EXPECTEDRESULT(  src: url("../skin/fonts/Poppins.ttf?cacheid=af705837") format("truetype");
  src: url("../skin/fonts/Roboto.ttf?cacheid=84d10248") format("truetype");
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT%23%3F/skin/index.css",
R"EXPECTEDRESULT(    background-image: url('../skin/search-icon.svg?cacheid=b10ae7ed');
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT%23%3F/skin/index.js",
R"EXPECTEDRESULT(                  <img src="${root}/skin/download-white.svg?cacheid=079ab989">
                                <img src="${root}/skin/download.png?cacheid=a39aa502" alt="${$t("direct-download-alt-text")}" />
                                <img src="${root}/skin/hash.png?cacheid=f836e872" alt="${$t("hash-download-alt-text")}" />
                                <img src="${root}/skin/magnet.png?cacheid=73b6bddf" alt="${$t("magnet-alt-text")}" />
                                <img src="${root}/skin/bittorrent.png?cacheid=4f5c6882" alt="${$t("torrent-download-alt-text")}" />
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT%23%3F/viewer",
R"EXPECTEDRESULT(    <link type="text/css" href="./skin/kiwix.css?cacheid=b4e29e64" rel="Stylesheet" />
    <link type="text/css" href="./skin/taskbar.css?cacheid=42e90cb9" rel="Stylesheet" />
    <link type="text/css" href="./skin/autoComplete/css/autoComplete.css?cacheid=f2d376c4" rel="Stylesheet" />
    <link type="text/css" href="./skin/print.css?cacheid=65b1c1d2" media="print" rel="Stylesheet" />
    <script type="text/javascript" src="./skin/polyfills.js?cacheid=a0e0343d"></script>
    <script type="module" src="./skin/i18n.js?cacheid=e9a10ac1" defer></script>
    <script type="text/javascript" src="./skin/languages.js?cacheid=08955948" defer></script>
    <script type="text/javascript" src="./skin/viewer.js?cacheid=3208c3ed" defer></script>
    <script type="text/javascript" src="./skin/autoComplete/autoComplete.min.js?cacheid=1191aaaf"></script>
      const blankPageUrl = root + "/skin/blank.html?cacheid=6b1fa032";
          <label for="kiwix_button_show_toggle"><img src="./skin/caret.png?cacheid=22b942b4" alt=""></label>
               src="./skin/langSelector.svg?cacheid=00b59961">
            src="./skin/blank.html?cacheid=6b1fa032" title="ZIM content" width="100%"
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT%23%3F/content/zimfile/A/index",
      ""
    },
    {
      /* url */ "/ROOT%23%3F/content/invalid-book/whatever",
R"EXPECTEDRESULT(    <link type="text/css" href="/ROOT%23%3F/skin/error.css?cacheid=b3fa90cf" rel="Stylesheet" />
      window.KIWIX_RESPONSE_TEMPLATE = "&lt;!DOCTYPE html&gt;\n&lt;html&gt;\n  &lt;head&gt;\n    &lt;meta charset=&quot;utf-8&quot;&gt;\n    &lt;meta name=&quot;viewport&quot; content=&quot;width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no&quot; /&gt;\n    &lt;title&gt;{{PAGE_TITLE}}&lt;/title&gt;\n    &lt;link type=&quot;text/css&quot; href=&quot;{{root}}/skin/error.css?cacheid=b3fa90cf&quot; rel=&quot;Stylesheet&quot; /&gt;\n{{#KIWIX_RESPONSE_DATA}}    &lt;script&gt;\n      window.KIWIX_RESPONSE_TEMPLATE = &quot;{{KIWIX_RESPONSE_TEMPLATE}}&quot;;\n      window.KIWIX_RESPONSE_DATA = {{{KIWIX_RESPONSE_DATA}}};\n    &lt;/script&gt;{{/KIWIX_RESPONSE_DATA}}\n  &lt;/head&gt;\n  &lt;body&gt;\n    &lt;header&gt;\n        &lt;img src=&quot;{{root}}/skin/404.svg?cacheid=b6d648af&quot;\n             alt=&quot;{{404_img_text}}&quot;\n             aria-label=&quot;{{404_img_text}}&quot;\n             title=&quot;{{404_img_text}}&quot;&gt;\n    &lt;/header&gt;\n    &lt;section class=&quot;intro&quot;&gt;\n      &lt;h1&gt;{{PAGE_HEADING}}&lt;/h1&gt;\n      &lt;p&gt;{{path_was_not_found_msg}}&lt;/p&gt;\n      &lt;p&gt;&lt;code&gt;{{url_path}}&lt;/code&gt;&lt;/p&gt;\n    &lt;/section&gt;\n    &lt;section class=&quot;advice&quot;&gt;\n      &lt;p&gt;{{advice.p1}}&lt;/p&gt;\n      &lt;p class=&quot;list-intro&quot;&gt;{{advice.p2}}&lt;/p&gt;\n      &lt;ul&gt;\n          &lt;li&gt;{{advice.p3}}&lt;/li&gt;\n          &lt;li&gt;{{advice.p4}}&lt;/li&gt;\n      &lt;/ul&gt;\n      &lt;p&gt;{{advice.p5}}&lt;/p&gt;\n    &lt;/section&gt;\n  &lt;/body&gt;\n&lt;/html&gt;\n";
        <img src="/ROOT%23%3F/skin/404.svg?cacheid=b6d648af"
)EXPECTEDRESULT"
    },
    {
      /* url */ "/ROOT%23%3F/catch/external?source=https%3A%2F%2Fkiwix.org",
R"EXPECTEDRESULT(    <link type="text/css" href="/ROOT%23%3F/skin/error.css?cacheid=b3fa90cf" rel="Stylesheet" />
    <script type="module" src="/ROOT%23%3F/skin/i18n.js?cacheid=e9a10ac1"></script>
      window.KIWIX_RESPONSE_TEMPLATE = "&lt;!DOCTYPE html&gt;\n&lt;html&gt;\n  &lt;head&gt;\n    &lt;meta charset=&quot;utf-8&quot;&gt;\n    &lt;meta name=&quot;viewport&quot; content=&quot;width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no&quot; /&gt;\n    &lt;title&gt;{{external_link_detected}}&lt;/title&gt;\n    &lt;link type=&quot;text/css&quot; href=&quot;{{root}}/skin/error.css?cacheid=b3fa90cf&quot; rel=&quot;Stylesheet&quot; /&gt;\n    &lt;script type=&quot;module&quot; src=&quot;{{root}}/skin/i18n.js?cacheid=e9a10ac1&quot;&gt;&lt;/script&gt;\n    &lt;script&gt;\n      window.KIWIX_RESPONSE_TEMPLATE = &quot;{{KIWIX_RESPONSE_TEMPLATE}}&quot;;\n      window.KIWIX_RESPONSE_DATA = {{{KIWIX_RESPONSE_DATA}}};\n    &lt;/script&gt;\n  &lt;/head&gt;\n  &lt;body&gt;\n    &lt;header&gt;\n        &lt;img src=&quot;{{root}}/skin/blocklink.svg?cacheid=bd56b116&quot;\n             alt=&quot;{{caution_warning}}&quot;\n             aria-label=&quot;{{caution_warning}}&quot;\n             title=&quot;{{caution_warning}}&quot;&gt;\n    &lt;/header&gt;\n    &lt;section class=&quot;intro&quot;&gt;\n      &lt;h1&gt;{{external_link_detected}}&lt;/h1&gt;\n      &lt;p&gt;{{external_link_intro}}&lt;/p&gt;\n      &lt;p&gt;&lt;a href=&quot;{{url}}&quot;&gt;{{ url }}&lt;/a&gt;&lt;/p&gt;\n    &lt;/section&gt;\n    &lt;section class=&quot;advice&quot;&gt;\n      &lt;p&gt;{{advice.p1}}&lt;/p&gt;\n      &lt;p&gt;{{advice.p2}}&lt;/p&gt;\n      &lt;p&gt;{{advice.p3}}&lt;/p&gt;\n    &lt;/section&gt;\n  &lt;/body&gt;\n&lt;/html&gt;\n";
        <img src="/ROOT%23%3F/skin/blocklink.svg?cacheid=bd56b116"
)EXPECTEDRESULT"
    },
    {
      // Searching in a ZIM file without a full-text index returns
      // a page rendered from static/templates/no_search_result_html
      /* url */ "/ROOT%23%3F/search?content=poor&pattern=whatever",
R"EXPECTEDRESULT(    <link type="text/css" href="/ROOT%23%3F/skin/search_results.css?cacheid=76d39c84" rel="Stylesheet" />
      window.KIWIX_RESPONSE_DATA = { "CSS_URL" : "/ROOT%23%3F/skin/search_results.css?cacheid=76d39c84", "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "fulltext-search-unavailable", "params" : { } }, "details" : [ { "p" : { "msgid" : "no-search-results", "params" : { } } } ] };
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

std::string getCacheIdFromUrl(const std::string& url)
{
  const std::string q("?cacheid=");
  const auto i = url.find(q);
  return i == std::string::npos ? "" : url.substr(i + q.size());
}

std::string runExternalCmdAndGetItsOutput(const std::string& cmd)
{
  std::string cmdOutput;

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

  if (FILE* pPipe = popen(cmd.c_str(), "r"))
  {
    char buf[128];
    while (fgets(buf, 128, pPipe)) {
        cmdOutput += std::string(buf, buf+128);
    }

    pclose(pPipe);
  }

  return cmdOutput;
}

std::string getSha1OfResponseData(const std::string& url)
{
  const std::string pythonScript =
    "import urllib.request as req; "
    "import hashlib; "
    "print(hashlib.sha1(req.urlopen('" + url + "').read()).hexdigest())";
  const std::string cmd = "python3 -c \"" + pythonScript + "\"";
  return runExternalCmdAndGetItsOutput(cmd);
}

TEST_F(ServerTest, CacheIdsOfStaticResourcesMatchTheSha1HashOfResourceContent)
{
  for ( const Resource& res : all200Resources() ) {
    if ( res.kind == STATIC_CONTENT ) {
      const TestContext ctx{ {"url", res.url} };
      const std::string fullUrl = "http://localhost:" + std::to_string(SERVER_PORT) + res.url;
      const std::string sha1 = getSha1OfResponseData(fullUrl);
      ASSERT_EQ(sha1.substr(0, 8), getCacheIdFromUrl(res.url)) << ctx;
    }
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
  "/ROOT%23%3F/skin/autoComplete/autoComplete.min.js?cacheid=wrongcacheid",
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

  "/ROOT%23%3F/nojs/download/thiszimdoesntexist"
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
    { "/catalog/root.xml",                 "application/atom+xml;profile=opds-catalog;kind=acquisition;charset=utf-8" },
    { "/catalog/v2/searchdescription.xml", "application/opensearchdescription+xml" },
    { "/catalog/v2/root.xml",              "application/atom+xml;profile=opds-catalog;kind=navigation;charset=utf-8" },
    { "/catalog/v2/languages",             "application/atom+xml;profile=opds-catalog;kind=navigation;charset=utf-8" },
    { "/catalog/v2/categories",            "application/atom+xml;profile=opds-catalog;kind=navigation;charset=utf-8" },
    { "/catalog/v2/entries",               "application/atom+xml;profile=opds-catalog;kind=acquisition;charset=utf-8" },
    { "/catalog/v2/entry/6f1d19d0-633f-087b-fb55-7ac324ff9baf", "application/atom+xml;type=entry;profile=opds-catalog;charset=utf-8" },
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
  const std::string expectedKiwixResponseData;
  const std::string bookName;
  const std::string bookTitle;
  const std::string expectedBody;
};

enum ExpectedResponseDataType
{
  expected_page_title,
  expected_css_url,
  expected_kiwix_response_data,
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
    case expected_page_title: return ExpectedResponseData{s, "", "", "", "", ""};
    case expected_css_url:    return ExpectedResponseData{"", s, "", "", "", ""};
    case expected_kiwix_response_data:
                              return ExpectedResponseData{"", "", s, "", "", ""};
    case book_name:           return ExpectedResponseData{"", "", "", s, "", ""};
    case book_title:          return ExpectedResponseData{"", "", "", "", s, ""};
    case expected_body:       return ExpectedResponseData{"", "", "", "", "", s};
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
    selectNonEmpty(a.expectedKiwixResponseData, b.expectedKiwixResponseData),
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
    // frag[0]
    R"FRAG(<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta content="text/html;charset=UTF-8" http-equiv="content-type" />
    <title>)FRAG",

    // frag[1]
    R"FRAG(</title>
)FRAG",

    // frag[2]
    R"(    <script>
      window.KIWIX_RESPONSE_TEMPLATE = )" + ERROR_HTML_TEMPLATE_JS_STRING + R"(;
      window.KIWIX_RESPONSE_DATA = )",

    // frag[3]
    R"FRAG(;
    </script>
  </head>
  <body>)FRAG",

  // frag[4]
  R"FRAG(  </body>
</html>
)FRAG"
  };

  return frag[0]
       + pageTitle()
       + frag[1]
       + pageCssLink()
       + frag[2]
       + expectedKiwixResponseData
       + frag[3]
       + expectedBody
       + frag[4];
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
       + R"(" rel="Stylesheet" />)"
       + "\n";
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
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "no-such-book", "params" : { "BOOK_NAME" : "non-existent-book" } } } ] })" &&
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      No such book: non-existent-book
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/random?content=non-existent-book&userlang=test",
      expected_page_title=="[I18N TESTING] Not Found - Try Again" &&
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "no-such-book", "params" : { "BOOK_NAME" : "non-existent-book" } } } ] })" &&
      expected_body==R"(
    <h1>[I18N TESTING] Content not found, but at least the server is alive</h1>
    <p>
      [I18N TESTING] No such book: non-existent-book. Sorry.
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/suggest?content=no-such-book&term=whatever",
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "no-such-book", "params" : { "BOOK_NAME" : "no-such-book" } } } ] })" &&
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      No such book: no-such-book
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/catalog/",
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "url-not-found", "params" : { "url" : "/ROOT%23%3F/catalog/" } } } ] })" &&
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/catalog/" was not found on this server.
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/catalog/?userlang=test",
      expected_page_title=="[I18N TESTING] Not Found - Try Again" &&
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "url-not-found", "params" : { "url" : "/ROOT%23%3F/catalog/" } } } ] })" &&
      expected_body==R"(
    <h1>[I18N TESTING] Content not found, but at least the server is alive</h1>
    <p>
      [I18N TESTING] URL not found: /ROOT%23%3F/catalog/
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/catalog/invalid_endpoint",
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "url-not-found", "params" : { "url" : "/ROOT%23%3F/catalog/invalid_endpoint" } } } ] })" &&
      expected_body==R"(
    <h1>Not Found</h1>
    <p>
      The requested URL "/ROOT%23%3F/catalog/invalid_endpoint" was not found on this server.
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/catalog/invalid_endpoint?userlang=test",
      expected_page_title=="[I18N TESTING] Not Found - Try Again" &&
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "url-not-found", "params" : { "url" : "/ROOT%23%3F/catalog/invalid_endpoint" } } } ] })" &&
      expected_body==R"(
    <h1>[I18N TESTING] Content not found, but at least the server is alive</h1>
    <p>
      [I18N TESTING] URL not found: /ROOT%23%3F/catalog/invalid_endpoint
    </p>
)"  },

    { /* url */ "/ROOT%23%3F/raw/no-such-book/meta/Title",
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "url-not-found", "params" : { "url" : "/ROOT%23%3F/raw/no-such-book/meta/Title" } } }, { "p" : { "msgid" : "no-such-book", "params" : { "BOOK_NAME" : "no-such-book" } } } ] })" &&
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
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "url-not-found", "params" : { "url" : "/ROOT%23%3F/raw/zimfile/XYZ" } } }, { "p" : { "msgid" : "invalid-raw-data-type", "params" : { "DATATYPE" : "XYZ" } } } ] })" &&
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
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "url-not-found", "params" : { "url" : "/ROOT%23%3F/raw/zimfile/meta/invalid-metadata" } } }, { "p" : { "msgid" : "raw-entry-not-found", "params" : { "DATATYPE" : "meta", "ENTRY" : "invalid-metadata" } } } ] })" &&
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
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "url-not-found", "params" : { "url" : "/ROOT%23%3F/raw/zimfile/content/invalid-article" } } }, { "p" : { "msgid" : "raw-entry-not-found", "params" : { "DATATYPE" : "content", "ENTRY" : "invalid-article" } } } ] })" &&
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
      expected_kiwix_response_data==R"({ "CSS_URL" : "/ROOT%23%3F/skin/search_results.css?cacheid=76d39c84", "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "fulltext-search-unavailable", "params" : { } }, "details" : [ { "p" : { "msgid" : "no-search-results", "params" : { } } } ] })" &&
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

std::string htmlEscape(std::string s)
{
  s = replace(s, "&", "&amp;");
  s = replace(s, "<", "&lt;");
  s = replace(s, ">", "&gt;");
  s = replace(s, "\"", "&quot;");
  return s;
}

std::string escapeJsString(std::string s)
{
  s = replace(s, "</script>", "</scr\\ipt>");
  s = replace(s, "\"", "\\\"");
  return s;
}

std::string expectedSexy404ErrorHtml(const std::string& url)
{
  const auto urlWithoutQuery = replace(url, "\\?.*$", "");
  const auto htmlSafeUrl = htmlEscape(urlWithoutQuery);
  const auto jsSafeUrl = escapeJsString(urlWithoutQuery);

  const std::string englishText[] = {
    "Page not found",
    "Not found!",
    "Oops. Page not found.",
    "The requested path was not found:",
    "The content you&apos;re looking for may still be available, but it might be located at a different place within the ZIM file.",
    "Please:",
    "Try using the search function to find the content you want",
    "Look for keywords or titles related to the information you&apos;re seeking",
    "This approach should help you locate the desired content, even if the original link isn&apos;t working properly."
  };

  const std::string translatedText[] = {
    "Page [I18N] not [TESTING] found",
    "[I18N] Not found! [TESTING]",
    "[I18N TESTING] Oops. Larry Page could not be reached. He may be on paternity leave.",
    "[I18N TESTING] The requested path was not found (in fact, nothing was found instead, either):",
    "Sh*t happens. [I18N TESTING] Take it easy!",
    "[I18N TESTING] Try one of the following:",
    "[I18N TESTING] Check the spelling of the URL path",
    "[I18N TESTING] Press the dice button",
    "Good luck! [I18N TESTING]"
  };

  const bool shouldTranslate = url.find("userlang=test") != std::string::npos;
  const std::string* const t = shouldTranslate ? translatedText : englishText;

  return R"RAWSTRINGLITERAL(<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
    <title>)RAWSTRINGLITERAL" + t[0] + R"RAWSTRINGLITERAL(</title>
    <link type="text/css" href="/ROOT%23%3F/skin/error.css?cacheid=b3fa90cf" rel="Stylesheet" />
    <script>
      window.KIWIX_RESPONSE_TEMPLATE = "&lt;!DOCTYPE html&gt;\n&lt;html&gt;\n  &lt;head&gt;\n    &lt;meta charset=&quot;utf-8&quot;&gt;\n    &lt;meta name=&quot;viewport&quot; content=&quot;width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no&quot; /&gt;\n    &lt;title&gt;{{PAGE_TITLE}}&lt;/title&gt;\n    &lt;link type=&quot;text/css&quot; href=&quot;{{root}}/skin/error.css?cacheid=b3fa90cf&quot; rel=&quot;Stylesheet&quot; /&gt;\n{{#KIWIX_RESPONSE_DATA}}    &lt;script&gt;\n      window.KIWIX_RESPONSE_TEMPLATE = &quot;{{KIWIX_RESPONSE_TEMPLATE}}&quot;;\n      window.KIWIX_RESPONSE_DATA = {{{KIWIX_RESPONSE_DATA}}};\n    &lt;/script&gt;{{/KIWIX_RESPONSE_DATA}}\n  &lt;/head&gt;\n  &lt;body&gt;\n    &lt;header&gt;\n        &lt;img src=&quot;{{root}}/skin/404.svg?cacheid=b6d648af&quot;\n             alt=&quot;{{404_img_text}}&quot;\n             aria-label=&quot;{{404_img_text}}&quot;\n             title=&quot;{{404_img_text}}&quot;&gt;\n    &lt;/header&gt;\n    &lt;section class=&quot;intro&quot;&gt;\n      &lt;h1&gt;{{PAGE_HEADING}}&lt;/h1&gt;\n      &lt;p&gt;{{path_was_not_found_msg}}&lt;/p&gt;\n      &lt;p&gt;&lt;code&gt;{{url_path}}&lt;/code&gt;&lt;/p&gt;\n    &lt;/section&gt;\n    &lt;section class=&quot;advice&quot;&gt;\n      &lt;p&gt;{{advice.p1}}&lt;/p&gt;\n      &lt;p class=&quot;list-intro&quot;&gt;{{advice.p2}}&lt;/p&gt;\n      &lt;ul&gt;\n          &lt;li&gt;{{advice.p3}}&lt;/li&gt;\n          &lt;li&gt;{{advice.p4}}&lt;/li&gt;\n      &lt;/ul&gt;\n      &lt;p&gt;{{advice.p5}}&lt;/p&gt;\n    &lt;/section&gt;\n  &lt;/body&gt;\n&lt;/html&gt;\n";
      window.KIWIX_RESPONSE_DATA = { "404_img_text" : { "msgid" : "404-img-text", "params" : { } }, "PAGE_HEADING" : { "msgid" : "new-404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "new-404-page-title", "params" : { } }, "advice" : { "p1" : { "msgid" : "404-advice.p1", "params" : { } }, "p2" : { "msgid" : "404-advice.p2", "params" : { } }, "p3" : { "msgid" : "404-advice.p3", "params" : { } }, "p4" : { "msgid" : "404-advice.p4", "params" : { } }, "p5" : { "msgid" : "404-advice.p5", "params" : { } } }, "path_was_not_found_msg" : { "msgid" : "path-was-not-found", "params" : { } }, "root" : "/ROOT%23%3F", "url_path" : ")RAWSTRINGLITERAL"
  +         // inject the URL
  jsSafeUrl // inject the URL
  +         // inject the URL
  R"RAWSTRINGLITERAL(" };
    </script>
  </head>
  <body>
    <header>
        <img src="/ROOT%23%3F/skin/404.svg?cacheid=b6d648af"
             alt=")RAWSTRINGLITERAL" + t[1] + R"RAWSTRINGLITERAL("
             aria-label=")RAWSTRINGLITERAL" + t[1] + R"RAWSTRINGLITERAL("
             title=")RAWSTRINGLITERAL" + t[1] + R"RAWSTRINGLITERAL(">
    </header>
    <section class="intro">
      <h1>)RAWSTRINGLITERAL" + t[2] + R"RAWSTRINGLITERAL(</h1>
      <p>)RAWSTRINGLITERAL" + t[3] + R"RAWSTRINGLITERAL(</p>
      <p><code>)RAWSTRINGLITERAL"
  +           // inject the URL
  htmlSafeUrl // inject the URL
  +           // inject the URL
  R"RAWSTRINGLITERAL(</code></p>
    </section>
    <section class="advice">
      <p>)RAWSTRINGLITERAL" + t[4] + R"RAWSTRINGLITERAL(</p>
      <p class="list-intro">)RAWSTRINGLITERAL" + t[5] + R"RAWSTRINGLITERAL(</p>
      <ul>
          <li>)RAWSTRINGLITERAL" + t[6] + R"RAWSTRINGLITERAL(</li>
          <li>)RAWSTRINGLITERAL" + t[7] + R"RAWSTRINGLITERAL(</li>
      </ul>
      <p>)RAWSTRINGLITERAL" + t[8] + R"RAWSTRINGLITERAL(</p>
    </section>
  </body>
</html>
)RAWSTRINGLITERAL";
}

TEST_F(ServerTest, HttpSexy404HtmlError)
{
  const std::vector<std::string> testUrls {
    // XXX: Nicer 404 error page no longer hints whether the error
    // XXX: is because of the missing book/ZIM-file or a missing article
    // XXX: inside a valid/existing book/ZIM-file. However it makes sense
    // XXX: to preserve both cases.
    "/ROOT%23%3F/content/invalid-book/whatever",
    "/ROOT%23%3F/content/invalid-book/whatever?userlang=test",
    "/ROOT%23%3F/content/zimfile/invalid-article",
    "/ROOT%23%3F/content/zimfile/invalid-article?userlang=test",

    // malicious URLs
    R"(/ROOT%23%3F/content/"><svg onload=alert(1)>)",
    R"(/ROOT%23%3F/content/zimfile/"><svg onload=alert(1)>)",

    // XXX: This test case is against a "</script>" string appearing inside
    // XXX: javascript code that will confuse the HTML parser
    R"(/ROOT%23%3F/content/zimfile/</script>)",
  };

  for ( const auto& url : testUrls ) {
    const TestContext ctx{ {"url", url} };
    const auto r = zfs1_->GET(url.c_str());
    EXPECT_EQ(r->status, 404) << ctx;
    EXPECT_EQ(r->body, expectedSexy404ErrorHtml(url)) << ctx;
  }
}

TEST_F(ServerTest, Http400HtmlError)
{
  using namespace TestingOfHtmlResponses;
  const std::vector<TestContentIn400HtmlResponse> testData{
    { /* url */ "/ROOT%23%3F/search",
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "400-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "400-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "invalid-request", "params" : { "url" : "/ROOT%23%3F/search" } } }, { "p" : { "msgid" : "too-many-books", "params" : { "LIMIT" : "3", "NB_BOOKS" : "4" } } } ] })" &&
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
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "400-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "400-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "invalid-request", "params" : { "url" : "/ROOT%23%3F/search?content=zimfile" } } }, { "p" : { "msgid" : "no-query", "params" : { } } } ] })" &&
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
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "400-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "400-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "invalid-request", "params" : { "url" : "/ROOT%23%3F/search?content=non-existing-book&pattern=asdfqwerty" } } }, { "p" : { "msgid" : "no-such-book", "params" : { "BOOK_NAME" : "non-existing-book" } } } ] })" &&
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
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "400-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "400-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "invalid-request", "params" : { "url" : "/ROOT%23%3F/search?content=non-existing-book&pattern=a%22%3Cscript%20foo%3E" } } }, { "p" : { "msgid" : "no-such-book", "params" : { "BOOK_NAME" : "non-existing-book" } } } ] })" &&
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
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "400-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "400-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "invalid-request", "params" : { "url" : "/ROOT%23%3F/search?books.filter.lang=eng&pattern" } } }, { "p" : { "msgid" : "no-query", "params" : { } } } ] })" &&
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
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "400-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "400-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "invalid-request", "params" : { "url" : "/ROOT%23%3F/search?pattern=foo" } } }, { "p" : { "msgid" : "too-many-books", "params" : { "LIMIT" : "3", "NB_BOOKS" : "4" } } } ] })" &&
      expected_body==R"(
    <h1>Invalid request</h1>
    <p>
      The requested URL "/ROOT%23%3F/search?pattern=foo" is not a valid request.
    </p>
    <p>
      Too many books requested (4) where limit is 3
    </p>
)"  },

    // Testing of translation
    { /* url */ "/ROOT%23%3F/search?content=zimfile&userlang=test",
      expected_page_title=="[I18N TESTING] Invalid request ($400 fine must be paid)" &&
      expected_kiwix_response_data==R"({ "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "400-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "400-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "invalid-request", "params" : { "url" : "/ROOT%23%3F/search?content=zimfile&userlang=test" } } }, { "p" : { "msgid" : "no-query", "params" : { } } } ] })" &&
      expected_body==R"(
    <h1>[I18N TESTING] -400 karma for an invalid request</h1>
    <p>
      [I18N TESTING] Invalid URL: "/ROOT%23%3F/search?content=zimfile&userlang=test"
    </p>
    <p>
      [I18N TESTING] Kiwix can read your thoughts but it is against GDPR. Please provide your query explicitly.
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

std::string expectedSexy500ErrorHtml(const std::string& url,
                                     const std::string& error)
{
  const auto urlWithoutQuery = replace(url, "\\?.*$", "");
  const auto htmlSafeUrl = htmlEscape(urlWithoutQuery);
  const auto jsSafeUrl = escapeJsString(urlWithoutQuery);

  const std::string englishText[] = {
    "Internal Server Error",
    "Page isn&apos;t working",
    "Oops. Page isn&apos;t working.",
    "The requested path cannot be properly delivered:",
  };

  const std::string translatedText[] = {
    "[I18N] Internal Server Error [TESTING]",
    "Page [I18N] isn&apos;t [TESTING] working",
    "Oops. [I18N] Page isn&apos;t [TESTING] working.",
    "The requested path [I18N TESTING] cannot be properly delivered:",
  };

  const bool shouldTranslate = url.find("userlang=test") != std::string::npos;
  const std::string* const t = shouldTranslate ? translatedText : englishText;

  return R"RAWSTRINGLITERAL(<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
    <title>)RAWSTRINGLITERAL" + t[0] + R"RAWSTRINGLITERAL(</title>
    <link type="text/css" href="/ROOT%23%3F/skin/error.css?cacheid=b3fa90cf" rel="Stylesheet" />
    <script>
      window.KIWIX_RESPONSE_TEMPLATE = "&lt;!DOCTYPE html&gt;\n&lt;html&gt;\n  &lt;head&gt;\n    &lt;meta charset=&quot;utf-8&quot;&gt;\n    &lt;meta name=&quot;viewport&quot; content=&quot;width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no&quot; /&gt;\n    &lt;title&gt;{{PAGE_TITLE}}&lt;/title&gt;\n    &lt;link type=&quot;text/css&quot; href=&quot;{{root}}/skin/error.css?cacheid=b3fa90cf&quot; rel=&quot;Stylesheet&quot; /&gt;\n    &lt;script&gt;\n      window.KIWIX_RESPONSE_TEMPLATE = &quot;{{KIWIX_RESPONSE_TEMPLATE}}&quot;;\n      window.KIWIX_RESPONSE_DATA = {{{KIWIX_RESPONSE_DATA}}};\n    &lt;/script&gt;\n  &lt;/head&gt;\n  &lt;body&gt;\n    &lt;header&gt;\n        &lt;img src=&quot;{{root}}/skin/500.svg?cacheid=32eb0f20&quot;\n             alt=&quot;{{500_img_text}}&quot;\n             aria-label=&quot;{{500_img_text}}&quot;\n             title=&quot;{{500_img_text}}&quot;&gt;\n    &lt;/header&gt;\n    &lt;section class=&quot;intro&quot;&gt;\n      &lt;h1&gt;{{PAGE_HEADING}}&lt;/h1&gt;\n      &lt;p&gt;{{PAGE_TEXT}}&lt;/p&gt;\n      &lt;p&gt;&lt;code&gt;{{url_path}}&lt;/code&gt;&lt;/p&gt;\n    &lt;/section&gt;\n{{#error}}\n    &lt;section class=&quot;advice&quot;&gt;\n      &lt;p&gt;{{error}}&lt;/p&gt;\n    &lt;/section&gt;\n{{/error}}\n  &lt;/body&gt;\n&lt;/html&gt;\n";
      window.KIWIX_RESPONSE_DATA = { "500_img_text" : { "msgid" : "500-img-text", "params" : { } }, "PAGE_HEADING" : { "msgid" : "500-page-heading", "params" : { } }, "PAGE_TEXT" : { "msgid" : "500-page-text", "params" : { } }, "PAGE_TITLE" : { "msgid" : "500-page-title", "params" : { } }, "error" : ")RAWSTRINGLITERAL"
  +                     // inject the error
  escapeJsString(error) // inject the error
  +                     // inject the error
  R"RAWSTRINGLITERAL(", "root" : "/ROOT%23%3F", "url_path" : ")RAWSTRINGLITERAL"
  +         // inject the URL
  jsSafeUrl // inject the URL
  +         // inject the URL
  R"RAWSTRINGLITERAL(" };
    </script>
  </head>
  <body>
    <header>
        <img src="/ROOT%23%3F/skin/500.svg?cacheid=32eb0f20"
             alt=")RAWSTRINGLITERAL" + t[1] + R"RAWSTRINGLITERAL("
             aria-label=")RAWSTRINGLITERAL" + t[1] + R"RAWSTRINGLITERAL("
             title=")RAWSTRINGLITERAL" + t[1] + R"RAWSTRINGLITERAL(">
    </header>
    <section class="intro">
      <h1>)RAWSTRINGLITERAL" + t[2] + R"RAWSTRINGLITERAL(</h1>
      <p>)RAWSTRINGLITERAL" + t[3] + R"RAWSTRINGLITERAL(</p>
      <p><code>)RAWSTRINGLITERAL"
  +           // inject the URL
  htmlSafeUrl // inject the URL
  +           // inject the URL
  R"RAWSTRINGLITERAL(</code></p>
    </section>
    <section class="advice">
      <p>)RAWSTRINGLITERAL" + error + R"RAWSTRINGLITERAL(</p>
    </section>
  </body>
</html>
)RAWSTRINGLITERAL";
}

TEST_F(ServerTest, 500)
{
  struct TestData {
    const std::string url;
    const std::string error;
  };
  const TestData testData[] = {
    {
      "/ROOT%23%3F/content/poor/A/redirect_loop.html",
      "Entry redirect_loop.html is a redirect entry."
    },

    {
      "/ROOT%23%3F/content/poor/A/redirect_loop.html?userlang=test",
      "Entry redirect_loop.html is a redirect entry."
    }
  };

  for ( const auto& td : testData ) {
    const TestContext ctx{ {"url", td.url} };
    const auto r = zfs1_->GET(td.url.c_str());
    EXPECT_EQ(r->status, 500) << ctx;
    EXPECT_EQ(r->get_header_value("Content-Type"), "text/html; charset=utf-8") << ctx;
    EXPECT_EQ(r->body, expectedSexy500ErrorHtml(td.url, td.error)) << ctx;
  }
}

TEST_F(ServerTest, UserLanguageList)
{
  const auto r = zfs1_->GET("/ROOT%23%3F/skin/languages.js");
  EXPECT_EQ(r->body,
R"EXPECTEDRESPONSE(const uiLanguages = [
  {
    "iso_code": "ar",
    "self_name": "الإنجليزية",
    "translation_count": 44
  },
  {
    "iso_code": "bn",
    "self_name": "বাংলা",
    "translation_count": 34
  },
  {
    "iso_code": "br",
    "self_name": "brezhoneg",
    "translation_count": 35
  },
  {
    "iso_code": "cs",
    "self_name": "Čeština",
    "translation_count": 25
  },
  {
    "iso_code": "dag",
    "self_name": "Silimiinsili",
    "translation_count": 48
  },
  {
    "iso_code": "de",
    "self_name": "Deutsch",
    "translation_count": 67
  },
  {
    "iso_code": "el",
    "self_name": "Αγγλικά",
    "translation_count": 23
  },
  {
    "iso_code": "en",
    "self_name": "English",
    "translation_count": 93
  },
  {
    "iso_code": "es",
    "self_name": "español",
    "translation_count": 67
  },
  {
    "iso_code": "fi",
    "self_name": "suomi",
    "translation_count": 29
  },
  {
    "iso_code": "fr",
    "self_name": "Français",
    "translation_count": 84
  },
  {
    "iso_code": "ha",
    "self_name": "Turanci",
    "translation_count": 57
  },
  {
    "iso_code": "he",
    "self_name": "עברית",
    "translation_count": 80
  },
  {
    "iso_code": "hi",
    "self_name": "हिन्दी",
    "translation_count": 59
  },
  {
    "iso_code": "hu",
    "self_name": "Magyar",
    "translation_count": 32
  },
  {
    "iso_code": "hy",
    "self_name": "Հայերեն",
    "translation_count": 26
  },
  {
    "iso_code": "ia",
    "self_name": "interlingua",
    "translation_count": 84
  },
  {
    "iso_code": "id",
    "self_name": "Bahasa Inggris",
    "translation_count": 68
  },
  {
    "iso_code": "ig",
    "self_name": "Bekee",
    "translation_count": 57
  },
  {
    "iso_code": "it",
    "self_name": "italiano",
    "translation_count": 58
  },
  {
    "iso_code": "ja",
    "self_name": "日本語",
    "translation_count": 26
  },
  {
    "iso_code": "ko",
    "self_name": "한국어",
    "translation_count": 73
  },
  {
    "iso_code": "ku-latn",
    "self_name": "kurdî",
    "translation_count": 26
  },
  {
    "iso_code": "lb",
    "self_name": "Lëtzebuergesch",
    "translation_count": 48
  },
  {
    "iso_code": "mk",
    "self_name": "македонски",
    "translation_count": 81
  },
  {
    "iso_code": "ms",
    "self_name": "Bahasa Melayu",
    "translation_count": 14
  },
  {
    "iso_code": "nb",
    "self_name": "Engelsk",
    "translation_count": 68
  },
  {
    "iso_code": "nl",
    "self_name": "Nederlands",
    "translation_count": 68
  },
  {
    "iso_code": "nqo",
    "self_name": "ߒߞߏ",
    "translation_count": 43
  },
  {
    "iso_code": "or",
    "self_name": "ଓଡ଼ିଆ",
    "translation_count": 49
  },
  {
    "iso_code": "pl",
    "self_name": "Polski",
    "translation_count": 32
  },
  {
    "iso_code": "pt-br",
    "self_name": "Português",
    "translation_count": 66
  },
  {
    "iso_code": "pt",
    "self_name": "português",
    "translation_count": 67
  },
  {
    "iso_code": "ro",
    "self_name": "Engleză",
    "translation_count": 68
  },
  {
    "iso_code": "ru",
    "self_name": "русский",
    "translation_count": 67
  },
  {
    "iso_code": "sc",
    "self_name": "Sardu",
    "translation_count": 49
  },
  {
    "iso_code": "sk",
    "self_name": "slovenčina",
    "translation_count": 25
  },
  {
    "iso_code": "skr-arab",
    "self_name": "سرائیکی",
    "translation_count": 33
  },
  {
    "iso_code": "sl",
    "self_name": "slovenščina",
    "translation_count": 57
  },
  {
    "iso_code": "sq",
    "self_name": "Shqip",
    "translation_count": 67
  },
  {
    "iso_code": "sv",
    "self_name": "Svenska",
    "translation_count": 67
  },
  {
    "iso_code": "sw",
    "self_name": "Kiswahili",
    "translation_count": 58
  },
  {
    "iso_code": "te",
    "self_name": "ఇంగ్లీషు",
    "translation_count": 49
  },
  {
    "iso_code": "tr",
    "self_name": "Türkçe",
    "translation_count": 57
  },
  {
    "iso_code": "zh-hans",
    "self_name": "简体中文",
    "translation_count": 68
  },
  {
    "iso_code": "zh-hant",
    "self_name": "繁體中文",
    "translation_count": 92
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
    const std::string expectedH1;

    operator TestContext() const
    {
      TestContext ctx{
          {"description", description},
          {"url", url},
          {"acceptLanguageHeader", acceptLanguageHeader},
      };

      return ctx;
    }
  };

  const TestData testData[] = {
    {
      "Default user language is English",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "",
      /* expected <h1> */ "Oops. Page not found."
    },
    {
      "userlang URL query parameter is respected",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article?userlang=en",
      /*Accept-Language:*/ "",
      /* expected <h1> */ "Oops. Page not found."
    },
    {
      "userlang URL query parameter is respected",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article?userlang=test",
      /*Accept-Language:*/ "",
      /* expected <h1> */ "[I18N TESTING] Oops. Larry Page could not be reached. He may be on paternity leave."
    },
    {
      "'Accept-Language: *' is handled",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "*",
      /* expected <h1> */ "Oops. Page not found."
    },
    {
      "Accept-Language: header is respected",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "test",
      /* expected <h1> */ "[I18N TESTING] Oops. Larry Page could not be reached. He may be on paternity leave."
    },
    {
      "userlang query parameter takes precedence over Accept-Language",
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article?userlang=en",
      /*Accept-Language:*/ "test",
      /* expected <h1> */ "Oops. Page not found."
    },
    {
      "Most suitable language is selected from the Accept-Language header",
      // In case of a comma separated list of languages (optionally weighted
      // with quality values) the most suitable language is selected.
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "test;q=0.9, en;q=0.2",
      /* expected <h1> */ "[I18N TESTING] Oops. Larry Page could not be reached. He may be on paternity leave."
    },
    {
      "Most suitable language is selected from the Accept-Language header",
      // In case of a comma separated list of languages (optionally weighted
      // with quality values) the most suitable language is selected.
      /*url*/ "/ROOT%23%3F/content/zimfile/invalid-article",
      /*Accept-Language:*/ "test;q=0.2, en;q=0.9",
      /* expected <h1> */ "Oops. Page not found."
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
    EXPECT_FALSE(r->has_header("Set-Cookie"));
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
