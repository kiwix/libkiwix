/*
 * Copyright 2017 Matthieu Gautier <mgautier@kymeria.fr>
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

#ifndef KIWIX_OPDS_DUMPER_H
#define KIWIX_OPDS_DUMPER_H

#include <time.h>
#include <sstream>
#include <string>

#include <pugixml.hpp>

#include "common/base64.h"
#include "common/pathTools.h"
#include "common/regexTools.h"
#include "library.h"
#include "reader.h"

using namespace std;

namespace kiwix
{

/**
 * A tool to dump a `Library` into a opds stream.
 *
 */
class OPDSDumper
{
 public:
  OPDSDumper() = default;
  OPDSDumper(Library library);
  ~OPDSDumper();

  /**
   * Dump the OPDS feed.
   *
   * @param id The id of the library.
   * @return The OPDS feed.
   */
  std::string dumpOPDSFeed();

  /**
   * Set the id of the opds stream.
   *
   * @param id the id to use.
   */
  void setId(const std::string& id) { this->id = id;}

  /**
   * Set the title oft the opds stream.
   *
   * @param title the title to use.
   */
  void setTitle(const std::string& title) { this->title = title; }

  /**
   * Set the root location used when generating url.
   *
   * @param rootLocation the root location to use.
   */
  void setRootLocation(const std::string& rootLocation) { this->rootLocation = rootLocation; }

  /**
   * Set the search url.
   *
   * @param searchUrl the search url to use.
   */
  void setSearchDescriptionUrl(const std::string& searchDescriptionUrl) { this->searchDescriptionUrl = searchDescriptionUrl; }

  /**
   * Set the library to dump.
   *
   * @param library The library to dump.
   */
  void setLibrary(Library library) { this->library = library; }

 protected:
  kiwix::Library library;
  std::string id;
  std::string title;
  std::string date;
  std::string rootLocation;
  std::string searchDescriptionUrl;

 private:
  pugi::xml_node handleBook(Book book, pugi::xml_node root_node);
};
}

#endif // KIWIX_OPDS_DUMPER_H
