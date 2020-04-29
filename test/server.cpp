#include "gtest/gtest.h"

#include "../include/manager.h"
#include "../include/server.h"
#include "../include/name_mapper.h"

#include "./httplib.h"

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

class ServerTest : public ::testing::Test
{
protected:
  std::unique_ptr<ZimFileServer>   zfs1_;

  const int PORT = 8001;
  const std::string ZIMFILE = "./test/zimfile.zim";

protected:
  void SetUp() override {
    zfs1_.reset(new ZimFileServer(PORT, ZIMFILE));
  }

  void TearDown() override {
    zfs1_.reset();
  }
};

struct Resource
{
  const char* url;
};

std::ostream& operator<<(std::ostream& out, const Resource& r)
{
  out << "url: " << r.url;
  return out;
}

Resource resources200Compressible[] = {
  { "/" },

  { "/skin/jquery-ui/jquery-ui.structure.min.css" },
  { "/skin/jquery-ui/jquery-ui.min.js" },
  { "/skin/jquery-ui/external/jquery/jquery.js" },
  { "/skin/jquery-ui/jquery-ui.theme.min.css" },
  { "/skin/jquery-ui/jquery-ui.min.css" },
  { "/skin/taskbar.js" },
  { "/skin/taskbar.css" },
  { "/skin/block_external.js" },

  { "/search?content=zimfile&pattern=abcd" },

  { "/suggest?content=zimfile&term=ray" },

  { "/catch/external?source=www.example.com" },

  { "/zimfile/A/index" },
  { "/zimfile/A/Ray_Charles" },
};

Resource resources200Uncompressible[] = {
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
  { "/skin/caret.png" },

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

  { "/zimfile/I/m/Ray_Charles_classic_piano_pose.jpg" },
};


TEST_F(ServerTest, 200)
{
  for ( const Resource& res : resources200Compressible )
    EXPECT_EQ(200, zfs1_->GET(res.url)->status) << "res.url: " << res.url;

  for ( const Resource& res : resources200Uncompressible )
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
