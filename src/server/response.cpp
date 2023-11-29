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
#include "libkiwix-resources.h"

#include "tools/regexTools.h"
#include "tools/stringTools.h"
#include "tools/otherTools.h"
#include "tools/archiveTools.h"

#include "string.h"
#include <mustache.hpp>
#include <zlib.h>

#include <array>

// This is somehow a magic value.
// If this value is too small, we will compress (and lost cpu time) too much
// content.
// If this value is too big, we will not compress enough content and send too
// much data.
// If we assume that MTU is 1500 Bytes it is useless to compress
// content smaller as the content will be sent in one packet anyway.
// 1400 Bytes seems to be a common accepted limit.
#define KIWIX_MIN_CONTENT_SIZE_TO_COMPRESS 1400

namespace kiwix {

namespace
{
// some utilities

std::string get_mime_type(const zim::Item& item)
{
  try {
    return item.getMimetype();
  } catch (std::exception& e) {
    return "application/octet-stream";
  }
}

bool is_compressible_mime_type(const std::string& mimeType)
{
  return mimeType.find("text/") != std::string::npos
      || mimeType.find("application/javascript") != std::string::npos
      || mimeType.find("application/atom") != std::string::npos
      || mimeType.find("application/opensearchdescription") != std::string::npos
      || mimeType.find("application/json") != std::string::npos

      // Web fonts
      || mimeType.find("application/font-") != std::string::npos
      || mimeType.find("application/x-font-") != std::string::npos
      || mimeType.find("application/vnd.ms-fontobject") != std::string::npos
      || mimeType.find("font/") != std::string::npos;
}

bool compress(std::string &content) {
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  auto ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8,
                          Z_DEFAULT_STRATEGY);
  if (ret != Z_OK) { return false; }

  strm.avail_in = static_cast<decltype(strm.avail_in)>(content.size());
  strm.next_in =
      const_cast<Bytef *>(reinterpret_cast<const Bytef *>(content.data()));

  std::string compressed;

  std::array<char, 16384> buff{};
  do {
    strm.avail_out = buff.size();
    strm.next_out = reinterpret_cast<Bytef *>(buff.data());
    ret = deflate(&strm, Z_FINISH);
    assert(ret != Z_STREAM_ERROR);
    compressed.append(buff.data(), buff.size() - strm.avail_out);
  } while (strm.avail_out == 0);

  assert(ret == Z_STREAM_END);
  assert(strm.avail_in == 0);

  content.swap(compressed);

  deflateEnd(&strm);
  return true;
}


const char* getCacheControlHeader(Response::Kind k)
{
  switch(k) {
    case Response::STATIC_RESOURCE: return "max-age=31536000, immutable";
    case Response::ZIM_CONTENT:     return "max-age=3600, must-revalidate";
    default:                        return "max-age=0, must-revalidate";
  }
}

} // unnamed namespace

Response::Response()
  : m_returnCode(MHD_HTTP_OK)
{
  add_header(MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
}

void Response::set_kind(Kind k)
{
  m_kind = k;
  if ( k == ZIM_CONTENT )
    m_etag.set_option(ETag::ZIM_CONTENT);
}

std::unique_ptr<Response> Response::build()
{
  return std::unique_ptr<Response>(new Response());
}

std::unique_ptr<Response> Response::build_304(const ETag& etag)
{
  auto response = Response::build();
  response->set_code(MHD_HTTP_NOT_MODIFIED);
  response->m_etag = etag;
  if ( etag.get_option(ETag::ZIM_CONTENT) ) {
    response->set_kind(Response::ZIM_CONTENT);
  }
  if ( etag.get_option(ETag::COMPRESSED_CONTENT) ) {
    response->add_header(MHD_HTTP_HEADER_VARY, "Accept-Encoding");
  }
  return response;
}

std::string ContentResponseBlueprint::getMessage(const std::string& msgId) const
{
  return getTranslatedString(m_request.get_user_language(), msgId);
}

std::unique_ptr<ContentResponse> ContentResponseBlueprint::generateResponseObject() const
{
  auto r = ContentResponse::build(m_root, m_template, m_data, m_mimeType);
  r->set_code(m_httpStatusCode);
  return r;
}

HTTPErrorResponse::HTTPErrorResponse(const std::string& root,
                                     const RequestContext& request,
                                     int httpStatusCode,
                                     const std::string& pageTitleMsgId,
                                     const std::string& headingMsgId,
                                     const std::string& cssUrl)
  : ContentResponseBlueprint(root,
                             &request,
                             httpStatusCode,
                             request.get_requested_format() == "html" ? "text/html; charset=utf-8" : "application/xml; charset=utf-8",
                             request.get_requested_format() == "html" ? RESOURCE::templates::error_html : RESOURCE::templates::error_xml)
{
  kainjow::mustache::list emptyList;
  this->m_data = kainjow::mustache::object{
                    {"CSS_URL", onlyAsNonEmptyMustacheValue(cssUrl) },
                    {"PAGE_TITLE",   getMessage(pageTitleMsgId)},
                    {"PAGE_HEADING", getMessage(headingMsgId)},
                    {"details", emptyList}
  };
}

HTTP404Response::HTTP404Response(const std::string& root,
                                 const RequestContext& request)
  : HTTPErrorResponse(root,
                      request,
                      MHD_HTTP_NOT_FOUND,
                      "404-page-title",
                      "404-page-heading")
{
}

UrlNotFoundResponse::UrlNotFoundResponse(const std::string& root,
                                         const RequestContext& request)
    : HTTP404Response(root, request)
{
  const std::string requestUrl = urlDecode(m_request.get_full_url(), false);
  *this += ParameterizedMessage("url-not-found", {{"url", requestUrl}});
}

HTTPErrorResponse& HTTPErrorResponse::operator+(const ParameterizedMessage& details)
{
  const std::string msg = details.getText(m_request.get_user_language());
  m_data["details"].push_back({"p", msg});
  return *this;
}

HTTPErrorResponse& HTTPErrorResponse::operator+=(const ParameterizedMessage& details)
{
  // operator+() is already a state-modifying operator (akin to operator+=)
  return *this + details;
}


HTTP400Response::HTTP400Response(const std::string& root,
                                 const RequestContext& request)
  : HTTPErrorResponse(root,
                      request,
                      MHD_HTTP_BAD_REQUEST,
                      "400-page-title",
                      "400-page-heading")
{
  std::string requestUrl = urlDecode(m_request.get_full_url(), false);
  const auto query = m_request.get_query();
  if (!query.empty()) {
    requestUrl += "?" + encodeDiples(query);
  }
  *this += ParameterizedMessage("invalid-request", {{"url", requestUrl}});
}

HTTP500Response::HTTP500Response(const std::string& root,
                                 const RequestContext& request)
  : HTTPErrorResponse(root,
                      request,
                      MHD_HTTP_INTERNAL_SERVER_ERROR,
                      "500-page-title",
                      "500-page-heading")
{
  *this += nonParameterizedMessage("500-page-text");
}

std::unique_ptr<ContentResponse> HTTP500Response::generateResponseObject() const
{
  const std::string mimeType = "text/html;charset=utf-8";
  auto r = ContentResponse::build(m_root, m_template, m_data, mimeType);
  r->set_code(m_httpStatusCode);
  return r;
}

std::unique_ptr<Response> Response::build_416(size_t resourceLength)
{
  auto response = Response::build();
// [FIXME] (compile with recent enough version of libmicrohttpd)
//  response->set_code(MHD_HTTP_RANGE_NOT_SATISFIABLE);
  response->set_code(416);
  std::ostringstream oss;
  oss << "bytes */" << resourceLength;
  response->add_header(MHD_HTTP_HEADER_CONTENT_RANGE, oss.str());

  return response;
}


std::unique_ptr<Response> Response::build_redirect(const std::string& redirectUrl)
{
  auto response = Response::build();
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

  size_t max_size_to_set = std::min<size_t>(
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


bool
ContentResponse::can_compress(const RequestContext& request) const
{
  return request.can_compress()
      && is_compressible_mime_type(m_mimeType)
      && (m_content.size() > KIWIX_MIN_CONTENT_SIZE_TO_COMPRESS);
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
  const bool isCompressed = can_compress(request) && compress(m_content);

  MHD_Response* response = MHD_create_response_from_buffer(
    m_content.size(), const_cast<char*>(m_content.data()), MHD_RESPMEM_MUST_COPY);

  if (isCompressed) {
    m_etag.set_option(ETag::COMPRESSED_CONTENT);
    MHD_add_response_header(
        response, MHD_HTTP_HEADER_VARY, "Accept-Encoding");
    MHD_add_response_header(
        response, MHD_HTTP_HEADER_CONTENT_ENCODING, "gzip");
  }
  return response;
}

MHD_Result Response::send(const RequestContext& request, bool verbose, MHD_Connection* connection)
{
  MHD_Response* response = create_mhd_response(request);

  MHD_add_response_header(response, MHD_HTTP_HEADER_CACHE_CONTROL,
                          getCacheControlHeader(m_kind));
  const std::string etag = m_etag.get_etag();
  if ( ! etag.empty() )
    MHD_add_response_header(response, MHD_HTTP_HEADER_ETAG, etag.c_str());
  for(auto& p: m_customHeaders) {
    MHD_add_response_header(response, p.first.c_str(), p.second.c_str());
  }

  if (m_returnCode == MHD_HTTP_OK && m_byteRange.kind() == ByteRange::RESOLVED_PARTIAL_CONTENT)
    m_returnCode = MHD_HTTP_PARTIAL_CONTENT;

  if (verbose)
    print_response_info(m_returnCode, response);

  auto ret = MHD_queue_response(connection, m_returnCode, response);
  MHD_destroy_response(response);
  return ret;
}

ContentResponse::ContentResponse(const std::string& content, const std::string& mimetype) :
  Response(),
  m_content(content),
  m_mimeType(mimetype)
{
  add_header(MHD_HTTP_HEADER_CONTENT_TYPE, m_mimeType);
}

std::unique_ptr<ContentResponse> ContentResponse::build(
  const std::string& content,
  const std::string& mimetype)
{
   return std::unique_ptr<ContentResponse>(new ContentResponse(
        content,
        mimetype));
}

std::unique_ptr<ContentResponse> ContentResponse::build(
  const std::string& root,
  const std::string& template_str,
  kainjow::mustache::data data,
  const std::string& mimetype)
{
  auto content = render_template(template_str, data);
  return ContentResponse::build(content, mimetype);
}

ItemResponse::ItemResponse(const zim::Item& item, const std::string& mimetype, const ByteRange& byterange) :
  Response(),
  m_item(item),
  m_mimeType(mimetype)
{
  m_byteRange = byterange;
  set_kind(Response::ZIM_CONTENT);
  add_header(MHD_HTTP_HEADER_CONTENT_TYPE, m_mimeType);
}

std::unique_ptr<Response> ItemResponse::build(const std::string& root, const RequestContext& request, const zim::Item& item)
{
  const std::string mimetype = get_mime_type(item);
  auto byteRange = request.get_range().resolve(item.getSize());
  const bool noRange = byteRange.kind() == ByteRange::RESOLVED_FULL_CONTENT;
  if (noRange && is_compressible_mime_type(mimetype)) {
    // Return a contentResponse
    auto response = ContentResponse::build(item.getData(), mimetype);
    response->set_kind(Response::ZIM_CONTENT);
    response->m_byteRange = byteRange;
    return std::move(response);
  }

  if (byteRange.kind() == ByteRange::RESOLVED_UNSATISFIABLE) {
    auto response = Response::build_416(item.getSize());
    response->set_kind(Response::ZIM_CONTENT);
    return response;
  }

  return std::unique_ptr<Response>(new ItemResponse(
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
