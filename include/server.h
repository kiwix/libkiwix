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

#ifndef KIWIX_SERVER_H
#define KIWIX_SERVER_H

#include <string>
#include <memory>
#include "tools.h"

namespace kiwix
{
  class Library;
  class NameMapper;
  class InternalServer;

  class Server {
     public:
       /**
        * The default constructor.
        *
        * @param library The library to serve.
        */
       Server(std::shared_ptr<Library> library, std::shared_ptr<NameMapper> nameMapper=nullptr);

       virtual ~Server();

       /**
        * Serve the content.
        */
       bool start();

       /**
        * Stop the daemon.
        */
       void stop();

       void setRoot(const std::string& root);
       void setAddress(const std::string& addr);
       void setPort(int port) { m_port = port; }
       void setNbThreads(int threads) { m_nbThreads = threads; }
       void setMultiZimSearchLimit(unsigned int limit) { m_multizimSearchLimit = limit; }
       void setIpConnectionLimit(int limit) { m_ipConnectionLimit = limit; }
       void setVerbose(bool verbose) { m_verbose = verbose; }
       void setIndexTemplateString(const std::string& indexTemplateString) { m_indexTemplateString = indexTemplateString; }
       void setTaskbar(bool withTaskbar, bool withLibraryButton)
        { m_withTaskbar = withTaskbar; m_withLibraryButton = withLibraryButton; }
       void setBlockExternalLinks(bool blockExternalLinks)
        { m_blockExternalLinks = blockExternalLinks; }
       void setCatalogOnlyMode(bool enable) { m_catalogOnlyMode = enable; }
       void setContentServerUrl(std::string url) { m_contentServerUrl = url; }
       void setIpMode(IpMode mode) { m_ipMode = mode; }
       int getPort() const;
       IpAddress getAddress() const;
       IpMode getIpMode() const;

     protected:
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
}

#endif
