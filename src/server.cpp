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
#include "tools/otherTools.h"

namespace kiwix {

Server::Configuration::Configuration(std::shared_ptr<Library> library, std::shared_ptr<NameMapper> nameMapper) :
  mp_library(library),
  mp_nameMapper(nameMapper ? nameMapper : std::make_shared<HumanReadableNameMapper>(*library, true))
{}

Server::Configuration& Server::Configuration::setRoot(const std::string& root)
{
  m_root = normalizeRootUrl(root);
  return *this;
}

Server::Server(const Server::Configuration& configuration) :
  m_configuration(configuration),
  mp_server(nullptr)
{
}

Server::~Server() = default;

bool Server::start() {
  mp_server.reset(new InternalServer(m_configuration));
  return mp_server->start();
}

void Server::stop() {
  if (mp_server) {
    mp_server->stop();
    mp_server.reset(nullptr);
  }
}

bool Server::isRunning() {
  if (!mp_server) {
    return false;
  }
  return mp_server->isRunning();
}

int Server::getPort()
{
  return mp_server->getPort();
}

std::string Server::getAddress()
{
  return mp_server->getAddress();
}

}
