/*
 * Copyright 2026 Hamazasp Avetisyan <hamik.avetisyan@gmail.com>
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
#include <iosfwd>

#include "library.h"

namespace kiwix
{

/**
 * A tool to dump a `Library` into a basic OPDS document, for local/offline
 * use (as opposed to `OPDSDumper`, which serves the live HTTP catalog).
 */
class LibOPDSDumper
{
 public:
  LibOPDSDumper() = default;
  explicit LibOPDSDumper(const Library* library);
  ~LibOPDSDumper();

  /**
   * Dump the library's OPDS content into the provided stream.
   *
   * @param bookIds the ids of the books to include.
   * @param os the output stream to write the OPDS content into.
   */
  void dumpOPDSContent(const std::vector<std::string>& bookIds, std::ostream& os);

  /**
   * Set the library to dump.
   *
   * @param library The library to dump.
   */
  void setLibrary(const Library* library) { this->library = library; }

  /**
   * Set the base directory book paths are resolved relative to when
   * rendered as a book's rel="self" link.
   *
   * @param baseDir the base directory to use.
   */
  void setBaseDir(const std::string& baseDir) { this->baseDir = baseDir; }

 protected:
  const kiwix::Library* library = nullptr;
  std::string baseDir;
 private:
  std::string handleBook(const Book& book) const;
};
}

#endif // KIWIX_LIBOPDS_DUMPER_H
