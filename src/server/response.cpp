


#include "response.h"
#include "request_context.h"
#include "kiwixlib-resources.h"

#include "tools/regexTools.h"
#include "tools/stringTools.h"

#include "string.h"
#include <mustache.hpp>
#include <zlib.h>


#define KIWIX_MIN_CONTENT_SIZE_TO_DEFLATE 100

namespace kiwix {

namespace
{
// some utilities

std::string get_mime_type(const kiwix::Entry& entry)
{
  try {
    return entry.getMimetype();
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

int get_range_len(const kiwix::Entry& entry, RequestContext::ByteRange range)
{
  const int entrySize = entry.getSize();
  return range.second == -1
       ? entrySize - range.first
       : std::min(range.second + 1, entrySize) - range.first;
}

} // unnamed namespace

Response::Response(const std::string& root, bool verbose, bool withTaskbar, bool withLibraryButton, bool blockExternalLinks)
  : m_verbose(verbose),
    m_root(root),
    m_content(""),
    m_mimeType(""),
    m_returnCode(MHD_HTTP_OK),
    m_withTaskbar(withTaskbar),
    m_withLibraryButton(withLibraryButton),
    m_blockExternalLinks(blockExternalLinks),
    m_addTaskbar(false),
    m_bookName(""),
    m_startRange(0),
    m_lenRange(0)
{
}


static int print_key_value (void *cls, enum MHD_ValueKind kind,
                            const char *key, const char *value)
{
  printf (" - %s: '%s'\n", key, value);
  return MHD_YES;
}


struct RunningResponse {
   kiwix::Entry entry;
   int range_start;

   RunningResponse(kiwix::Entry entry,
                   int range_start) :
     entry(entry),
     range_start(range_start)
   {}
};

static ssize_t callback_reader_from_entry(void* cls,
                                  uint64_t pos,
                                  char* buf,
                                  size_t max)
{
  RunningResponse* response = static_cast<RunningResponse*>(cls);

  size_t max_size_to_set = min<size_t>(
    max,
    response->entry.getSize() - pos - response->range_start);

  if (max_size_to_set <= 0) {
    return MHD_CONTENT_READER_END_WITH_ERROR;
  }

  zim::Blob blob = response->entry.getBlob(response->range_start+pos, max_size_to_set);
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


std::string render_template(const std::string& template_str, kainjow::mustache::data data)
{
  kainjow::mustache::mustache tmpl(template_str);
  kainjow::mustache::data urlencode{kainjow::mustache::lambda2{
                               [](const std::string& str,const kainjow::mustache::renderer& r) { return urlEncode(r(str), true); }}};
  data.set("urlencoded", urlencode);
  std::stringstream ss;
  tmpl.render(data, [&ss](const std::string& str) { ss << str; });
  return ss.str();
}


void Response::introduce_taskbar()
{
  if (! m_withTaskbar)
    // Taskbar is globally disabled.
    return;
  kainjow::mustache::data data;
  data.set("root", m_root);
  data.set("content", m_bookName);
  data.set("hascontent", !m_bookName.empty());
  data.set("title", m_bookTitle);
  data.set("withlibrarybutton", m_withLibraryButton);
  auto head_content = render_template(RESOURCE::templates::head_part_html, data);
  m_content = appendToFirstOccurence(
    m_content,
    "<head>",
    head_content);

  auto taskbar_part = render_template(RESOURCE::templates::taskbar_part_html, data);
  m_content = appendToFirstOccurence(
    m_content,
    "<body[^>]*>",
    taskbar_part);
}


void Response::inject_externallinks_blocker()
{
  kainjow::mustache::data data;
  data.set("root", m_root);
  auto script_tag = render_template(RESOURCE::templates::external_blocker_part_html, data);
  m_content = appendToFirstOccurence(
    m_content,
    "<head>",
    script_tag);
}

bool
Response::can_compress(const RequestContext& request) const
{
  return request.can_compress()
      && is_compressible_mime_type(m_mimeType)
      && (m_content.size() > KIWIX_MIN_CONTENT_SIZE_TO_DEFLATE);
}

MHD_Response*
Response::create_raw_content_mhd_response(const RequestContext& request)
{
  if (m_addTaskbar) {
    introduce_taskbar();
  }
  if ( m_blockExternalLinks ) {
    inject_externallinks_blocker();
  }

  bool shouldCompress = m_compress && can_compress(request);
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

  // At shis point m_etag.get_option(ETag::COMPRESSED_CONTENT) and
  // shouldCompress can have different values. This can happen for a 304 (Not
  // Modified) response generated while handling a conditional If-None-Match
  // request. In that case the m_etag (together with its COMPRESSED_CONTENT
  // option) is obtained from the ETag list of the If-None-Match header and the
  // response has no body (which shouldn't be compressed).
  if ( m_etag.get_option(ETag::COMPRESSED_CONTENT) ) {
    MHD_add_response_header(
        response, MHD_HTTP_HEADER_VARY, "Accept-Encoding");
  }
  if (shouldCompress) {
    MHD_add_response_header(
        response, MHD_HTTP_HEADER_CONTENT_ENCODING, "deflate");
  }
  MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, m_mimeType.c_str());
  return response;
}

MHD_Response*
Response::create_redirection_mhd_response() const
{
  MHD_Response* response = MHD_create_response_from_buffer(0, nullptr, MHD_RESPMEM_MUST_COPY);
  MHD_add_response_header(response, MHD_HTTP_HEADER_LOCATION, m_content.c_str());
  return response;
}

MHD_Response*
Response::create_entry_mhd_response() const
{
  MHD_Response* response = MHD_create_response_from_callback(m_lenRange,
                                               16384,
                                               callback_reader_from_entry,
                                               new RunningResponse(m_entry, m_startRange),
                                               callback_free_response);
  MHD_add_response_header(response,
    MHD_HTTP_HEADER_CONTENT_TYPE, m_mimeType.c_str());
  MHD_add_response_header(response, MHD_HTTP_HEADER_ACCEPT_RANGES, "bytes");
  std::ostringstream oss;
  oss << "bytes " << m_startRange << "-" << m_startRange + m_lenRange - 1
      << "/" << m_entry.getSize();

  MHD_add_response_header(response,
    MHD_HTTP_HEADER_CONTENT_RANGE, oss.str().c_str());

  MHD_add_response_header(response,
    MHD_HTTP_HEADER_CONTENT_LENGTH, kiwix::to_string(m_lenRange).c_str());
  return response;
}

MHD_Response*
Response::create_mhd_response(const RequestContext& request)
{
  switch (m_mode) {
    case ResponseMode::RAW_CONTENT :
      return create_raw_content_mhd_response(request);

    case ResponseMode::REDIRECTION :
      return create_redirection_mhd_response();

    case ResponseMode::ENTRY :
      return create_entry_mhd_response();
  }
  return nullptr;
}

int Response::send(const RequestContext& request, MHD_Connection* connection)
{
  MHD_Response* response = create_mhd_response(request);

  MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
  MHD_add_response_header(response, MHD_HTTP_HEADER_CACHE_CONTROL,
    m_etag.get_option(ETag::CACHEABLE_ENTITY) ? "max-age=2723040, public" : "no-cache, no-store, must-revalidate");
  const std::string etag = m_etag.get_etag();
  if ( ! etag.empty() )
      MHD_add_response_header(response, MHD_HTTP_HEADER_ETAG, etag.c_str());

  if (m_returnCode == MHD_HTTP_OK && request.has_range())
    m_returnCode = MHD_HTTP_PARTIAL_CONTENT;

  if (m_verbose)
    print_response_info(m_returnCode, response);

  auto ret = MHD_queue_response(connection, m_returnCode, response);
  MHD_destroy_response(response);
  return ret;
}

void Response::set_template(const std::string& template_str, kainjow::mustache::data data) {
  set_content(render_template(template_str, data));
}

void Response::set_content(const std::string& content) {
  m_content = content;
  m_mode = ResponseMode::RAW_CONTENT;
}

void Response::set_redirection(const std::string& url) {
  m_content = url;
  m_mode = ResponseMode::REDIRECTION;
  m_returnCode = MHD_HTTP_FOUND;
}

void Response::set_entry(const Entry& entry, const RequestContext& request) {
  m_entry = entry;
  m_mode = ResponseMode::ENTRY;

  const std::string mimeType = get_mime_type(entry);
  set_mimeType(mimeType);
  set_cacheable();

  if ( is_compressible_mime_type(mimeType) ) {
    zim::Blob raw_content = entry.getBlob();
    const std::string content = string(raw_content.data(), raw_content.size());

    set_content(content);
    set_compress(true);
  } else {
    const int range_len = get_range_len(entry, request.get_range());
    set_range_first(request.get_range().first);
    set_range_len(range_len);
  }
}

void Response::set_taskbar(const std::string& bookName, const std::string& bookTitle)
{
  m_addTaskbar = true;
  m_bookName = bookName;
  m_bookTitle = bookTitle;
}


}
