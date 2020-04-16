#include "gtest/gtest.h"

#include "../include/manager.h"
#include "../include/server.h"
#include "../include/name_mapper.h"

#include "./httplib.h"

bool is_valid_etag(const std::string& etag)
{
  return etag.size() >= 2 &&
         etag.front() == '"' &&
         etag.back() == '"';
}

class ZimFileServer
{
public: // types
  typedef httplib::Headers                    Headers;
  typedef std::shared_ptr<httplib::Response>  Response;

public: // functions
  ZimFileServer(int serverPort, std::string zimpath);
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

ZimFileServer::ZimFileServer(int serverPort, std::string zimpath)
: manager(&this->library)
{
  if (!manager.addBookFromPath(zimpath, zimpath, "", false))
    throw std::runtime_error("Unable to add the ZIM file '" + zimpath + "'");

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

class ServerTest : public ::testing::TestWithParam<const char*>
{
protected:
  std::unique_ptr<ZimFileServer>   zfs1_;

  const int PORT = 8001;
  const std::string ZIMFILE = "zimfile.zim";

protected:
  void SetUp() override {
    zfs1_.reset(new ZimFileServer(PORT, ZIMFILE));
  }

  void TearDown() override {
    zfs1_.reset();
  }

  const char* test_path() const { return GetParam(); }
};

struct Resource
{
  const char* url;
};

Resource resources200[] = {
  { "/" },

  { "/skin/jquery-ui/jquery-ui.structure.min.css" },
  { "/skin/jquery-ui/jquery-ui.min.js" },
  { "/skin/jquery-ui/external/jquery/jquery.js" },
  { "/skin/jquery-ui/images/ui-bg_flat_0_aaaaaa_40x100.png" },
  { "/skin/jquery-ui/images/ui-bg_flat_75_ffffff_40x100.png" },
  { "/skin/jquery-ui/images/ui-icons_222222_256x240.png" },
  { "/skin/jquery-ui/images/ui-bg_glass_55_fbf9ee_1x400.png" },
  { "/skin/jquery-ui/images/ui-bg_highlight-soft_75_cccccc_1x100.png" },
  { "/skin/jquery-ui/images/ui-bg_glass_65_ffffff_1x400.png" },
  { "/skin/jquery-ui/images/ui-icons_2e83ff_256x240.png" },
  { "/skin/jquery-ui/images/ui-icons_cd0a0a_256x240.png" },
  { "/skin/jquery-ui/images/ui-icons_888888_256x240.png" },
  { "/skin/jquery-ui/images/ui-bg_glass_75_e6e6e6_1x400.png" },
  { "/skin/jquery-ui/images/animated-overlay.gif" },
  { "/skin/jquery-ui/images/ui-bg_glass_75_dadada_1x400.png" },
  { "/skin/jquery-ui/images/ui-icons_454545_256x240.png" },
  { "/skin/jquery-ui/images/ui-bg_glass_95_fef1ec_1x400.png" },
  { "/skin/jquery-ui/jquery-ui.theme.min.css" },
  { "/skin/jquery-ui/jquery-ui.min.css" },
  { "/skin/caret.png" },
  { "/skin/taskbar.js" },
  { "/skin/taskbar.css" },
  { "/skin/block_external.js" },

  { "/catalog/root.xml" },
  { "/catalog/searchdescription.xml" },
  { "/catalog/search" },

  { "/meta?content=zimfile&name=title" },
  { "/meta?content=zimfile&name=description" },
  { "/meta?content=zimfile&name=language" },
  { "/meta?content=zimfile&name=name" },
  { "/meta?content=zimfile&name=tags" },
  { "/meta?content=zimfile&name=date" },
  { "/meta?content=zimfile&name=creator" },
  { "/meta?content=zimfile&name=publisher" },
  { "/meta?content=zimfile&name=favicon" },

  { "/search?content=zimfile&pattern=abcd" },

  { "/suggest?content=zimfile&term=abcd" },

  { "/catch/external?source=www.example.com" },

  { "/zimfile/A/index" },
};

TEST_F(ServerTest, 200)
{
  for ( const Resource& res : resources200 )
    EXPECT_EQ(200, zfs1_->GET(res.url)->status) << "res.url: " << res.url;
}

TEST_F(ServerTest, BookMainPageIsRedirectedToArticleIndex)
{
  auto g = zfs1_->GET("/zimfile");
  ASSERT_EQ(302, g->status);
  ASSERT_TRUE(g->has_header("Location"));
  ASSERT_EQ("/zimfile/A/index", g->get_header_value("Location"));
}

TEST_P(ServerTest, HeadMethodIsSupported)
{
  ASSERT_EQ(200, zfs1_->HEAD(test_path())->status);
}

TEST_P(ServerTest, TheResponseToHeadRequestHasNoBody)
{
  ASSERT_TRUE(zfs1_->HEAD(test_path())->body.empty());
}

TEST_P(ServerTest, HeadersAreTheSameInResponsesToHeadAndGetRequests)
{
  httplib::Headers g = zfs1_->GET(test_path())->headers;
  httplib::Headers h = zfs1_->HEAD(test_path())->headers;
  g.erase("Date");
  h.erase("Date");
  ASSERT_EQ(g, h);
}

TEST_P(ServerTest, ETagHeaderIsSet)
{
  const auto responseToGet = zfs1_->GET(test_path());
  EXPECT_TRUE(responseToGet->has_header("ETag"));
  EXPECT_TRUE(is_valid_etag(responseToGet->get_header_value("ETag")));

  const auto responseToHead = zfs1_->HEAD(test_path());
  EXPECT_TRUE(responseToHead->has_header("ETag"));
  EXPECT_TRUE(is_valid_etag(responseToHead->get_header_value("ETag")));
}

TEST_P(ServerTest, EtagIsTheSameInResponsesToDifferentRequestsOfTheSameURL)
{
  const auto h1 = zfs1_->HEAD(test_path());
  const auto h2 = zfs1_->HEAD(test_path());
  ASSERT_EQ(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
}

TEST_P(ServerTest, EtagIsTheSameAcrossHeadAndGet)
{
  const auto g = zfs1_->GET(test_path());
  const auto h = zfs1_->HEAD(test_path());
  ASSERT_EQ(h->get_header_value("ETag"), g->get_header_value("ETag"));
}

TEST_P(ServerTest, DifferentServerInstancesProduceDifferentETags)
{
  ZimFileServer zfs2(PORT + 1, ZIMFILE);
  const auto h1 = zfs1_->HEAD(test_path());
  const auto h2 = zfs2.HEAD(test_path());
  ASSERT_NE(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
}

TEST_P(ServerTest, IfNoneMatchRequestsWithMatchingEtagResultIn304Responses)
{
  const auto g = zfs1_->GET(test_path());
  const auto etag = g->get_header_value("Etag");
  const auto h = zfs1_->HEAD(test_path(), { {"If-None-Match", etag} } );
  const auto g2 = zfs1_->GET(test_path(), { {"If-None-Match", etag} } );
  EXPECT_EQ(304, h->status);
  EXPECT_EQ(304, g2->status);
}

TEST_P(ServerTest, IfNoneMatchRequestsWithMismatchingEtagResultIn200Responses)
{
  const auto g = zfs1_->GET(test_path());
  const auto etag = g->get_header_value("Etag");
  const auto etag2 = etag.substr(0, etag.size() - 1) + "x\"";
  const auto h = zfs1_->HEAD(test_path(), { {"If-None-Match", etag2} } );
  const auto g2 = zfs1_->GET(test_path(), { {"If-None-Match", etag2} } );
  EXPECT_EQ(200, h->status);
  EXPECT_EQ(200, g2->status);
}

TEST_P(ServerTest, ETagDependsOnTheValueOfAcceptEncodingHeader)
{
  const auto h1 = zfs1_->HEAD(test_path());
  const auto h2 = zfs1_->HEAD(test_path(), { {"Accept-Encoding", "deflate"} } );
  const auto h3 = zfs1_->HEAD(test_path(), { {"Accept-Encoding", ""} } );
  ASSERT_EQ(200, h2->status);
  ASSERT_EQ(200, h3->status);
  EXPECT_NE(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
  EXPECT_NE(h1->get_header_value("ETag"), h3->get_header_value("ETag"));
  EXPECT_NE(h2->get_header_value("ETag"), h3->get_header_value("ETag"));
}

INSTANTIATE_TEST_CASE_P(Kiwix,
                        ServerTest,
                        ::testing::Values("/", "/zimfile/A/index"));
