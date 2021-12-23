/*
 * Copyright 2012 Emmanuel Engelhart <kelson@kiwix.org>
 * Copyright 2021 Nikhil Tanwar <2002nikhiltanwar@gmail.com>
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

#include "tools.h"
#include <tools/networkTools.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include <sstream>
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#endif

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

std::map<std::string, std::string> kiwix::getNetworkInterfaces() {
  std::map<std::string, std::string> interfaces;

#ifdef _WIN32
  SOCKET sd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
  if (sd == INVALID_SOCKET) {
    std::cerr << "Failed to get a socket. Error " << WSAGetLastError() << std::endl;
    return interfaces;
  }

  INTERFACE_INFO InterfaceList[20];
  unsigned long nBytesReturned;
  if (WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
      sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR) {
    std::cerr << "Failed calling WSAIoctl: error " << WSAGetLastError() << std::endl;
    return interfaces;
  }

  int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);
  for (int i = 0; i < nNumInterfaces; ++i) {
    sockaddr_in *pAddress;
    pAddress = (sockaddr_in *) & (InterfaceList[i].iiAddress.AddressIn);
    if(pAddress->sin_family == AF_INET) {
    /* Add to the map */
    std::string interfaceName = std::string(inet_ntoa(pAddress->sin_addr));
    interfaces[interfaceName] = interfaceName;
    }
  }
#else
  /* Get Network interfaces information */
  char buf[16384];
  struct ifconf ifconf;
  int fd = socket(PF_INET, SOCK_DGRAM, 0); /* Only IPV4 */
  ifconf.ifc_len = sizeof(buf);
  ifconf.ifc_buf=buf;
  if(ioctl(fd, SIOCGIFCONF, &ifconf)!=0) {
    perror("ioctl(SIOCGIFCONF)");
  }

  /* Go through each interface */
  struct ifreq *ifreq;
  ifreq = ifconf.ifc_req;
  for (int i = 0; i < ifconf.ifc_len; ) {
    if (ifreq->ifr_addr.sa_family == AF_INET) {
      /* Get the network interface ip */
      char host[128] = { 0 };
      const int error = getnameinfo(&(ifreq->ifr_addr), sizeof(ifreq->ifr_addr),
                                    host, sizeof(host),
                                    0, 0, NI_NUMERICHOST);
      if (!error) {
        std::string interfaceName = std::string(ifreq->ifr_name);
        std::string interfaceIp = std::string(host);
        /* Add to the map */
        interfaces[interfaceName] = interfaceIp;
      } else {
        perror("getnameinfo()");
      }
    }

    /* some systems have ifr_addr.sa_len and adjust the length that
     * way, but not mine. weird */
    size_t len;
#ifndef __linux__
    len = IFNAMSIZ + ifreq->ifr_addr.sa_len;
#else
    len = sizeof(*ifreq);
#endif
    ifreq = (struct ifreq*)((char*)ifreq+len);
    i += len;
  }
#endif
  return interfaces;
}

std::string kiwix::getBestPublicIp() {
  auto interfaces = getNetworkInterfaces();

#ifndef _WIN32
  const char* const prioritizedNames[] =
      { "eth0", "eth1", "wlan0", "wlan1", "en0", "en1" };
  for(auto name: prioritizedNames) {
    auto it = interfaces.find(name);
    if(it != interfaces.end()) {
      return it->second;
    }
  }
#endif

  const char* const prefixes[] = { "192.168", "172.16.", "10.0" };
  for(auto prefix : prefixes){
    for(auto& itr : interfaces) {
      auto interfaceIp = itr.second;
      if (interfaceIp.find(prefix) == 0) {
        return interfaceIp;
      }
    }
  }

  return "127.0.0.1";
}
