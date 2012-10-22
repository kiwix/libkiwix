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

#include "networkTools.h"

std::map<std::string, std::string> kiwix::getNetworkInterfaces() {
  std::map<std::string, std::string> interfaces;

#ifdef _WIN32
  SOCKET sd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
  if (sd == SOCKET_ERROR) {
    std::cerr << "Failed to get a socket. Error " << WSAGetLastError() <<
      std::endl;
    return interfaces;
  }

  INTERFACE_INFO InterfaceList[20];
  unsigned long nBytesReturned;
  if (WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
	       sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR) {
    std::cerr << "Failed calling WSAIoctl: error " << WSAGetLastError() <<
      std::endl;
    return interfaces;
  }
  
  int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);
  std::cout << "There are " << nNumInterfaces << " interfaces:" << std::endl;
  for (int i = 0; i < nNumInterfaces; ++i) {
    std::cout << std::endl;
    
    sockaddr_in *pAddress;
    pAddress = (sockaddr_in *) & (InterfaceList[i].iiAddress);

    /* Add to the map */
    std::string interfaceName = std::string(inet_ntoa(pAddress->sin_addr));
    std::string interfaceIp = std::string(inet_ntoa(pAddress->sin_addr));
    interfaces.insert(std::pair<std::string, std::string>(interfaceName, interfaceIp)); 
  }
#else
  /* Get Network interfaces information */
  char buf[16384];
  struct ifconf ifconf;
  int fd = socket(PF_INET, SOCK_DGRAM, 0); /* Only IPV4 */
  ifconf.ifc_len=sizeof buf;
  ifconf.ifc_buf=buf;
  if(ioctl(fd, SIOCGIFCONF, &ifconf)!=0) {
    perror("ioctl(SIOCGIFCONF)");
    exit(EXIT_FAILURE);
  }

  /* Go through each interface */
  int i;
  size_t len;
  struct ifreq *ifreq;
  ifreq=ifconf.ifc_req;
  for(i=0;i<ifconf.ifc_len;) {
    /* Get the network interface name */
    std::string interfaceName = std::string(ifreq->ifr_name);

    /* Get the network interface ip */
    char host[128];
    getnameinfo(&(ifreq->ifr_addr), sizeof ifreq->ifr_addr, host, sizeof host, 0, 0, NI_NUMERICHOST);
    std::string interfaceIp = std::string(host);

    /* Add to the map */
    interfaces.insert(std::pair<std::string, std::string>(interfaceName, interfaceIp)); 

    /* some systems have ifr_addr.sa_len and adjust the length that                                                      
     * way, but not mine. weird */
#ifndef linux
    len=IFNAMSIZ + ifreq->ifr_addr.sa_len;
#else
    len=sizeof *ifreq;
#endif
    ifreq=(struct ifreq*)((char*)ifreq+len);
    i+=len;
  }
#endif
  return interfaces;
}

std::string kiwix::getBestPublicIp() {
  std::map<std::string, std::string> interfaces = kiwix::getNetworkInterfaces();

#ifndef _WIN32
  if (interfaces.find("eth0") != interfaces.end()) {
    return interfaces.find("eth0")->second;
  } else if (interfaces.find("eth1") != interfaces.end()) {
    return interfaces.find("eth1")->second;
  } else if (interfaces.find("wlan0") != interfaces.end()) {
    return interfaces.find("wlan0")->second;
  } else if (interfaces.find("wlan1") != interfaces.end()) {
    return interfaces.find("wlan1")->second;
  }
#endif

  for(std::map<std::string, std::string>::iterator iter = interfaces.begin(); 
      iter != interfaces.end(); ++iter) {
    std::string interfaceIp = iter->second;
    if (interfaceIp.length() >= 7 && interfaceIp.substr(0, 7) == "192.168")
      return interfaceIp;
  }

  for(std::map<std::string, std::string>::iterator iter = interfaces.begin(); 
      iter != interfaces.end(); ++iter) {
    std::string interfaceIp = iter->second;
    if (interfaceIp.length() >= 3 && interfaceIp.substr(0, 3) == "172")
      return interfaceIp;
  }

  return "127.0.0.1";
}
