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

Server::Server(LibraryPtr library, std::shared_ptr<NameMapper> nameMapper) :
  mp_library(library),
  mp_nameMapper(nameMapper),
  mp_server(nullptr)
{
}

Server::~Server() = default;

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
    m_ipv6,
    m_indexTemplateString,
    m_ipConnectionLimit));
  return mp_server->start();
}

void Server::stop() {
  if (mp_server) {
    mp_server->stop();
    mp_server.reset(nullptr);
  }
}

void Server::setRoot(const std::string& root)
{
  m_root = root;
  if (m_root[0] != '/') {
    m_root = "/" + m_root;
  }
  if (m_root.back() == '/') {
    m_root.erase(m_root.size() - 1);
  }
}

int Server::getPort()
{
  return mp_server->getPort();
}

std::string Server::getAddress()
{
  return mp_server->getAddress();
}

bool Server::isAddressIPv6()
{
  return mp_server->isAddressIPv6();
}

}
