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

#ifndef KIWIXLIB_SERVER_INTERNALSERVER_H
#define KIWIXLIB_SERVER_INTERNALSERVER_H


extern "C" {
#include "microhttpd_wrapper.h"
}

#include "library.h"
#include "name_mapper.h"

#include <zim/search.h>
#include <zim/suggestion.h>

#include <mustache.hpp>

#include <atomic>
#include <string>

#include "server/request_context.h"
#include "server/response.h"

#include "tools/concurrent_cache.h"

namespace kiwix {

struct GeoQuery {
  GeoQuery()
    : GeoQuery(0, 0, -1)
  {}

  GeoQuery(float latitude, float longitude, float distance)
    : latitude(latitude), longitude(longitude), distance(distance)
  {}
  float latitude;
  float longitude;
  float distance;

  explicit operator bool() const {
    return distance >= 0;
  }

  friend bool operator<(const GeoQuery& l, const GeoQuery& r)
  {
      return std::tie(l.latitude, l.longitude, l.distance)
           < std::tie(r.latitude, r.longitude, r.distance); // keep the same order
  }
};

class SearchInfo {
  public:
    SearchInfo(const std::string& pattern);
    SearchInfo(const std::string& pattern, GeoQuery geoQuery);
    SearchInfo(const RequestContext& request);

    zim::Query getZimQuery(bool verbose) const;

    friend bool operator<(const SearchInfo& l, const SearchInfo& r)
    {
        return std::tie(l.bookName, l.pattern, l.geoQuery)
             < std::tie(r.bookName, r.pattern, r.geoQuery); // keep the same order
    }

  public: //data
    std::string pattern;
    GeoQuery geoQuery;
    std::string bookName;
};


typedef kainjow::mustache::data MustacheData;
typedef ConcurrentCache<string, std::shared_ptr<zim::Searcher>> SearcherCache;
typedef ConcurrentCache<SearchInfo, std::shared_ptr<zim::Search>> SearchCache;
typedef ConcurrentCache<string, std::shared_ptr<zim::SuggestionSearcher>> SuggestionSearcherCache;

class Entry;
class OPDSDumper;

class InternalServer {
  public:
    InternalServer(Library* library,
                   NameMapper* nameMapper,
                   std::string addr,
                   int port,
                   std::string root,
                   int nbThreads,
                   bool verbose,
                   bool withTaskbar,
                   bool withLibraryButton,
                   bool blockExternalLinks,
                   std::string indexTemplateString,
                   int ipConnectionLimit);
    virtual ~InternalServer() = default;

    MHD_Result handlerCallback(struct MHD_Connection* connection,
                               const char* url,
                               const char* method,
                               const char* version,
                               const char* upload_data,
                               size_t* upload_data_size,
                               void** cont_cls);
    bool start();
    void stop();
    std::string getAddress() { return m_addr; }
    int getPort() { return m_port; }

  private: // functions
    std::unique_ptr<Response> handle_request(const RequestContext& request);
    std::unique_ptr<Response> build_redirect(const std::string& bookName, const zim::Item& item) const;
    std::unique_ptr<Response> build_homepage(const RequestContext& request);
    std::unique_ptr<Response> handle_skin(const RequestContext& request);
    std::unique_ptr<Response> handle_catalog(const RequestContext& request);
    std::unique_ptr<Response> handle_catalog_v2(const RequestContext& request);
    std::unique_ptr<Response> handle_catalog_v2_root(const RequestContext& request);
    std::unique_ptr<Response> handle_catalog_v2_entries(const RequestContext& request, bool partial);
    std::unique_ptr<Response> handle_catalog_v2_complete_entry(const RequestContext& request, const std::string& entryId);
    std::unique_ptr<Response> handle_catalog_v2_categories(const RequestContext& request);
    std::unique_ptr<Response> handle_catalog_v2_languages(const RequestContext& request);
    std::unique_ptr<Response> handle_catalog_v2_illustration(const RequestContext& request);
    std::unique_ptr<Response> handle_search(const RequestContext& request);
    std::unique_ptr<Response> handle_suggest(const RequestContext& request);
    std::unique_ptr<Response> handle_random(const RequestContext& request);
    std::unique_ptr<Response> handle_captured_external(const RequestContext& request);
    std::unique_ptr<Response> handle_content(const RequestContext& request);
    std::unique_ptr<Response> handle_raw(const RequestContext& request);

    std::vector<std::string> search_catalog(const RequestContext& request,
                                            kiwix::OPDSDumper& opdsDumper);

    MustacheData get_default_data() const;

    bool etag_not_needed(const RequestContext& r) const;
    ETag get_matching_if_none_match_etag(const RequestContext& request) const;

  private: // data
    std::string m_addr;
    int m_port;
    std::string m_root;
    int m_nbThreads;
    std::atomic_bool m_verbose;
    bool m_withTaskbar;
    bool m_withLibraryButton;
    bool m_blockExternalLinks;
    std::string m_indexTemplateString;
    int m_ipConnectionLimit;
    struct MHD_Daemon* mp_daemon;

    Library* mp_library;
    NameMapper* mp_nameMapper;

    SearcherCache searcherCache;
    SearchCache searchCache;
    SuggestionSearcherCache suggestionSearcherCache;

    std::string m_server_id;
    std::string m_library_id;

    friend std::unique_ptr<Response> Response::build(const InternalServer& server);
    friend std::unique_ptr<ContentResponse> ContentResponse::build(const InternalServer& server, const std::string& content, const std::string& mimetype, bool isHomePage, bool raw);
    friend std::unique_ptr<Response> ItemResponse::build(const InternalServer& server, const RequestContext& request, const zim::Item& item, bool raw);
    friend std::unique_ptr<Response> Response::build_500(const InternalServer& server, const std::string& msg);

};

}

#endif //KIWIXLIB_SERVER_INTERNALSERVER_H
