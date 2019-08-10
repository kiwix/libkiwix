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
       /**
        * The default constructor.
        *
        * @param library The library to serve.
        */
       Server(Library* library, NameMapper* nameMapper=nullptr);

       virtual ~Server();

       /**
        * Serve the content.
        */
       bool start();

       /**
        * Stop the daemon.
        */
       void stop();

       void set_root(const std::string& root);
       void set_addr(const std::string& addr) { m_addr = addr; }
       void set_port(int port) { m_port = port; }
       void set_nbThreads(int threads) { m_nbThreads = threads; }
       void set_verbose(bool verbose) { m_verbose = verbose; }
       void set_taskbar(bool withTaskbar, bool withLibraryButton)
         { m_withTaskbar = withTaskbar; m_withLibraryButton = withLibraryButton; }

     protected:
       Library* mp_library;
       NameMapper* mp_nameMapper;
       std::string m_root = "";
       std::string m_addr = "";
       int m_port = 80;
       int m_nbThreads = 1;
       bool m_verbose = false;
       bool m_withTaskbar = true;
       bool m_withLibraryButton = true;
       std::unique_ptr<InternalServer> mp_server;
  };
}

#endif
