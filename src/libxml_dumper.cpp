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

#include "libxml_dumper.h"
#include "book.h"

#include "tools/otherTools.h"

namespace kiwix
{
/* Constructor */
LibXMLDumper::LibXMLDumper(const Library* library)
  : library(library)
{
}
/* Destructor */
LibXMLDumper::~LibXMLDumper()
{
}

#define ADD_TEXT_ENTRY(node, child, value) (node).append_child((child)).append_child(pugi::node_pcdata).set_value((value).c_str())

void LibXMLDumper::handleBookmark(Bookmark bookmark, pugi::xml_node root_node) {

  auto entry_node = root_node.append_child("bookmark");
  auto book_node = entry_node.append_child("book");

  try {
    auto book = library->getBookByIdThreadSafe(bookmark.getBookId());
    ADD_TEXT_ENTRY(book_node, "id", book.getId());
    ADD_TEXT_ENTRY(book_node, "title", book.getTitle());
    ADD_TEXT_ENTRY(book_node, "name", book.getName());
    ADD_TEXT_ENTRY(book_node, "flavour", book.getFlavour());
    ADD_TEXT_ENTRY(book_node, "language", book.getCommaSeparatedLanguages());
    ADD_TEXT_ENTRY(book_node, "date", book.getDate());
  } catch (...) {
    ADD_TEXT_ENTRY(book_node, "id", bookmark.getBookId());
    ADD_TEXT_ENTRY(book_node, "title", bookmark.getBookTitle());
    ADD_TEXT_ENTRY(book_node, "name", bookmark.getBookName());
    ADD_TEXT_ENTRY(book_node, "flavour", bookmark.getBookFlavour());
    ADD_TEXT_ENTRY(book_node, "language", bookmark.getLanguage());
    ADD_TEXT_ENTRY(book_node, "date", bookmark.getDate());
  }
  ADD_TEXT_ENTRY(entry_node, "title", bookmark.getTitle());
  ADD_TEXT_ENTRY(entry_node, "url", bookmark.getUrl());
}


std::string LibXMLDumper::dumpLibXMLBookmark()
{
  pugi::xml_document doc;

  /* Add the library node */
  pugi::xml_node bookmarksNode = doc.append_child("bookmarks");

  if (library) {
    for (auto& bookmark: library->getBookmarks(false)) {
      handleBookmark(bookmark, bookmarksNode);
    }
  }
  return nodeToString(bookmarksNode);
}

}
