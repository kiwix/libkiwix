/*
 * Copyright 2019 Matthieu Gautier<mgautier@kymeria.fr>
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


#ifndef KIWIXLIB_SERVER_RESPONSE_H
#define KIWIXLIB_SERVER_RESPONSE_H

#include <string>
#include <map>

#include <mustache.hpp>
#include "byte_range.h"
#include "entry.h"
#include "etag.h"

extern "C" {
#include "microhttpd_wrapper.h"
}

namespace zim {
class Archive;
} // namespace zim

namespace kiwix {

class InternalServer;
class RequestContext;

class ContentResponse;

class Response {
  public:
    Response(bool verbose);
    virtual ~Response() = default;

    static std::unique_ptr<Response> build(const InternalServer& server);
    static std::unique_ptr<Response> build_304(const InternalServer& server, const ETag& etag);
    static std::unique_ptr<ContentResponse> build_404(const InternalServer& server, const std::string& url, const std::string& bookName, const std::string& bookTitle, const std::string& details="");
    static std::unique_ptr<Response> build_416(const InternalServer& server, size_t resourceLength);
    static std::unique_ptr<Response> build_500(const InternalServer& server, const std::string& msg);
    static std::unique_ptr<Response> build_redirect(const InternalServer& server, const std::string& redirectUrl);

    MHD_Result send(const RequestContext& request, MHD_Connection* connection);

    void set_code(int code) { m_returnCode = code; }
    void set_cacheable() { m_etag.set_option(ETag::CACHEABLE_ENTITY); }
    void set_server_id(const std::string& id) { m_etag.set_server_id(id); }
    void add_header(const std::string& name, const std::string& value) { m_customHeaders[name] = value; }

    int getReturnCode() const { return m_returnCode; }

  private: // functions
    virtual MHD_Response* create_mhd_response(const RequestContext& request);
    MHD_Response* create_error_response(const RequestContext& request) const;

  protected: // data
    bool m_verbose;
    int m_returnCode;
    ByteRange m_byteRange;
    ETag m_etag;
    std::map<std::string, std::string> m_customHeaders;

    friend class ItemResponse;
};


class ContentResponse : public Response {
  public:
    ContentResponse(
      const std::string& root,
      bool verbose,
      bool raw,
      bool withTaskbar,
      bool withLibraryButton,
      bool blockExternalLinks,
      const std::string& content,
      const std::string& mimetype);
    static std::unique_ptr<ContentResponse> build(
      const InternalServer& server,
      const std::string& content,
      const std::string& mimetype,
      bool isHomePage = false,
      bool raw = false);
    static std::unique_ptr<ContentResponse> build(
      const InternalServer& server,
      const std::string& template_str,
      kainjow::mustache::data data,
      const std::string& mimetype,
      bool isHomePage = false);

    void set_taskbar(const std::string& bookName, const std::string& bookTitle);

  private:
    MHD_Response* create_mhd_response(const RequestContext& request);

    void introduce_taskbar();
    void inject_externallinks_blocker();
    void inject_root_link();
    bool can_compress(const RequestContext& request) const;
    bool contentDecorationAllowed() const;


  private:
    std::string m_root;
    std::string m_content;
    std::string m_mimeType;
    bool m_raw;
    bool m_withTaskbar;
    bool m_withLibraryButton;
    bool m_blockExternalLinks;
    std::string m_bookName;
    std::string m_bookTitle;
 };

std::unique_ptr<ContentResponse> withTaskbarInfo(const std::string& bookName,
                                                 const zim::Archive* archive,
                                                 std::unique_ptr<ContentResponse> r);

class ItemResponse : public Response {
  public:
    ItemResponse(bool verbose, const zim::Item& item, const std::string& mimetype, const ByteRange& byterange);
    static std::unique_ptr<Response> build(const InternalServer& server, const RequestContext& request, const zim::Item& item, bool raw = false);

  private:
    MHD_Response* create_mhd_response(const RequestContext& request);

    zim::Item m_item;
    std::string m_mimeType;
};

}

#endif //KIWIXLIB_SERVER_RESPONSE_H
