/*
 * Copyright 2012 Emmanuel Engelhart <kelson@kiwix.org>
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

#include <tools/networkTools.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>

#include <sstream>
#include <iostream>
#include <stdexcept>


size_t write_callback_to_iss(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  auto str = static_cast<std::stringstream*>(userdata);
  str->write(ptr, nmemb);
  return nmemb;
}

std::string kiwix::download(const std::string& url) {
  auto curl = curl_easy_init();
  std::stringstream ss;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback_to_iss);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ss);
  auto res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    curl_easy_cleanup(curl);
    throw std::runtime_error("Cannot perform request");
  }
  long response_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  curl_easy_cleanup(curl);
  if (response_code != 200) {
    throw std::runtime_error("Invalid return code from server");
  }
  return ss.str();
}
