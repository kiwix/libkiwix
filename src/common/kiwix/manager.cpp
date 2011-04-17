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

    if (result) {
      pugi::xml_node libraryNode = doc.child("library");
      library.current = libraryNode.attribute("current").value();

      for (pugi::xml_node bookNode = libraryNode.child("book"); bookNode; bookNode = bookNode.next_sibling("book")) {
	kiwix::Book book;
	book.id = bookNode.attribute("id").value();
	book.path = bookNode.attribute("path").value();
	book.last = bookNode.attribute("last").value();
	book.indexPath = bookNode.attribute("indexPath").value();
	book.indexType = bookNode.attribute("indexType").value() == "xapian" ? XAPIAN: CLUCENE;
	library.addBook(book);
      }

    }

    return result;
  }

  kiwix::Library Manager::cloneLibrary() {
    return this->library;
  }

}
