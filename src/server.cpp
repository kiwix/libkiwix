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
  mp_server(new InternalServer(library, nameMapper))
{
}

Server::~Server()
{
  stop();
}

bool Server::start() {
  return mp_server->start();
}

void Server::stop() {
  mp_server->stop();
}

#define ENSURE_SERVER_IS_NOT_RUNNING()   \
  if ( this->isRunning() ) {             \
    throw std::runtime_error("A kiwix::Server::set*() method was called on a running server instance"); \
  } else {}

void Server::setRoot(const std::string& root)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setRoot(root);
}

void Server::setAddress(const std::string& addr)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setAddress(addr);
}

void Server::setPort(int port)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setPort(port);
}

int Server::getPort() const
{
  return mp_server->getPort();
}

IpAddress Server::getAddress() const
{
  return mp_server->getAddress();
}

void Server::setNbThreads(int threads)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setNbThreads(threads);
}

void Server::setMultiZimSearchLimit(unsigned int limit)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setMultiZimSearchLimit(limit);
}

void Server::setIpConnectionLimit(int limit)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setIpConnectionLimit(limit);
}

void Server::setVerbose(bool verbose)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setVerbose(verbose);
}

void Server::setIndexTemplateString(const std::string& indexTemplateString)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setIndexTemplateString(indexTemplateString);
}

void Server::setTaskbar(bool withTaskbar, bool withLibraryButton)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setTaskbar(withTaskbar, withLibraryButton);
}

void Server::setBlockExternalLinks(bool blockExternalLinks)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setBlockExternalLinks(blockExternalLinks);
}

void Server::setCatalogOnlyMode(bool enable)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setCatalogOnlyMode(enable);
}

void Server::setContentServerUrl(std::string url)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setContentServerUrl(url);
}

void Server::setIpMode(IpMode mode)
{
  ENSURE_SERVER_IS_NOT_RUNNING();
  mp_server->setIpMode(mode);
}

IpMode Server::getIpMode() const
{
  return mp_server->getIpMode();
}

std::vector<std::string> Server::getServerAccessUrls() const
{
  std::vector<std::string> result;
  const auto port = getPort();
  const auto root = mp_server->getRoot();
  const IpAddress addr = mp_server->getAddress();
  if (!addr.addr.empty()) {
    result.push_back(makeServerUrl(addr.addr, port, root));
  }
  if (!addr.addr6.empty()) {
    result.push_back(makeServerUrl("[" + addr.addr6 + "]", port, root));
  }
  return result;
}

bool Server::isRunning() const
{
  return mp_server->isRunning();
}

}
