/*
 * Copyright 2018 Matthieu Gautier <mgautier@kymeria.fr>
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

#include "downloader.h"
#include "common/pathTools.h"

#ifndef _WIN32
# include <unistd.h>
#endif
#include <iostream>

namespace kiwix
{

pthread_mutex_t Downloader::globalLock = PTHREAD_MUTEX_INITIALIZER;


/* Constructor */
Downloader::Downloader()
{
#ifdef ENABLE_LIBARIA2
  aria2::SessionConfig config;
  config.downloadEventCallback = Downloader::downloadEventCallback;
  config.userData = this;
  tmpDir = makeTmpDirectory();
  aria2::KeyVals options;
  options.push_back(std::pair<std::string, std::string>("dir", tmpDir));
  session = aria2::sessionNew(options, config);
#endif
}


/* Destructor */
Downloader::~Downloader()
{
#ifdef ENABLE_LIBARIA2
  aria2::sessionFinal(session);
#endif
  rmdir(tmpDir.c_str());
}

#ifdef ENABLE_LIBARIA2
int Downloader::downloadEventCallback(aria2::Session* session,
                                      aria2::DownloadEvent event,
                                      aria2::A2Gid gid,
                                      void* userData)
{
  Downloader* downloader = static_cast<Downloader*>(userData);

  auto fileHandle = downloader->fileHandle;
  auto dh = aria2::getDownloadHandle(session, gid);

  if (!dh) {
    return 0;
  }

  switch (event) {
    case aria2::EVENT_ON_DOWNLOAD_COMPLETE:
      {
        if (dh->getNumFiles() > 0) {
          auto f = dh->getFile(1);
          fileHandle->path = f.path;
          fileHandle->success = true;
        }
      }
      break;
    case aria2::EVENT_ON_DOWNLOAD_ERROR:
      {
        fileHandle->success = false;
      }
      break;
    default:
      break;
  }
  aria2::deleteDownloadHandle(dh);
  return 0;
}
#endif

DownloadedFile Downloader::download(const std::string& url) {
  pthread_mutex_lock(&globalLock);
  DownloadedFile fileHandle;
#ifdef ENABLE_LIBARIA2
  try {
    std::vector<std::string> uris = {url};
    aria2::KeyVals options;
    aria2::A2Gid gid;
    int ret;
    DownloadedFile fileHandle;
    
    ret = aria2::addUri(session, &gid, uris, options);
    if (ret < 0) {
      std::cerr << "Failed to download" << std::endl;
    } else {
      this->fileHandle = &fileHandle;
      aria2::run(session, aria2::RUN_DEFAULT);
    }
  } catch (...) {};
  this->fileHandle = nullptr;
  pthread_mutex_unlock(&globalLock);
#endif
  return fileHandle;
}

}
