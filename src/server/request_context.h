/*
 * Copyright 2009-2016 Emmanuel Engelhart <kelson@kiwix.org>
 * Copyright 2017 Matthieu Gautier<mgautier@kymeria.fr>
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


#ifndef REQUEST_CONTEXT_H
#define REQUEST_CONTEXT_H

#include <string>
#include <sstream>
#include <map>
#include <stdexcept>

#include "byte_range.h"

extern "C" {
#include "microhttpd_wrapper.h"
}

namespace kiwix {

enum class RequestMethod {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE_,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH,
    OTHER
};

class KeyError : public std::runtime_error {};
class IndexError: public std::runtime_error {};


class RequestContext {
  public: // functions
    RequestContext(struct MHD_Connection* connection,
                   std::string rootLocation,
                   const std::string& url,
                   const std::string& method,
                   const std::string& version);
    ~RequestContext();

    void print_debug_info() const;

    bool is_valid_url() const;

    std::string get_header(const std::string& name) const;
    template<typename T=std::string>
    T get_argument(const std::string& name) const {
        std::istringstream stream(arguments.at(name));
        T v;
        stream >> v;
        return v;
    }

    template<class T>
    T get_optional_param(const std::string& name, T default_value) const
    {
      try {
        return get_argument<T>(name);
      } catch (...) {}
      return default_value;
    }


    RequestMethod get_method() const;
    std::string get_url() const;
    std::string get_url_part(int part) const;
    std::string get_full_url() const;
    std::string get_query() const;

    ByteRange get_range() const;

    bool can_compress() const { return acceptEncodingGzip; }

    std::string get_user_language() const;

  private: // data
    std::string full_url;
    std::string url;
    RequestMethod method;
    std::string version;
    unsigned long long requestIndex;

    bool acceptEncodingGzip;

    ByteRange byteRange_;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> arguments;

  private: // functions
    static MHD_Result fill_header(void *, enum MHD_ValueKind, const char*, const char*);
    static MHD_Result fill_argument(void *, enum MHD_ValueKind, const char*, const char*);
};

template<> std::string RequestContext::get_argument(const std::string& name) const;

}

#endif //REQUEST_CONTEXT_H
