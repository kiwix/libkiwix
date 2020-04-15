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

class ServerTest : public ::testing::Test
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
};

TEST_F(ServerTest, HeadMethodIsSupported)
{
  ASSERT_EQ(200, zfs1_->HEAD("/")->status);
}

TEST_F(ServerTest, TheResponseToHeadRequestHasNoBody)
{
  ASSERT_TRUE(zfs1_->HEAD("/")->body.empty());
}

TEST_F(ServerTest, HeadersAreTheSameInResponsesToHeadAndGetRequests)
{
  httplib::Headers g = zfs1_->GET("/")->headers;
  httplib::Headers h = zfs1_->HEAD("/")->headers;
  g.erase("Date");
  h.erase("Date");
  ASSERT_EQ(g, h);
}

TEST_F(ServerTest, ETagHeaderIsSet)
{
  const auto responseToGet = zfs1_->GET("/");
  EXPECT_TRUE(responseToGet->has_header("ETag"));
  EXPECT_TRUE(is_valid_etag(responseToGet->get_header_value("ETag")));

  const auto responseToHead = zfs1_->HEAD("/");
  EXPECT_TRUE(responseToHead->has_header("ETag"));
  EXPECT_TRUE(is_valid_etag(responseToHead->get_header_value("ETag")));
}

TEST_F(ServerTest, EtagIsTheSameInResponsesToDifferentRequestsOfTheSameURL)
{
  const auto h1 = zfs1_->HEAD("/");
  const auto h2 = zfs1_->HEAD("/");
  ASSERT_EQ(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
}

TEST_F(ServerTest, EtagIsTheSameAcrossHeadAndGet)
{
  const auto g = zfs1_->GET("/");
  const auto h = zfs1_->HEAD("/");
  ASSERT_EQ(h->get_header_value("ETag"), g->get_header_value("ETag"));
}

TEST_F(ServerTest, DifferentServerInstancesProduceDifferentETags)
{
  ZimFileServer zfs2(PORT + 1, ZIMFILE);
  const auto h1 = zfs1_->HEAD("/");
  const auto h2 = zfs2.HEAD("/");
  ASSERT_NE(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
}

TEST_F(ServerTest, IfNoneMatchRequestsWithMatchingEtagResultIn304Responses)
{
  const auto g = zfs1_->GET("/");
  const auto etag = g->get_header_value("Etag");
  const auto h = zfs1_->HEAD("/", { {"If-None-Match", etag} } );
  const auto g2 = zfs1_->GET("/", { {"If-None-Match", etag} } );
  EXPECT_EQ(304, h->status);
  EXPECT_EQ(304, g2->status);
}

TEST_F(ServerTest, IfNoneMatchRequestsWithMismatchingEtagResultIn200Responses)
{
  const auto g = zfs1_->GET("/");
  const auto etag = g->get_header_value("Etag");
  const auto etag2 = etag.substr(0, etag.size() - 1) + "x\"";
  const auto h = zfs1_->HEAD("/", { {"If-None-Match", etag2} } );
  const auto g2 = zfs1_->GET("/", { {"If-None-Match", etag2} } );
  EXPECT_EQ(200, h->status);
  EXPECT_EQ(200, g2->status);
}
