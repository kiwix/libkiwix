/*
 * Copyright 2019 Matthieu Gautier <mgautier@kymeria.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "internalServer.h"

#ifdef __FreeBSD__
#include <netinet/in.h>
#endif

#ifdef _WIN32
# if !defined(__MINGW32__) && (_MSC_VER < 1600)
#   include "stdint4win.h"
# endif
# include <winsock2.h>
# include <ws2tcpip.h>
# ifdef __GNUC__
  // inet_pton is not declared in mingw, even if the function exists.
  extern "C" {
    WINSOCK_API_LINKAGE  INT WSAAPI inet_pton( INT Family, PCSTR pszAddrString, PVOID pAddrBuf);
  }
# endif
  typedef UINT64 uint64_t;
  typedef UINT16 uint16_t;
#endif

extern "C" {
#include "microhttpd_wrapper.h"
}

#include "tools/otherTools.h"
#include "tools/pathTools.h"
#include "tools/regexTools.h"
#include "tools/stringTools.h"
#include "library.h"
#include "name_mapper.h"
#include "entry.h"
#include "searcher.h"
#include "search_renderer.h"
#include "opds_dumper.h"

#include <zim/uuid.h>

#include <mustache.hpp>

#include <atomic>
#include <string>
#include <vector>
#include <chrono>
#include "kiwixlib-resources.h"

#ifndef _WIN32
# include <arpa/inet.h>
#endif

#include "request_context.h"
#include "response.h"

#define MAX_SEARCH_LEN 140
#define KIWIX_MIN_CONTENT_SIZE_TO_DEFLATE 100

namespace kiwix {

static IdNameMapper defaultNameMapper;

static MHD_Result staticHandlerCallback(void* cls,
                                        struct MHD_Connection* connection,
                                        const char* url,
                                        const char* method,
                                        const char* version,
                                        const char* upload_data,
                                        size_t* upload_data_size,
                                        void** cont_cls);


InternalServer::InternalServer(Library* library,
                               NameMapper* nameMapper,
                               std::string addr,
                               int port,
                               std::string root,
                               int nbThreads,
                               bool verbose,
                               bool withTaskbar,
                               bool withLibraryButton,
                               bool blockExternalLinks) :
  m_addr(addr),
  m_port(port),
  m_root(root),
  m_nbThreads(nbThreads),
  m_verbose(verbose),
  m_withTaskbar(withTaskbar),
  m_withLibraryButton(withLibraryButton),
  m_blockExternalLinks(blockExternalLinks),
  mp_daemon(nullptr),
  mp_library(library),
  mp_nameMapper(nameMapper ? nameMapper : &defaultNameMapper)
{}

bool InternalServer::start() {
#ifdef _WIN32
  int flags = MHD_USE_SELECT_INTERNALLY;
#else
  int flags = MHD_USE_POLL_INTERNALLY;
#endif
  if (m_verbose.load())
    flags |= MHD_USE_DEBUG;


  struct sockaddr_in sockAddr;
  memset(&sockAddr, 0, sizeof(sockAddr));
  sockAddr.sin_family = AF_INET;
  sockAddr.sin_port = htons(m_port);
  if (m_addr.empty()) {
    if (0 != INADDR_ANY)
      sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    if (inet_pton(AF_INET, m_addr.c_str(), &(sockAddr.sin_addr.s_addr)) == 0) {
      std::cerr << "Ip address " << m_addr << "  is not a valid ip address" << std::endl;
      return false;
    }
  }

  mp_daemon = MHD_start_daemon(flags,
                            m_port,
                            NULL,
                            NULL,
                            &staticHandlerCallback,
                            this,
                            MHD_OPTION_SOCK_ADDR, &sockAddr,
                            MHD_OPTION_THREAD_POOL_SIZE, m_nbThreads,
                            MHD_OPTION_END);
  if (mp_daemon == nullptr) {
    std::cerr << "Unable to instantiate the HTTP daemon. The port " << m_port
              << " is maybe already occupied or need more permissions to be open. "
                 "Please try as root or with a port number higher or equal to 1024."
              << std::endl;
    return false;
  }
  auto server_start_time = std::chrono::system_clock::now().time_since_epoch();
  m_server_id = kiwix::to_string(server_start_time.count());
  return true;
}

void InternalServer::stop()
{
  MHD_stop_daemon(mp_daemon);
}

static MHD_Result staticHandlerCallback(void* cls,
                                        struct MHD_Connection* connection,
                                        const char* url,
                                        const char* method,
                                        const char* version,
                                        const char* upload_data,
                                        size_t* upload_data_size,
                                        void** cont_cls)
{
  InternalServer* _this = static_cast<InternalServer*>(cls);

  return _this->handlerCallback(connection,
                                url,
                                method,
                                version,
                                upload_data,
                                upload_data_size,
                                cont_cls);
}

MHD_Result InternalServer::handlerCallback(struct MHD_Connection* connection,
                                           const char* url,
                                           const char* method,
                                           const char* version,
                                           const char* upload_data,
                                           size_t* upload_data_size,
                                           void** cont_cls)
{
  auto start_time = std::chrono::steady_clock::now();
  if (m_verbose.load() ) {
    printf("======================\n");
    printf("Requesting : \n");
    printf("full_url  : %s\n", url);
  }
  RequestContext request(connection, m_root, url, method, version);

  if (m_verbose.load() ) {
    request.print_debug_info();
  }
  /* Unexpected method */
  if (request.get_method() != RequestMethod::GET
   && request.get_method() != RequestMethod::POST
   && request.get_method() != RequestMethod::HEAD) {
    printf("Reject request because of unhandled request method.\n");
    printf("----------------------\n");
    return MHD_NO;
  }

  auto response = handle_request(request);

  if (response->getReturnCode() == MHD_HTTP_INTERNAL_SERVER_ERROR) {
    printf("========== INTERNAL ERROR !! ============\n");
    if (!m_verbose.load()) {
      printf("Requesting : \n");
      printf("full_url : %s\n", url);
      request.print_debug_info();
    }
  }

  if (response->getReturnCode() == MHD_HTTP_OK && !etag_not_needed(request))
    response->set_server_id(m_server_id);

  auto ret = response->send(request, connection);
  auto end_time = std::chrono::steady_clock::now();
  auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time);
  if (m_verbose.load()) {
    printf("Request time : %fs\n", time_span.count());
    printf("----------------------\n");
  }
  return ret;
}

std::unique_ptr<Response> InternalServer::handle_request(const RequestContext& request)
{
  try {
    if (! request.is_valid_url())
      return Response::build_404(*this, request, "");

    const ETag etag = get_matching_if_none_match_etag(request);
    if ( etag )
      return Response::build_304(*this, etag);

    if (kiwix::startsWith(request.get_url(), "/skin/"))
      return handle_skin(request);

    if (startsWith(request.get_url(), "/catalog"))
      return handle_catalog(request);

    if (request.get_url() == "/meta")
      return handle_meta(request);

    if (request.get_url() == "/search")
      return handle_search(request);

    if (request.get_url() == "/suggest")
     return handle_suggest(request);

    if (request.get_url() == "/random")
      return handle_random(request);

    if (request.get_url() == "/catch/external")
      return handle_captured_external(request);

    return handle_content(request);
  } catch (std::exception& e) {
    fprintf(stderr, "===== Unhandled error : %s\n", e.what());
    return Response::build_500(*this, e.what());
  } catch (...) {
    fprintf(stderr, "===== Unhandled unknown error\n");
    return Response::build_500(*this, "Unknown error");
  }
}

MustacheData InternalServer::get_default_data() const
{
  MustacheData data;
  data.set("root", m_root);
  return data;
}

MustacheData InternalServer::homepage_data() const
{
  auto data = get_default_data();

  MustacheData books{MustacheData::type::list};
  for (auto& bookId: mp_library->filter(kiwix::Filter().local(true).valid(true))) {
    auto& currentBook = mp_library->getBookById(bookId);

    MustacheData book;
    book.set("name", mp_nameMapper->getNameForId(bookId));
    book.set("title", currentBook.getTitle());
    book.set("description", currentBook.getDescription());
    book.set("articleCount", beautifyInteger(currentBook.getArticleCount()));
    book.set("mediaCount", beautifyInteger(currentBook.getMediaCount()));
    books.push_back(book);
  }

  data.set("books", books);
  return data;
}

bool InternalServer::etag_not_needed(const RequestContext& request) const
{
  const std::string url = request.get_url();
  return kiwix::startsWith(url, "/catalog")
      || url == "/search"
      || url == "/suggest"
      || url == "/random"
      || url == "/catch/external";
}

ETag
InternalServer::get_matching_if_none_match_etag(const RequestContext& r) const
{
  try {
    const std::string etag_list = r.get_header(MHD_HTTP_HEADER_IF_NONE_MATCH);
    return ETag::match(etag_list, m_server_id);
  } catch (const std::out_of_range&) {
    return ETag();
  }
}

std::unique_ptr<Response> InternalServer::build_homepage(const RequestContext& request)
{
  return ContentResponse::build(*this, RESOURCE::templates::index_html, homepage_data(), "text/html; charset=utf-8");
}

std::unique_ptr<Response> InternalServer::handle_meta(const RequestContext& request)
{
  std::string bookName;
  std::string bookId;
  std::string meta_name;
  std::shared_ptr<Reader> reader;
  try {
    bookName = request.get_argument("content");
    bookId = mp_nameMapper->getIdForName(bookName);
    meta_name = request.get_argument("name");
    reader = mp_library->getReaderById(bookId);
  } catch (const std::out_of_range& e) {
    return Response::build_404(*this, request, bookName);
  }

  if (reader == nullptr) {
    return Response::build_404(*this, request, bookName);
  }

  std::string content;
  std::string mimeType = "text";

  if (meta_name == "title") {
    content = reader->getTitle();
  } else if (meta_name == "description") {
    content = reader->getDescription();
  } else if (meta_name == "language") {
    content = reader->getLanguage();
  } else if (meta_name == "name") {
    content = reader->getName();
  } else if (meta_name == "tags") {
    content = reader->getTags();
  } else if (meta_name == "date") {
    content = reader->getDate();
  } else if (meta_name == "creator") {
    content = reader->getCreator();
  } else if (meta_name == "publisher") {
    content = reader->getPublisher();
  } else if (meta_name == "favicon") {
    reader->getFavicon(content, mimeType);
  } else {
    return Response::build_404(*this, request, bookName);
  }

  auto response = ContentResponse::build(*this, content, mimeType);
  response->set_cacheable();
  return std::move(response);
}

std::unique_ptr<Response> InternalServer::handle_suggest(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_suggest\n");
  }

  std::string content;
  std::string mimeType;
  unsigned int maxSuggestionCount = 10;
  unsigned int suggestionCount = 0;

  std::string bookName;
  std::string bookId;
  std::string term;
  std::shared_ptr<Reader> reader;
  try {
    bookName = request.get_argument("content");
    bookId = mp_nameMapper->getIdForName(bookName);
    term = request.get_argument("term");
    reader = mp_library->getReaderById(bookId);
  } catch (const std::out_of_range&) {
    return Response::build_404(*this, request, bookName);
  }

  if (m_verbose.load()) {
    printf("Searching suggestions for: \"%s\"\n", term.c_str());
  }

  MustacheData results{MustacheData::type::list};

  bool first = true;
  if (reader != nullptr) {
    /* Get the suggestions */
    SuggestionsList_t suggestions;
    reader->searchSuggestionsSmart(term, maxSuggestionCount, suggestions);
    for(auto& suggestion:suggestions) {
      MustacheData result;
      result.set("label", suggestion[0]);
      result.set("value", suggestion[0]);
      result.set("first", first);
      first = false;
      results.push_back(result);
      suggestionCount++;
    }
  }

  /* Propose the fulltext search if possible */
  if (reader->hasFulltextIndex()) {
    MustacheData result;
    result.set("label", "containing '" + term + "'...");
    result.set("value", term + " ");
    result.set("first", first);
    results.push_back(result);
  }

  auto data = get_default_data();
  data.set("suggestions", results);

  auto response = ContentResponse::build(*this, RESOURCE::templates::suggestion_json, data, "application/json; charset=utf-8");
  return std::move(response);
}

std::unique_ptr<Response> InternalServer::handle_skin(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_skin\n");
  }

  auto resourceName = request.get_url().substr(1);
  try {
    auto response = ContentResponse::build(
        *this,
        getResource(resourceName),
        getMimeTypeForFile(resourceName));
    response->set_cacheable();
    return std::move(response);
  } catch (const ResourceNotFound& e) {
    return Response::build_404(*this, request, "");
  }
}

std::unique_ptr<Response> InternalServer::handle_search(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_search\n");
  }

  std::string bookName;
  std::string bookId;
  try {
    bookName = request.get_argument("content");
    bookId = mp_nameMapper->getIdForName(bookName);
  } catch (const std::out_of_range&) {}

  std::string patternString;
  try {
    patternString = request.get_argument("pattern");
  } catch (const std::out_of_range&) {}

  /* Retrive geo search */
  bool has_geo_query = false;
  float latitude = 0;
  float longitude = 0;
  float distance = 0;
  try {
    latitude = request.get_argument<float>("latitude");
    longitude = request.get_argument<float>("longitude");
    distance = request.get_argument<float>("distance");
    has_geo_query = true;
  } catch(const std::out_of_range&) {}
    catch(const std::invalid_argument&) {}

  std::shared_ptr<Reader> reader(nullptr);
  try {
    reader = mp_library->getReaderById(bookId);
  } catch (const std::out_of_range&) {}

  /* Try first to load directly the article */
  if (reader != nullptr && !patternString.empty()) {
    std::string patternCorrespondingUrl;
    auto variants = reader->getTitleVariants(patternString);
    auto variantsItr = variants.begin();

    while (patternCorrespondingUrl.empty() && variantsItr != variants.end()) {
      try {
        auto entry = reader->getEntryFromTitle(*variantsItr);
        entry = entry.getFinalEntry();
        patternCorrespondingUrl = entry.getPath();
        break;
      } catch(kiwix::NoEntry& e) {
        variantsItr++;
      }
    }

    /* If article found then redirect directly to it */
    if (!patternCorrespondingUrl.empty()) {
      auto redirectUrl = m_root + "/" + bookName + "/" + patternCorrespondingUrl;
      return Response::build_redirect(*this, redirectUrl);
    }
  }

  /* Make the search */
  if ( (!reader && !bookName.empty())
    || (patternString.empty() && ! has_geo_query) ) {
    auto data = get_default_data();
    data.set("pattern", encodeDiples(patternString));
    auto response = ContentResponse::build(*this, RESOURCE::templates::no_search_result_html, data, "text/html; charset=utf-8");
    response->set_taskbar(bookName, reader ? reader->getTitle() : "");
    response->set_code(MHD_HTTP_NOT_FOUND);
    return std::move(response);
  }

  Searcher searcher;
  if (reader) {
    searcher.add_reader(reader.get());
  } else {
    for (auto& bookId: mp_library->filter(kiwix::Filter().local(true).valid(true))) {
      auto currentReader = mp_library->getReaderById(bookId);
      if (currentReader) {
        searcher.add_reader(currentReader.get());
      }
    }
  }

  auto start = 0;
  try {
    start = request.get_argument<unsigned int>("start");
  } catch (const std::exception&) {}

  auto pageLength = 25;
  try {
    pageLength = request.get_argument<unsigned int>("pageLength");
  } catch (const std::exception&) {}
  if (pageLength > MAX_SEARCH_LEN) {
    pageLength = MAX_SEARCH_LEN;
  }
  if (pageLength == 0) {
    pageLength = 25;
  }

  auto end = start + pageLength;

  /* Get the results */
  try {
    if (patternString.empty()) {
      searcher.geo_search(latitude, longitude, distance,
                           start, end, m_verbose.load());
    } else {
      searcher.search(patternString,
                       start, end, m_verbose.load());
    }
    SearchRenderer renderer(&searcher, mp_nameMapper);
    renderer.setSearchPattern(patternString);
    renderer.setSearchContent(bookName);
    renderer.setProtocolPrefix(m_root + "/");
    renderer.setSearchProtocolPrefix(m_root + "/search?");
    renderer.setPageLength(pageLength);
    auto response = ContentResponse::build(*this, renderer.getHtml(), "text/html; charset=utf-8");
    response->set_taskbar(bookName, reader ? reader->getTitle() : "");
    //changing status code if no result obtained
    if(searcher.getEstimatedResultCount() == 0)
    {
      response->set_code(MHD_HTTP_NO_CONTENT);
    }

    return std::move(response);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return Response::build_500(*this, e.what());
  }
}

std::unique_ptr<Response> InternalServer::handle_random(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_random\n");
  }

  std::string bookName;
  std::string bookId;
  std::shared_ptr<Reader> reader;
  try {
    bookName = request.get_argument("content");
    bookId = mp_nameMapper->getIdForName(bookName);
    reader = mp_library->getReaderById(bookId);
  } catch (const std::out_of_range&) {
    return Response::build_404(*this, request, bookName);
  }

  if (reader == nullptr) {
    return Response::build_404(*this, request, bookName);
  }

  try {
    auto entry = reader->getRandomPage();
    return build_redirect(bookName, entry.getFinalEntry());
  } catch(kiwix::NoEntry& e) {
    return Response::build_404(*this, request, bookName);
  }
}

std::unique_ptr<Response> InternalServer::handle_captured_external(const RequestContext& request)
{
  std::string source = "";
  try {
    source = kiwix::urlDecode(request.get_argument("source"));
  } catch (const std::out_of_range& e) {}

  if (source.empty())
    return Response::build_404(*this, request, "");

  auto data = get_default_data();
  data.set("source", source);
  return ContentResponse::build(*this, RESOURCE::templates::captured_external_html, data, "text/html; charset=utf-8");
}

std::unique_ptr<Response> InternalServer::handle_catalog(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_catalog");
  }

  std::string host;
  std::string url;
  try {
    host = request.get_header("Host");
    url  = request.get_url_part(1);
  } catch (const std::out_of_range&) {
    return Response::build_404(*this, request, "");
  }

  if (url != "searchdescription.xml" && url != "root.xml" && url != "search") {
    return Response::build_404(*this, request, "");
  }

  if (url == "searchdescription.xml") {
    auto response = ContentResponse::build(*this, RESOURCE::opensearchdescription_xml, get_default_data(), "application/opensearchdescription+xml");
    return std::move(response);
  }

  zim::Uuid uuid;
  kiwix::OPDSDumper opdsDumper;
  opdsDumper.setRootLocation(m_root);
  opdsDumper.setSearchDescriptionUrl("catalog/searchdescription.xml");
  opdsDumper.setLibrary(mp_library);
  std::vector<std::string> bookIdsToDump;
  if (url == "root.xml") {
    opdsDumper.setTitle("All zims");
    uuid = zim::Uuid::generate(host);
    bookIdsToDump = mp_library->filter(kiwix::Filter().valid(true).local(true).remote(true));
  } else if (url == "search") {
    auto filter = kiwix::Filter().valid(true).local(true).remote(true);
    string query("<Empty query>");
    size_t count(10);
    size_t startIndex(0);
    try {
      query = request.get_argument("q");
      filter.query(query);
    } catch (const std::out_of_range&) {}
    try {
      filter.maxSize(extractFromString<unsigned long>(request.get_argument("maxsize")));
    } catch (...) {}
    try {
      filter.name(request.get_argument("name"));
    } catch (const std::out_of_range&) {}
    try {
      filter.lang(request.get_argument("lang"));
    } catch (const std::out_of_range&) {}
    try {
      count = extractFromString<unsigned long>(request.get_argument("count"));
    } catch (...) {}
    try {
      startIndex = extractFromString<unsigned long>(request.get_argument("start"));
    } catch (...) {}
    try {
      filter.acceptTags(kiwix::split(request.get_argument("tag"), ";"));
    } catch (...) {}
    try {
      filter.rejectTags(kiwix::split(request.get_argument("notag"), ";"));
    } catch (...) {}
    opdsDumper.setTitle("Search result for " + query);
    uuid = zim::Uuid::generate();
    bookIdsToDump = mp_library->filter(filter);
    auto totalResults = bookIdsToDump.size();
    bookIdsToDump.erase(bookIdsToDump.begin(), bookIdsToDump.begin()+startIndex);
    if (count>0 && bookIdsToDump.size() > count) {
      bookIdsToDump.resize(count);
    }
    opdsDumper.setOpenSearchInfo(totalResults, startIndex, bookIdsToDump.size());
  }

  opdsDumper.setId(kiwix::to_string(uuid));
  auto response = ContentResponse::build(
      *this,
      opdsDumper.dumpOPDSFeed(bookIdsToDump),
      "application/atom+xml; profile=opds-catalog; kind=acquisition; charset=utf-8");
  return std::move(response);
}

namespace
{

std::string get_book_name(const RequestContext& request)
{
  try {
    return request.get_url_part(0);
  } catch (const std::out_of_range& e) {
    return std::string();
  }
}

} // unnamed namespace

std::shared_ptr<Reader>
InternalServer::get_reader(const std::string& bookName) const
{
  std::shared_ptr<Reader> reader;
  try {
    const std::string bookId = mp_nameMapper->getIdForName(bookName);
    reader = mp_library->getReaderById(bookId);
  } catch (const std::out_of_range& e) {
  }
  return reader;
}

std::unique_ptr<Response>
InternalServer::build_redirect(const std::string& bookName, const kiwix::Entry& entry) const
{
  auto redirectUrl = m_root + "/" + bookName + "/" + kiwix::urlEncode(entry.getPath());
  return Response::build_redirect(*this, redirectUrl);
}

std::unique_ptr<Response> InternalServer::handle_content(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_content\n");
  }

  const std::string bookName = get_book_name(request);
  if (bookName.empty())
    return build_homepage(request);

  const std::shared_ptr<Reader> reader = get_reader(bookName);
  if (reader == nullptr) {
    return Response::build_404(*this, request, bookName);
  }

  auto urlStr = request.get_url().substr(bookName.size()+1);
  if (urlStr[0] == '/') {
    urlStr = urlStr.substr(1);
  }

  try {
    auto entry = reader->getEntryFromPath(urlStr);
    if (entry.isRedirect() || urlStr.empty()) {
      // If urlStr is empty, we want to mainPage.
      // We must do a redirection to the real page.
      return build_redirect(bookName, entry.getFinalEntry());
    }
    auto response = ItemResponse::build(*this, request, entry.getZimEntry().getItem());
    try {
      dynamic_cast<ContentResponse&>(*response).set_taskbar(bookName, reader->getTitle());
    } catch (std::bad_cast& e) {}

    if (m_verbose.load()) {
      printf("Found %s\n", entry.getPath().c_str());
      printf("mimeType: %s\n", entry.getMimetype().c_str());
    }

    return response;
  } catch(kiwix::NoEntry& e) {
    if (m_verbose.load())
      printf("Failed to find %s\n", urlStr.c_str());

    return Response::build_404(*this, request, bookName);
  }
}

}
