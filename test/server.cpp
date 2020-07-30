
#include "./httplib.h"
#include "gtest/gtest.h"

#include "../include/manager.h"
#include "../include/server.h"
#include "../include/name_mapper.h"


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


class ZimFileServer
{
public: // types
  typedef std::shared_ptr<httplib::Response>  Response;
  typedef std::vector<std::string> FilePathCollection;

public: // functions
  ZimFileServer(int serverPort, const FilePathCollection& zimpaths);
  ~ZimFileServer();

  Response GET(const char* path, const Headers& headers = Headers())
  {
    return client->Get(path, headers);
  }

  Response HEAD(const char* path, const Headers& headers = Headers())
  {
    return client->Head(path, headers);
  }

private: // data
  kiwix::Library library;
  kiwix::Manager manager;
  std::unique_ptr<kiwix::HumanReadableNameMapper> nameMapper;
  std::unique_ptr<kiwix::Server> server;
  std::unique_ptr<httplib::Client> client;
};

ZimFileServer::ZimFileServer(int serverPort, const FilePathCollection& zimpaths)
: manager(&this->library)
{
  for ( const auto zimpath : zimpaths ) {
    if (!manager.addBookFromPath(zimpath, zimpath, "", false))
      throw std::runtime_error("Unable to add the ZIM file '" + zimpath + "'");
  }

  const std::string address = "127.0.0.1";
  nameMapper.reset(new kiwix::HumanReadableNameMapper(library, false));
  server.reset(new kiwix::Server(&library, nameMapper.get()));
  server->setAddress(address);
  server->setPort(serverPort);
  server->setNbThreads(2);
  server->setVerbose(false);

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
  { WITH_ETAG, "/" },

  { WITH_ETAG, "/skin/jquery-ui/jquery-ui.structure.min.css" },
  { WITH_ETAG, "/skin/jquery-ui/jquery-ui.min.js" },
  { WITH_ETAG, "/skin/jquery-ui/external/jquery/jquery.js" },
  { WITH_ETAG, "/skin/jquery-ui/jquery-ui.theme.min.css" },
  { WITH_ETAG, "/skin/jquery-ui/jquery-ui.min.css" },
  { WITH_ETAG, "/skin/taskbar.js" },
  { WITH_ETAG, "/skin/taskbar.css" },
  { WITH_ETAG, "/skin/block_external.js" },

  { NO_ETAG,   "/catalog/root.xml" },
  { NO_ETAG,   "/catalog/searchdescription.xml" },
  { NO_ETAG,   "/catalog/search" },

  { NO_ETAG,   "/search?content=zimfile&pattern=abcd" },

  { NO_ETAG,   "/suggest?content=zimfile&term=ray" },

  { NO_ETAG,   "/catch/external?source=www.example.com" },

  { WITH_ETAG, "/zimfile/A/index" },
  { WITH_ETAG, "/zimfile/A/Ray_Charles" },
};

const ResourceCollection resources200Uncompressible{
  { WITH_ETAG, "/skin/jquery-ui/images/animated-overlay.gif" },
  { WITH_ETAG, "/skin/caret.png" },

  { WITH_ETAG, "/meta?content=zimfile&name=title" },
  { WITH_ETAG, "/meta?content=zimfile&name=description" },
  { WITH_ETAG, "/meta?content=zimfile&name=language" },
  { WITH_ETAG, "/meta?content=zimfile&name=name" },
  { WITH_ETAG, "/meta?content=zimfile&name=tags" },
  { WITH_ETAG, "/meta?content=zimfile&name=date" },
  { WITH_ETAG, "/meta?content=zimfile&name=creator" },
  { WITH_ETAG, "/meta?content=zimfile&name=publisher" },
  { WITH_ETAG, "/meta?content=zimfile&name=favicon" },

  { WITH_ETAG, "/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg" },

  { WITH_ETAG, "/corner_cases/A/empty.html" },
  { WITH_ETAG, "/corner_cases/-/empty.css" },
  { WITH_ETAG, "/corner_cases/-/empty.js" },
};

ResourceCollection all200Resources()
{
  return concat(resources200Compressible, resources200Uncompressible);
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

const char* urls404[] = {
  "/non-existent-item",
  "/skin/non-existent-skin-resource",
  "/catalog",
  "/catalog/non-existent-item",
  "/meta",
  "/meta?content=zimfile",
  "/meta?content=zimfile&name=non-existent-item",
  "/meta?content=non-existent-book&name=title",
  "/random",
  "/random?content=non-existent-book",
  "/search",
  "/suggest",
  "/suggest?content=zimfile",
  "/suggest?content=non-existent-book&term=abcd",
  "/catch/external",
  "/zimfile/A/non-existent-article",
};

TEST_F(ServerTest, 404)
{
  for ( const char* url : urls404 )
    EXPECT_EQ(404, zfs1_->GET(url)->status) << "url: " << url;
}

TEST_F(ServerTest, SuccessfulSearchForAnArticleTitleRedirectsToTheArticle)
{
  auto g = zfs1_->GET("/search?content=zimfile&pattern=ray%20charles" );
  ASSERT_EQ(302, g->status);
  ASSERT_TRUE(g->has_header("Location"));
  ASSERT_EQ("/zimfile/A/Ray_Charles", g->get_header_value("Location"));
}

TEST_F(ServerTest, RandomPageRedirectsToAnExistingArticle)
{
  auto g = zfs1_->GET("/random?content=zimfile");
  ASSERT_EQ(302, g->status);
  ASSERT_TRUE(g->has_header("Location"));
  ASSERT_TRUE(g->get_header_value("Location").find("/zimfile/A/") != std::string::npos);
}

TEST_F(ServerTest, BookMainPageIsRedirectedToArticleIndex)
{
  auto g = zfs1_->GET("/zimfile");
  ASSERT_EQ(302, g->status);
  ASSERT_TRUE(g->has_header("Location"));
  ASSERT_EQ("/zimfile/A/index", g->get_header_value("Location"));
}

TEST_F(ServerTest, HeadMethodIsSupported)
{
  for ( const Resource& res : all200Resources() )
    EXPECT_EQ(200, zfs1_->HEAD(res.url)->status) << res;
}

TEST_F(ServerTest, TheResponseToHeadRequestHasNoBody)
{
  for ( const Resource& res : all200Resources() )
    EXPECT_TRUE(zfs1_->HEAD(res.url)->body.empty()) << res;
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
// NOTe: responses).
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
  const char url[] = "/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";
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
  const char url[] = "/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";

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
  const char url[] = "/corner_cases/-/empty.js";

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
  const char url[] = "/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";

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
  const char url[] = "/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg";
  const auto r0 = zfs1_->GET(url, { {"Range", "bytes=100-200"} } );

  const char* header_variations[] = { "RANGE", "range", "rAnGe", "RaNgE" };
  for ( const char* header : header_variations ) {
    const auto r = zfs1_->GET(url, { {header, "bytes=100-200"} } );
    EXPECT_EQ(206, r->status);
    EXPECT_EQ("bytes 100-200/20077", r->get_header_value("Content-Range"));
    EXPECT_EQ(r0->body, r->body);
  }
}
