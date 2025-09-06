
#include "../include/manager.h"
#include "../include/server.h"
#include "../include/name_mapper.h"
#include "../include/tools.h"
#include <zim/entry.h>
#include <zim/item.h>

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

std::string replace(std::string s, std::string pattern, std::string replacement)
{
  return std::regex_replace(s, std::regex(pattern), replacement);
}

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

  enum Options
  {
    NO_TASKBAR_NO_LINK_BLOCKING = 0,

    WITH_TASKBAR         = 1 << 1,
    WITH_LIBRARY_BUTTON  = 1 << 2,
    BLOCK_EXTERNAL_LINKS = 1 << 3,
    NO_NAME_MAPPER       = 1 << 4,
    CATALOG_ONLY_MODE    = 1 << 5,

    WITH_TASKBAR_AND_LIBRARY_BUTTON = WITH_TASKBAR | WITH_LIBRARY_BUTTON,

    DEFAULT_OPTIONS = WITH_TASKBAR | WITH_LIBRARY_BUTTON
  };

  struct Cfg
  {
    std::string root = "ROOT#?";
    std::string contentServerUrl = "";
    Options options = DEFAULT_OPTIONS;

    Cfg(Options opts = DEFAULT_OPTIONS) : options(opts) {}
  };

public: // functions
  ZimFileServer(int serverPort, Cfg cfg, std::string libraryFilePath);
  ZimFileServer(int serverPort,
                Cfg cfg,
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
  std::shared_ptr<kiwix::Library> library;
  kiwix::Manager manager;
  std::shared_ptr<kiwix::NameMapper> nameMapper;
  std::unique_ptr<kiwix::Server> server;
  std::unique_ptr<httplib::Client> client;
  const Cfg cfg;
};

ZimFileServer::ZimFileServer(int serverPort, Cfg _cfg, std::string libraryFilePath)
: library(kiwix::Library::create())
, manager(this->library)
, cfg(_cfg)
{
  if ( kiwix::isRelativePath(libraryFilePath) )
    libraryFilePath = kiwix::computeAbsolutePath(kiwix::getCurrentDirectory(), libraryFilePath);
  manager.readFile(libraryFilePath, true, true);
  run(serverPort);
}

ZimFileServer::ZimFileServer(int serverPort,
                             Cfg _cfg,
                             const FilePathCollection& zimpaths,
                             std::string indexTemplateString)
: library(kiwix::Library::create())
, manager(this->library)
, cfg(_cfg)
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
  if (cfg.options & NO_NAME_MAPPER) {
    nameMapper.reset(new kiwix::IdNameMapper());
  } else {
    nameMapper.reset(new kiwix::HumanReadableNameMapper(*library, false));
  }
  server.reset(new kiwix::Server(library, nameMapper));
  server->setRoot(cfg.root);
  server->setAddress(address);
  server->setPort(serverPort);
  server->setNbThreads(2);
  server->setVerbose(false);
  server->setTaskbar(cfg.options & WITH_TASKBAR, cfg.options & WITH_LIBRARY_BUTTON);
  server->setBlockExternalLinks(cfg.options & BLOCK_EXTERNAL_LINKS);
  server->setMultiZimSearchLimit(3);
  server->setCatalogOnlyMode(cfg.options & CATALOG_ONLY_MODE);
  server->setContentServerUrl(cfg.contentServerUrl);
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

  const ZimFileServer::FilePathCollection ZIMFILES {
    "./test/zimfile.zim",
    "./test/example.zim",
    "./test/poor.zim",
    "./test/corner_cases#&.zim"
  };

protected:
  void SetUp() override {
    resetServer(ZimFileServer::DEFAULT_OPTIONS);
  }

  void resetServer(ZimFileServer::Cfg cfg) {
    zfs1_.reset();
    zfs1_.reset(new ZimFileServer(SERVER_PORT, cfg, ZIMFILES));
  }

  void TearDown() override {
    zfs1_.reset();
  }
};

static const std::string ERROR_HTML_TEMPLATE_JS_STRING = R"("&lt;!DOCTYPE html&gt;\n&lt;html xmlns=&quot;http://www.w3.org/1999/xhtml&quot;&gt;\n  &lt;head&gt;\n    &lt;meta content=&quot;text/html;charset=UTF-8&quot; http-equiv=&quot;content-type&quot; /&gt;\n    &lt;title&gt;{{PAGE_TITLE}}&lt;/title&gt;\n{{#CSS_URL}}\n    &lt;link type=&quot;text/css&quot; href=&quot;{{{CSS_URL}}}&quot; rel=&quot;Stylesheet&quot; /&gt;\n{{/CSS_URL}}{{#KIWIX_RESPONSE_DATA}}    &lt;script&gt;\n      window.KIWIX_RESPONSE_TEMPLATE = &quot;{{KIWIX_RESPONSE_TEMPLATE}}&quot;;\n      window.KIWIX_RESPONSE_DATA = {{{KIWIX_RESPONSE_DATA}}};\n    &lt;/script&gt;{{/KIWIX_RESPONSE_DATA}}\n  &lt;/head&gt;\n  &lt;body&gt;\n    &lt;h1&gt;{{PAGE_HEADING}}&lt;/h1&gt;\n{{#details}}\n    &lt;p&gt;\n      {{{p}}}\n    &lt;/p&gt;\n{{/details}}\n  &lt;/body&gt;\n&lt;/html&gt;\n")";
