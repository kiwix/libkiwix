#include "gtest/gtest.h"

#include "kiwix/manager.h"
#include "kiwix/server.h"
#include "kiwix/name_mapper.h"

#include "httplib.h"


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
  std::unique_ptr<httplib::Client> client1_;

protected:
  void SetUp() override {
    zfs1_.reset(new ZimFileServer("127.0.0.1", 8888, "zimfile.zim"));
    client1_.reset(new httplib::Client("127.0.0.1", 8888));
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
  const auto h1 = client1_->Head("/");
  ASSERT_TRUE(h1->has_header("ETag"));
}

TEST_F(Server, shouldUseTheSameEtagInResponsesToDifferentRequestsOfTheSameURL)
{
  const auto h1 = client1_->Head("/");
  const auto h2 = client1_->Head("/");
  ASSERT_EQ(h1->get_header_value("ETag"), h2->get_header_value("ETag"));
}
