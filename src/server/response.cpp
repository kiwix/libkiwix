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

#include "response.h"
#include "request_context.h"
#include "internalServer.h"
#include "kiwixlib-resources.h"

#include "tools/regexTools.h"
#include "tools/stringTools.h"
#include "tools/otherTools.h"

#include "string.h"
#include <mustache.hpp>
#include <zlib.h>


#define KIWIX_MIN_CONTENT_SIZE_TO_DEFLATE 100

namespace kiwix {

namespace
{
// some utilities

std::string get_mime_type(const zim::Item& item)
{
  try {
    return item.getMimetype();
  } catch (exception& e) {
    return "application/octet-stream";
  }
}

bool is_compressible_mime_type(const std::string& mimeType)
{
  return mimeType.find("text/") != string::npos
      || mimeType.find("application/javascript") != string::npos
      || mimeType.find("application/atom") != string::npos
      || mimeType.find("application/opensearchdescription") != string::npos
      || mimeType.find("application/json") != string::npos;
}


} // unnamed namespace

Response::Response(bool verbose)
  : m_verbose(verbose),
    m_returnCode(MHD_HTTP_OK)
{
  add_header(MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
}

std::unique_ptr<Response> Response::build(const InternalServer& server)
{
  return std::unique_ptr<Response>(new Response(server.m_verbose.load()));
}

std::unique_ptr<Response> Response::build_304(const InternalServer& server, const ETag& etag)
{
  auto response = Response::build(server);
  response->set_code(MHD_HTTP_NOT_MODIFIED);
  response->m_etag = etag;
  if ( etag.get_option(ETag::COMPRESSED_CONTENT) ) {
    response->add_header(MHD_HTTP_HEADER_VARY, "Accept-Encoding");
  }
  return response;
}

std::unique_ptr<ContentResponse> Response::build_404(const InternalServer& server, const std::string& url, const std::string& bookName, const std::string& bookTitle, const std::string& details)
{
  MustacheData results;
  if ( !url.empty() ) {
    results.set("url", url);
  }
  results.set("details", details);

  auto response = ContentResponse::build(server, RESOURCE::templates::_404_html, results, "text/html");
  response->set_code(MHD_HTTP_NOT_FOUND);
  response->set_taskbar(bookName, bookTitle);

  return response;
}

std::unique_ptr<Response> Response::build_416(const InternalServer& server, size_t resourceLength)
{
  auto response = Response::build(server);
// [FIXME] (compile with recent enough version of libmicrohttpd)
//  response->set_code(MHD_HTTP_RANGE_NOT_SATISFIABLE);
  response->set_code(416);
  std::ostringstream oss;
  oss << "bytes */" << resourceLength;
  response->add_header(MHD_HTTP_HEADER_CONTENT_RANGE, oss.str());

  return response;
}

std::unique_ptr<Response> Response::build_500(const InternalServer& server, const std::string& msg)
{
  MustacheData data;
  data.set("error", msg);
  auto content = render_template(RESOURCE::templates::_500_html, data);
  std::unique_ptr<Response> response (
      new ContentResponse(
        server.m_root, //root
        true, //verbose
        true, //raw
        false, //withTaskbar
        false, //withLibraryButton
        false, //blockExternalLinks
        content, //content
        "text/html" //mimetype
  ));
  response->set_code(MHD_HTTP_INTERNAL_SERVER_ERROR);
  return response;
}


std::unique_ptr<Response> Response::build_redirect(const InternalServer& server, const std::string& redirectUrl)
{
  auto response = Response::build(server);
  response->m_returnCode = MHD_HTTP_FOUND;
  response->add_header(MHD_HTTP_HEADER_LOCATION, redirectUrl);
  return response;
}

static MHD_Result print_key_value (void *cls, enum MHD_ValueKind kind,
                                   const char *key, const char *value)
{
  printf (" - %s: '%s'\n", key, value);
  return MHD_YES;
}


struct RunningResponse {
   zim::Item item;
   int range_start;

   RunningResponse(zim::Item item,
                   int range_start) :
     item(item),
     range_start(range_start)
   {}
};

static ssize_t callback_reader_from_item(void* cls,
                                  uint64_t pos,
                                  char* buf,
                                  size_t max)
{
  RunningResponse* response = static_cast<RunningResponse*>(cls);

  size_t max_size_to_set = min<size_t>(
    max,
    response->item.getSize() - pos - response->range_start);

  if (max_size_to_set <= 0) {
    return MHD_CONTENT_READER_END_WITH_ERROR;
  }

  zim::Blob blob = response->item.getData(response->range_start+pos, max_size_to_set);
  memcpy(buf, blob.data(), max_size_to_set);
  return max_size_to_set;
}

static void callback_free_response(void* cls)
{
  RunningResponse* response = static_cast<RunningResponse*>(cls);
  delete response;
}



void print_response_info(int retCode, MHD_Response* response)
{
  printf("Response :\n");
  printf("httpResponseCode : %d\n", retCode);
  printf("headers :\n");
  MHD_get_response_headers(response, print_key_value, nullptr);
}


void ContentResponse::introduce_taskbar()
{
  kainjow::mustache::data data;
  data.set("root", m_root);
  data.set("content", m_bookName);
  data.set("hascontent", (!m_bookName.empty() && !m_bookTitle.empty()));
  data.set("title", m_bookTitle);
  data.set("withlibrarybutton", m_withLibraryButton);
  auto head_content = render_template(RESOURCE::templates::head_taskbar_html, data);
  m_content = prependToFirstOccurence(
    m_content,
    "</head[ \\t]*>",
    head_content);

  auto taskbar_part = render_template(RESOURCE::templates::taskbar_part_html, data);
  m_content = appendToFirstOccurence(
    m_content,
    "<body[^>]*>",
    taskbar_part);
}


void ContentResponse::inject_externallinks_blocker()
{
  kainjow::mustache::data data;
  data.set("root", m_root);
  auto script_tag = render_template(RESOURCE::templates::external_blocker_part_html, data);
  m_content = prependToFirstOccurence(
    m_content,
    "</head[ \\t]*>",
    script_tag);
}

void ContentResponse::inject_root_link(){
  m_content = prependToFirstOccurence(
    m_content,
    "</head[ \\t]*>",
    "<link type=\"root\" href=\"" + m_root + "\">");
}

bool
ContentResponse::can_compress(const RequestContext& request) const
{
  return request.can_compress()
      && is_compressible_mime_type(m_mimeType)
      && (m_content.size() > KIWIX_MIN_CONTENT_SIZE_TO_DEFLATE);
}

bool
ContentResponse::contentDecorationAllowed() const
{
  if (m_raw) {
    return false;
  }
  return (startsWith(m_mimeType, "text/html")
    && m_mimeType.find(";raw=true") == std::string::npos);
}

MHD_Response*
Response::create_mhd_response(const RequestContext& request)
{
  MHD_Response* response = MHD_create_response_from_buffer(0, nullptr, MHD_RESPMEM_PERSISTENT);
  return response;
}

MHD_Response*
ContentResponse::create_mhd_response(const RequestContext& request)
{
  if (contentDecorationAllowed()) {
    inject_root_link();

    if (m_withTaskbar) {
      introduce_taskbar();
    }
    if (m_blockExternalLinks) {
      inject_externallinks_blocker();
    }
  }

  bool shouldCompress = can_compress(request);
  if (shouldCompress) {
    std::vector<Bytef> compr_buffer(compressBound(m_content.size()));
    uLongf comprLen = compr_buffer.capacity();
    int err = compress(&compr_buffer[0],
                       &comprLen,
                       (const Bytef*)(m_content.data()),
                       m_content.size());
    if (err == Z_OK && comprLen > 2 && comprLen < (m_content.size() + 2)) {
      /* /!\ Internet Explorer has a bug with deflate compression.
         It can not handle the first two bytes (compression headers)
         We need to chunk them off (move the content 2bytes)
         It has no incidence on other browsers
         See http://www.subbu.org/blog/2008/03/ie7-deflate-or-not and comments */
      m_content = string((char*)&compr_buffer[2], comprLen - 2);
      m_etag.set_option(ETag::COMPRESSED_CONTENT);
    } else {
      shouldCompress = false;
    }
  }

  MHD_Response* response = MHD_create_response_from_buffer(
    m_content.size(), const_cast<char*>(m_content.data()), MHD_RESPMEM_MUST_COPY);

  if (shouldCompress) {
    MHD_add_response_header(
        response, MHD_HTTP_HEADER_VARY, "Accept-Encoding");
    MHD_add_response_header(
        response, MHD_HTTP_HEADER_CONTENT_ENCODING, "deflate");
  }
  return response;
}

MHD_Result Response::send(const RequestContext& request, MHD_Connection* connection)
{
  MHD_Response* response = create_mhd_response(request);

  MHD_add_response_header(response, MHD_HTTP_HEADER_CACHE_CONTROL,
    m_etag.get_option(ETag::CACHEABLE_ENTITY) ? "max-age=2723040, public" : "no-cache, no-store, must-revalidate");
  const std::string etag = m_etag.get_etag();
  if ( ! etag.empty() )
    MHD_add_response_header(response, MHD_HTTP_HEADER_ETAG, etag.c_str());
  for(auto& p: m_customHeaders) {
    MHD_add_response_header(response, p.first.c_str(), p.second.c_str());
  }

  if (m_returnCode == MHD_HTTP_OK && m_byteRange.kind() == ByteRange::RESOLVED_PARTIAL_CONTENT)
    m_returnCode = MHD_HTTP_PARTIAL_CONTENT;

  if (m_verbose)
    print_response_info(m_returnCode, response);

  auto ret = MHD_queue_response(connection, m_returnCode, response);
  MHD_destroy_response(response);
  return ret;
}

void ContentResponse::set_taskbar(const std::string& bookName, const std::string& bookTitle)
{
  m_bookName = bookName;
  m_bookTitle = bookTitle;
}


ContentResponse::ContentResponse(const std::string& root, bool verbose, bool raw, bool withTaskbar, bool withLibraryButton, bool blockExternalLinks, const std::string& content, const std::string& mimetype) :
  Response(verbose),
  m_root(root),
  m_content(content),
  m_mimeType(mimetype),
  m_raw(raw),
  m_withTaskbar(withTaskbar),
  m_withLibraryButton(withLibraryButton),
  m_blockExternalLinks(blockExternalLinks),
  m_bookName(""),
  m_bookTitle("")
{
  add_header(MHD_HTTP_HEADER_CONTENT_TYPE, m_mimeType);
}

std::unique_ptr<ContentResponse> ContentResponse::build(
  const InternalServer& server,
  const std::string& content,
  const std::string& mimetype,
  bool isHomePage,
  bool raw)
{
   return std::unique_ptr<ContentResponse>(new ContentResponse(
        server.m_root,
        server.m_verbose.load(),
        raw,
        server.m_withTaskbar && !isHomePage,
        server.m_withLibraryButton,
        server.m_blockExternalLinks,
        content,
        mimetype));
}

std::unique_ptr<ContentResponse> ContentResponse::build(
  const InternalServer& server,
  const std::string& template_str,
  kainjow::mustache::data data,
  const std::string& mimetype,
  bool isHomePage)
{
  auto content = render_template(template_str, data);
  return ContentResponse::build(server, content, mimetype, isHomePage);
}

ItemResponse::ItemResponse(bool verbose, const zim::Item& item, const std::string& mimetype, const ByteRange& byterange) :
  Response(verbose),
  m_item(item),
  m_mimeType(mimetype)
{
  m_byteRange = byterange;
  set_cacheable();
  add_header(MHD_HTTP_HEADER_CONTENT_TYPE, m_mimeType);
}

std::unique_ptr<Response> ItemResponse::build(const InternalServer& server, const RequestContext& request, const zim::Item& item, bool raw)
{
  const std::string mimetype = get_mime_type(item);
  auto byteRange = request.get_range().resolve(item.getSize());
  const bool noRange = byteRange.kind() == ByteRange::RESOLVED_FULL_CONTENT;
  if (noRange && is_compressible_mime_type(mimetype)) {
    // Return a contentResponse
    auto response = ContentResponse::build(server, item.getData(), mimetype, /*isHomePage=*/false, raw);
    response->set_cacheable();
    response->m_byteRange = byteRange;
    return std::move(response);
  }

  if (byteRange.kind() == ByteRange::RESOLVED_UNSATISFIABLE) {
    auto response = Response::build_416(server, item.getSize());
    response->set_cacheable();
    return response;
  }

  return std::unique_ptr<Response>(new ItemResponse(
        server.m_verbose.load(),
        item,
        mimetype,
        byteRange));
}

MHD_Response*
ItemResponse::create_mhd_response(const RequestContext& request)
{
  const auto content_length = m_byteRange.length();
  MHD_Response* response = MHD_create_response_from_callback(content_length,
                                               16384,
                                               callback_reader_from_item,
                                               new RunningResponse(m_item, m_byteRange.first()),
                                               callback_free_response);
  MHD_add_response_header(response, MHD_HTTP_HEADER_ACCEPT_RANGES, "bytes");
  if ( m_byteRange.kind() == ByteRange::RESOLVED_PARTIAL_CONTENT ) {
    std::ostringstream oss;
    oss << "bytes " << m_byteRange.first() << "-" << m_byteRange.last()
        << "/" << m_item.getSize();

    MHD_add_response_header(response,
      MHD_HTTP_HEADER_CONTENT_RANGE, oss.str().c_str());
  }

  MHD_add_response_header(response,
    MHD_HTTP_HEADER_CONTENT_LENGTH, kiwix::to_string(content_length).c_str());
  return response;
}


}
