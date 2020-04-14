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
public:
  ZimFileServer(std::string address, int serverPort, std::string zimpath);
  ~ZimFileServer();

private:
  kiwix::Library library;
  kiwix::Manager manager;
  std::unique_ptr<kiwix::HumanReadableNameMapper> nameMapper;
  std::unique_ptr<kiwix::Server> server;
};

ZimFileServer::ZimFileServer(std::string address, int serverPort, std::string zimpath)
: manager(&this->library)
{
  if (!manager.addBookFromPath(zimpath, zimpath, "", false))
    throw std::runtime_error("Unable to add the ZIM file '" + zimpath + "'");

  nameMapper.reset(new kiwix::HumanReadableNameMapper(library, false));
  server.reset(new kiwix::Server(&library, nameMapper.get()));
  server->setAddress(address);
  server->setPort(serverPort);
  server->setNbThreads(2);
  server->setVerbose(false);

  if ( !server->start() )
    throw std::runtime_error("ZimFileServer failed to start");
}

ZimFileServer::~ZimFileServer()
{
  server->stop();
}

class Server : public ::testing::Test
{
protected:
  std::unique_ptr<ZimFileServer>   zfs1_;
  std::unique_ptr<ZimFileServer>   zfs2_;
  std::unique_ptr<httplib::Client> client1_;
  std::unique_ptr<httplib::Client> client2_;

protected:
  void SetUp() override {
    zfs1_.reset(new ZimFileServer("127.0.0.1", 8001, "zimfile.zim"));
    zfs2_.reset(new ZimFileServer("127.0.0.1", 8002, "zimfile.zim"));
    client1_.reset(new httplib::Client("127.0.0.1", 8001));
    client2_.reset(new httplib::Client("127.0.0.1", 8002));
  }

  void TearDown() override {
    zfs1_.reset();
  }
};

TEST_F(Server, shouldSupportTheHeadMethod)
{
  ASSERT_EQ(200, client1_->Head("/")->status);
}

TEST_F(Server, shouldSetTheETagHeader)
{
  const auto responseToGet = client1_->Get("/");
  EXPECT_TRUE(responseToGet->has_header("ETag"));
  EXPECT_TRUE(is_valid_etag(responseToGet->get_header_value("ETag")));

  const auto responseToHead = client1_->Head("/");
  EXPECT_TRUE(responseToHead->has_header("ETag"));
  EXPECT_TRUE(is_valid_etag(responseToHead->get_header_value("ETag")));
}

TEST_F(Server, shouldUseTheSameEtagInResponsesToDifferentRequestsOfTheSameURL)
{
  const auto h1 = client1_->Head("/");
  const auto h2 = client1_->Head("/");
  ASSERT_EQ(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
}

TEST_F(Server, shouldUseTheSameEtagForHeadAndGet)
{
  const auto g = client1_->Get("/");
  const auto h = client1_->Head("/");
  ASSERT_EQ(h->get_header_value("ETag"), g->get_header_value("ETag"));
}

TEST_F(Server, differentServerInstancesShouldProduceDifferentETags)
{
  const auto h1 = client1_->Head("/");
  const auto h2 = client2_->Head("/");
  ASSERT_NE(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
}

TEST_F(Server, shouldRespondWith304ToIfNoneMatchRequestsWithMatchingEtag)
{
  const auto g = client1_->Get("/");
  const auto etag = g->get_header_value("Etag");
  const auto h = client1_->Head("/", { {"If-None-Match", etag} } );
  const auto g2 = client1_->Get("/", { {"If-None-Match", etag} } );
  EXPECT_EQ(304, h->status);
  EXPECT_EQ(304, g2->status);
}

TEST_F(Server, shouldSendFullResponseToIfNoneMatchRequestsWithMismatchingEtag)
{
  const auto g = client1_->Get("/");
  const auto etag = g->get_header_value("Etag");
  const auto etag2 = etag.substr(0, etag.size() - 1) + "x\"";
  const auto h = client1_->Head("/", { {"If-None-Match", etag2} } );
  const auto g2 = client1_->Get("/", { {"If-None-Match", etag2} } );
  EXPECT_EQ(200, h->status);
  EXPECT_EQ(200, g2->status);
}
