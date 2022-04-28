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


#include "request_context.h"
#include <string.h>
#include <stdexcept>
#include <sstream>
#include <cstdio>
#include <atomic>

#include "tools/stringTools.h"

namespace kiwix {

static std::atomic_ullong s_requestIndex(0);

namespace {

RequestMethod str2RequestMethod(const std::string& method) {
  if      (method == "GET")     return RequestMethod::GET;
  else if (method == "HEAD")    return RequestMethod::HEAD;
  else if (method == "POST")    return RequestMethod::POST;
  else if (method == "PUT")     return RequestMethod::PUT;
  else if (method == "DELETE")  return RequestMethod::DELETE_;
  else if (method == "CONNECT") return RequestMethod::CONNECT;
  else if (method == "OPTIONS") return RequestMethod::OPTIONS;
  else if (method == "TRACE")   return RequestMethod::TRACE;
  else if (method == "PATCH")   return RequestMethod::PATCH;
  else                          return RequestMethod::OTHER;
}

std::string
fullURL2LocalURL(const std::string& full_url, const std::string& rootLocation)
{
  if (rootLocation.empty()) {
    // nothing special to handle.
    return full_url;
  } else if (full_url == rootLocation) {
    return "/";
  } else if (full_url.size() > rootLocation.size() &&
             full_url.substr(0, rootLocation.size()+1) == rootLocation + "/") {
    return full_url.substr(rootLocation.size());
  } else {
    return "";
  }
}

} // unnamed namespace

RequestContext::RequestContext(struct MHD_Connection* connection,
                               std::string rootLocation,
                               const std::string& _url,
                               const std::string& _method,
                               const std::string& version) :
  full_url(_url),
  url(fullURL2LocalURL(_url, rootLocation)),
  method(str2RequestMethod(_method)),
  version(version),
  requestIndex(s_requestIndex++),
  acceptEncodingGzip(false),
  byteRange_()
{
  MHD_get_connection_values(connection, MHD_HEADER_KIND, &RequestContext::fill_header, this);
  MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, &RequestContext::fill_argument, this);

  try {
    acceptEncodingGzip =
        (get_header(MHD_HTTP_HEADER_ACCEPT_ENCODING).find("gzip") != std::string::npos);
  } catch (const std::out_of_range&) {}

  try {
    byteRange_ = ByteRange::parse(get_header(MHD_HTTP_HEADER_RANGE));
  } catch (const std::out_of_range&) {}
}

RequestContext::~RequestContext()
{}

MHD_Result RequestContext::fill_header(void *__this, enum MHD_ValueKind kind,
                                       const char *key, const char *value)
{
  RequestContext *_this = static_cast<RequestContext*>(__this);
  _this->headers[lcAll(key)] = value;
  return MHD_YES;
}

MHD_Result RequestContext::fill_argument(void *__this, enum MHD_ValueKind kind,
                                         const char *key, const char* value)
{
  RequestContext *_this = static_cast<RequestContext*>(__this);
  _this->arguments[key] = value == nullptr ? "" : value;
  return MHD_YES;
}

void RequestContext::print_debug_info() const {
  printf("method    : %s (%d)\n", method==RequestMethod::GET ? "GET" :
                                  method==RequestMethod::POST ? "POST" :
                                  "OTHER", (int)method);
  printf("version   : %s\n", version.c_str());
  printf("request#  : %lld\n", requestIndex);
  printf("headers   :\n");
  for (auto it=headers.begin(); it!=headers.end(); it++) {
    printf(" - %s : '%s'\n", it->first.c_str(), it->second.c_str());
  }
  printf("arguments :\n");
  for (auto it=arguments.begin(); it!=arguments.end(); it++) {
    printf(" - %s : '%s'\n", it->first.c_str(), it->second.c_str());
  }
  printf("Parsed : \n");
  printf("full_url: %s\n", full_url.c_str());
  printf("url   : %s\n", url.c_str());
  printf("acceptEncodingGzip : %d\n", acceptEncodingGzip);
  printf("has_range : %d\n", byteRange_.kind() != ByteRange::NONE);
  printf("is_valid_url : %d\n", is_valid_url());
  printf(".............\n");
}


RequestMethod RequestContext::get_method() const {
  return method;
}

std::string RequestContext::get_url() const {
  return url;
}

std::string RequestContext::get_url_part(int number) const {
  size_t start = 1;
  while(true) {
    auto found = url.find('/', start);
    if (number == 0) {
      if (found == std::string::npos) {
        return url.substr(start);
      } else {
        return url.substr(start, found-start);
      }
    } else {
      if (found == std::string::npos) {
        throw std::out_of_range("No parts");
      }
      start = found + 1;
      number -= 1;
    }
  }
}

std::string RequestContext::get_full_url() const {
  return full_url;
}

bool RequestContext::is_valid_url() const {
  return !url.empty();
}

ByteRange RequestContext::get_range() const {
  return byteRange_;
}

template<>
std::string RequestContext::get_argument(const std::string& name) const {
  return arguments.at(name);
}

std::string RequestContext::get_header(const std::string& name) const {
  return headers.at(lcAll(name));
}

std::string RequestContext::get_query() const {
  std::string q;
  const char* sep = "";
  for ( const auto& a : arguments ) {
    q += sep + a.first + '=' + a.second;
    sep = "&";
  }
  return q;
}

std::string RequestContext::get_user_language() const
{
  try {
    return get_argument("userlang");
  } catch(const std::out_of_range&) {}

  try {
    return get_header("Accept-Language");
  } catch(const std::out_of_range&) {}

  return "en";
}

}
