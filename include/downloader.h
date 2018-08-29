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

#ifndef KIWIX_DOWNLOADER_H
#define KIWIX_DOWNLOADER_H

#include <string>
#include <pthread.h>
#include <memory>

namespace kiwix
{

class Aria2;
struct DownloadedFile {
  DownloadedFile()
   : success(false) {}
  bool success;
  std::string path;
};

/**
 * A tool to download things.
 *
 */
class Downloader
{
 public:
  Downloader();
  ~Downloader();

  /**
   * Download a content.
   *
   * @param url the url to download
   * @return the content downloaded.
   */
  DownloadedFile download(const std::string& url);

 private:

  std::unique_ptr<Aria2> mp_aria;
};
}

#endif
