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

#include "opds_dumper.h"


#include <common/otherTools.h>

namespace kiwix
{
/* Constructor */
OPDSDumper::OPDSDumper(Library library)
  : library(library)
{
}
/* Destructor */
OPDSDumper::~OPDSDumper()
{
}

std::string gen_date_str()
{
  auto now = time(0);
  auto tm = localtime(&now);

  std::stringstream is;
  is << std::setw(2) << std::setfill('0')
     << 1900+tm->tm_year << "-"
     << std::setw(2) << std::setfill('0') << tm->tm_mon << "-"
     << std::setw(2) << std::setfill('0') << tm->tm_mday << "T"
     << std::setw(2) << std::setfill('0') << tm->tm_hour << ":"
     << std::setw(2) << std::setfill('0') << tm->tm_min << ":"
     << std::setw(2) << std::setfill('0') << tm->tm_sec << "Z";
  return is.str();
}

#define ADD_TEXT_ENTRY(node, child, value) (node).append_child((child)).append_child(pugi::node_pcdata).set_value((value).c_str())

pugi::xml_node OPDSDumper::handleBook(Book book, pugi::xml_node root_node) {
  auto entry_node = root_node.append_child("entry");
  ADD_TEXT_ENTRY(entry_node, "title", book.title);
  ADD_TEXT_ENTRY(entry_node, "id", "urn:uuid:"+book.id);
  ADD_TEXT_ENTRY(entry_node, "icon", rootLocation + "/meta?name=favicon&content=" + book.getHumanReadableIdFromPath());
  ADD_TEXT_ENTRY(entry_node, "updated", date);
  ADD_TEXT_ENTRY(entry_node, "summary", book.description);

  auto content_node = entry_node.append_child("link");
  content_node.append_attribute("type") = "text/html";
  content_node.append_attribute("href") = (rootLocation + "/" + book.getHumanReadableIdFromPath()).c_str();

  auto author_node = entry_node.append_child("author");
  ADD_TEXT_ENTRY(author_node, "name", book.creator);

  if (! book.url.empty()) {
    auto acquisition_link = entry_node.append_child("link");
    acquisition_link.append_attribute("rel") = "http://opds-spec.org/acquisition/open-access";
    acquisition_link.append_attribute("type") = "application/x-zim";
    acquisition_link.append_attribute("href") = book.url.c_str();
  }

  if (! book.faviconMimeType.empty() ) {
    auto image_link = entry_node.append_child("link");
    image_link.append_attribute("rel") = "http://opds-spec.org/image/thumbnail";
    image_link.append_attribute("type") = book.faviconMimeType.c_str();
    image_link.append_attribute("href") = (rootLocation + "/meta?name=favicon&content=" + book.getHumanReadableIdFromPath()).c_str();
  }
  return entry_node;
}

string OPDSDumper::dumpOPDSFeed()
{
  date = gen_date_str();
  pugi::xml_document doc;

  auto root_node = doc.append_child("feed");
  root_node.append_attribute("xmlns") = "http://www.w3.org/2005/Atom";
  root_node.append_attribute("xmlns:opds") = "http://opds-spec.org/2010/catalog";

  ADD_TEXT_ENTRY(root_node, "id", id);

  ADD_TEXT_ENTRY(root_node, "title", title);
  ADD_TEXT_ENTRY(root_node, "updated", date);

  auto self_link_node = root_node.append_child("link");
  self_link_node.append_attribute("rel") = "self";
  self_link_node.append_attribute("href") = "";
  self_link_node.append_attribute("type") = "application/atom+xml";


  if (!searchDescriptionUrl.empty() ) {
    auto search_link = root_node.append_child("link");
    search_link.append_attribute("rel") = "search";
    search_link.append_attribute("type") = "application/opensearchdescription+xml";
    search_link.append_attribute("href") = searchDescriptionUrl.c_str();
  }

  for (auto book: library.books) {
    handleBook(book, root_node);
  }

  return nodeToString(root_node);
}

}
