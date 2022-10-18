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

namespace kiwix
{
  class Library;
  class NameMapper;
  class InternalServer;


  class Server {
    public:
      class Configuration {
        public:
          explicit Configuration(std::shared_ptr<Library> library, std::shared_ptr<NameMapper> nameMapper=nullptr);

          Configuration& setRoot(const std::string& root);
          Configuration& setAddress(const std::string& addr) {
            m_addr = addr;
            return *this;
          }

          Configuration& setPort(int port) {
            m_port = port;
            return *this;
          }

          Configuration& setNbThreads(int threads) {
            m_nbThreads = threads;
            return *this;
          }

          Configuration& setMultiZimSearchLimit(unsigned int limit) {
            m_multizimSearchLimit = limit;
            return *this;
          }

          Configuration& setIpConnectionLimit(int limit) {
            m_ipConnectionLimit = limit;
            return *this;
          }

          Configuration& setVerbose(bool verbose) {
            m_verbose = verbose;
            return *this;
          }

          Configuration& setIndexTemplateString(const std::string& indexTemplateString) {
            m_indexTemplateString = indexTemplateString;
            return *this;
          }

          Configuration& setTaskbar(bool withTaskbar, bool withLibraryButton) {
            m_withTaskbar = withTaskbar;
            m_withLibraryButton = withLibraryButton;
            return *this;
          }

          Configuration& setBlockExternalLinks(bool blockExternalLinks) {
            m_blockExternalLinks = blockExternalLinks;
            return *this;
          }

          std::shared_ptr<Library> mp_library;
          std::shared_ptr<NameMapper> mp_nameMapper;
          std::string m_root = "";
          std::string m_addr = "";
          std::string m_indexTemplateString = "";
          int m_port = 80;
          int m_nbThreads = 1;
          unsigned int m_multizimSearchLimit = 0;
          bool m_verbose = false;
          bool m_withTaskbar = true;
          bool m_withLibraryButton = true;
          bool m_blockExternalLinks = false;
          int m_ipConnectionLimit = 0;
       };

       /**
        * The default constructor.
        *
        * @param library The library to serve.
        */
       explicit Server(const Configuration& configuration);
       virtual ~Server();

       /**
        * Serve the content.
        */
       bool start();

       /**
        * Stop the daemon.
        */
       void stop();

       /**
        * Tell if the server is running or not.
        */
       bool isRunning();

       /**
        * Get the port of the server
        */
       int getPort();

       /**
        * Get the ipAddress of the server
        */
       std::string getAddress();

     protected:
       std::unique_ptr<InternalServer> mp_server;
  };
}

#endif
