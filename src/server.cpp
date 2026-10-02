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

#include "server.h"

#include "library.h"
#include "name_mapper.h"

#include <string>

#include <zim/item.h>
#include "server/internalServer.h"

namespace kiwix {

namespace
{

std::string makeServerUrl(std::string host, int port, std::string root)
{
  const int httpDefaultPort = 80;

  if (port == httpDefaultPort) {
    return "http://" + host + root;
  } else {
    return "http://" + host + ":" + std::to_string(port) + root;
  }
}

}  // unnamed namespace

class Server::Impl
{
 public:
  std::shared_ptr<Library> mp_library;
  std::shared_ptr<NameMapper> mp_nameMapper;
  std::string m_root = "";
  IpAddress m_addr;
  std::string m_indexTemplateString = "";
  int m_port = 80;
  int m_nbThreads = 1;
  unsigned int m_multizimSearchLimit = 0;
  bool m_verbose = false;
  bool m_withTaskbar = true;
  bool m_withLibraryButton = true;
  bool m_blockExternalLinks = false;
  IpMode m_ipMode = IpMode::AUTO;
  int m_ipConnectionLimit = 0;
  bool m_catalogOnlyMode = false;
  std::string m_contentServerUrl;
  std::unique_ptr<InternalServer> mp_server;
};

Server::Server(LibraryPtr library, std::shared_ptr<NameMapper> nameMapper) :
  mp_impl(new Impl())
{
  mp_impl->mp_library = library;
  mp_impl->mp_nameMapper = nameMapper;
}

Server::~Server() = default;

bool Server::start() {
  mp_impl->mp_server.reset(new InternalServer(
    mp_impl->mp_library,
    mp_impl->mp_nameMapper,
    mp_impl->m_addr,
    mp_impl->m_port,
    mp_impl->m_root,
    mp_impl->m_nbThreads,
    mp_impl->m_multizimSearchLimit,
    mp_impl->m_verbose,
    mp_impl->m_withTaskbar,
    mp_impl->m_withLibraryButton,
    mp_impl->m_blockExternalLinks,
    mp_impl->m_ipMode,
    mp_impl->m_indexTemplateString,
    mp_impl->m_ipConnectionLimit,
    mp_impl->m_catalogOnlyMode,
    mp_impl->m_contentServerUrl));
  if (mp_impl->mp_server->start()) {
    // this syncs m_addr of InternalServer and Server as they may diverge
    mp_impl->m_addr = mp_impl->mp_server->getAddress();
    return true;
  } else {
    return false;
  }
}

void Server::stop() {
  if (mp_impl->mp_server) {
    mp_impl->mp_server->stop();
    mp_impl->mp_server.reset(nullptr);
  }
}

void Server::setRoot(const std::string& root)
{
  mp_impl->m_root = root;
  while (!mp_impl->m_root.empty() && mp_impl->m_root.back() == '/')
    mp_impl->m_root.pop_back();

  while (!mp_impl->m_root.empty() && mp_impl->m_root.front() == '/')
    mp_impl->m_root = mp_impl->m_root.substr(1);
  mp_impl->m_root = mp_impl->m_root.empty() ? mp_impl->m_root : "/" + mp_impl->m_root;
}

void Server::setAddress(const std::string& addr)
{
  mp_impl->m_addr.addr.clear();
  mp_impl->m_addr.addr6.clear();

  if (addr.empty()) return;

  if (addr.find(':') != std::string::npos) { // IPv6
    mp_impl->m_addr.addr6 = (addr[0] == '[') ? addr.substr(1, addr.length() - 2) : addr; // Remove brackets if any
  } else {
    mp_impl->m_addr.addr = addr;
  }
}

void Server::setPort(int port) { mp_impl->m_port = port; }
void Server::setNbThreads(int threads) { mp_impl->m_nbThreads = threads; }
void Server::setMultiZimSearchLimit(unsigned int limit) { mp_impl->m_multizimSearchLimit = limit; }
void Server::setIpConnectionLimit(int limit) { mp_impl->m_ipConnectionLimit = limit; }
void Server::setVerbose(bool verbose) { mp_impl->m_verbose = verbose; }
void Server::setIndexTemplateString(const std::string& indexTemplateString) { mp_impl->m_indexTemplateString = indexTemplateString; }
void Server::setTaskbar(bool withTaskbar, bool withLibraryButton)
{
  mp_impl->m_withTaskbar = withTaskbar;
  mp_impl->m_withLibraryButton = withLibraryButton;
}
void Server::setBlockExternalLinks(bool blockExternalLinks) { mp_impl->m_blockExternalLinks = blockExternalLinks; }
void Server::setCatalogOnlyMode(bool enable) { mp_impl->m_catalogOnlyMode = enable; }
void Server::setContentServerUrl(std::string url) { mp_impl->m_contentServerUrl = url; }
void Server::setIpMode(IpMode mode) { mp_impl->m_ipMode = mode; }

int Server::getPort() const
{
  return mp_impl->m_port;
}

IpAddress Server::getAddress() const
{
  return mp_impl->m_addr;
}

IpMode Server::getIpMode() const
{
  return mp_impl->mp_server->getIpMode();
}

std::vector<std::string> Server::getServerAccessUrls() const
{
  std::vector<std::string> result;
  if (!mp_impl->m_addr.addr.empty()) {
    result.push_back(makeServerUrl(mp_impl->m_addr.addr, mp_impl->m_port, mp_impl->m_root));
  }
  if (!mp_impl->m_addr.addr6.empty()) {
    result.push_back(makeServerUrl("[" + mp_impl->m_addr.addr6 + "]", mp_impl->m_port, mp_impl->m_root));
  }
  return result;
}

}
