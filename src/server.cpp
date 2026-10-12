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

Server::Server(LibraryPtr library, std::shared_ptr<NameMapper> nameMapper) :
  mp_library(library),
  mp_nameMapper(nameMapper),
  mp_server(nullptr)
{
}

Server::~Server()
{
  stop();
}

bool Server::start() {
  mp_server.reset(new InternalServer(
    mp_library,
    mp_nameMapper,
    m_addr,
    m_port,
    m_root,
    m_nbThreads,
    m_multizimSearchLimit,
    m_verbose,
    m_withTaskbar,
    m_withLibraryButton,
    m_blockExternalLinks,
    m_ipMode,
    m_indexTemplateString,
    m_ipConnectionLimit,
    m_catalogOnlyMode,
    m_contentServerUrl));
  if (mp_server->start()) {
    // this syncs m_addr of InternalServer and Server as they may diverge
    m_addr = mp_server->getAddress();
    return true;
  } else {
    return false;
  }
}

void Server::stop() {
  if (mp_server) {
    mp_server->stop();
    mp_server.reset(nullptr);
  }
}

#define ENSURE_SERVER_IS_NOT_RUNNING()   \
  if ( this->isRunning() ) {             \
    throw std::runtime_error("A kiwix::Server::set*() method was called on a running server instance"); \
  } else {}

void Server::setRoot(const std::string& root)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_root = root;
  while (!m_root.empty() && m_root.back() == '/')
    m_root.pop_back();

  while (!m_root.empty() && m_root.front() == '/')
    m_root = m_root.substr(1);
  m_root = m_root.empty() ? m_root : "/" + m_root;
}

void Server::setAddress(const std::string& addr)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_addr.addr.clear();
  m_addr.addr6.clear();

  if (addr.empty()) return;

  if (addr.find(':') != std::string::npos) { // IPv6
    m_addr.addr6 = (addr[0] == '[') ? addr.substr(1, addr.length() - 2) : addr; // Remove brackets if any
  } else {
    m_addr.addr = addr;
  }
}

void Server::setPort(int port)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_port = port;
}

int Server::getPort() const
{
  return m_port;
}

IpAddress Server::getAddress() const
{
  return m_addr;
}

void Server::setNbThreads(int threads)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_nbThreads = threads;
}

void Server::setMultiZimSearchLimit(unsigned int limit)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_multizimSearchLimit = limit;
}

void Server::setIpConnectionLimit(int limit)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_ipConnectionLimit = limit;
}

void Server::setVerbose(bool verbose)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_verbose = verbose;
}

void Server::setIndexTemplateString(const std::string& indexTemplateString)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_indexTemplateString = indexTemplateString;
}

void Server::setTaskbar(bool withTaskbar, bool withLibraryButton)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_withTaskbar = withTaskbar;
  m_withLibraryButton = withLibraryButton;
}

void Server::setBlockExternalLinks(bool blockExternalLinks)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_blockExternalLinks = blockExternalLinks;
}

void Server::setCatalogOnlyMode(bool enable)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_catalogOnlyMode = enable;
}

void Server::setContentServerUrl(std::string url)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_contentServerUrl = url;
}

void Server::setIpMode(IpMode mode)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  m_ipMode = mode;
}

IpMode Server::getIpMode() const
{
  return mp_server->getIpMode();
}

std::vector<std::string> Server::getServerAccessUrls() const
{
  std::vector<std::string> result;
  if (!m_addr.addr.empty()) {
    result.push_back(makeServerUrl(m_addr.addr, m_port, m_root));
  }
  if (!m_addr.addr6.empty()) {
    result.push_back(makeServerUrl("[" + m_addr.addr6 + "]", m_port, m_root));
  }
  return result;
}

bool Server::isRunning() const
{
  return mp_server && mp_server->isRunning();
}

}
