/*
 * Copyright 2011 Emmanuel Engelhart <kelson@kiwix.org>
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

#include "manager.h"
#include <pugixml.hpp>

namespace kiwix {

  /* Constructor */
  Manager::Manager() {
  }
  
  /* Destructor */
  Manager::~Manager() {
  }

  bool Manager::readFile(const string path) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str());

    return result;
  }

}
