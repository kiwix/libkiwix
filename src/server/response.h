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

#include <mustache.hpp>
#include "entry.h"

extern "C" {
#include <microhttpd.h>
}

namespace kiwix {

enum class ResponseMode {
  RAW_CONTENT,
  REDIRECTION,
  ENTRY
};

class RequestContext;

class Response {
  public:
    Response(const std::string& root, bool verbose, bool withTaskbar, bool withLibraryButton, bool blockExternalLinks);
    ~Response() = default;

    int send(const RequestContext& request, MHD_Connection* connection);

    void set_template(const std::string& template_str, kainjow::mustache::data data);
    void set_content(const std::string& content);
    void set_redirection(const std::string& url);
    void set_entry(const Entry& entry);


    void set_mimeType(const std::string& mimeType) { m_mimeType = mimeType; }
    void set_code(int code) { m_returnCode = code; }
    void set_cache(bool cache) { m_useCache = cache; }
    void set_compress(bool compress) { m_compress = compress; }
    void set_taskbar(const std::string& bookName, const std::string& bookTitle);
    void set_range_first(uint64_t start) { m_startRange = start; }
    void set_range_len(uint64_t len) { m_lenRange = len; }

    int getReturnCode() { return m_returnCode; }

    void introduce_taskbar();
    void inject_externallinks_blocker();

  private: // functions
    MHD_Response* create_mhd_response(const RequestContext& request);
    MHD_Response* create_raw_content_mhd_response(const RequestContext& request);
    MHD_Response* create_redirection_mhd_response() const;
    MHD_Response* create_entry_mhd_response() const;

  private: // data
    bool m_verbose;
    ResponseMode m_mode;
    std::string m_root;
    std::string m_content;
    Entry m_entry;
    std::string m_mimeType;
    int m_returnCode;
    bool m_withTaskbar;
    bool m_withLibraryButton;
    bool m_blockExternalLinks;
    bool m_useCache;
    bool m_compress;
    bool m_addTaskbar;
    std::string m_bookName;
    std::string m_bookTitle;
    uint64_t m_startRange;
    uint64_t m_lenRange;
};

}

#endif //KIWIXLIB_SERVER_RESPONSE_H
