/*
 * Copyright 2018 Matthieu Gautier <mgautier@kymeria.fr> TODO
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

#ifndef KIWIX_LIBOPDS_DUMPER_H
#define KIWIX_LIBOPDS_DUMPER_H

#include <string>
#include <vector>

#include "library.h"

namespace kiwix
{

/**
 * A tool to dump a `Library` into a basic OPDS feed.
 *
 */
class LibOPDSDumper
{
 public:
  LibOPDSDumper() = default;
  LibOPDSDumper(const Library* library);
  ~LibOPDSDumper();

  /**
   * Dump the OPDS feed.
   *
   * @param bookIds the ids of the books to include in the feed.
   * @return The OPDS feed content.
   */
  std::string dumpOPDSContent(const std::vector<std::string>& bookIds);

  /**
   * Set the root location used when generating urls.
   *
   * @param rootLocation the root location to use.
   */
  void setRootLocation(const std::string& rootLocation) { this->rootLocation = rootLocation; }

  /**
   * Set the library to dump.
   *
   * @param library The library to dump.
   */
  void setLibrary(const Library* library) { this->library = library; }

 protected:
  const kiwix::Library* library;
  std::string rootLocation;
 private:
  std::string handleBook(const Book& book) const;
};
}

#endif // KIWIX_LIBOPDS_DUMPER_H
