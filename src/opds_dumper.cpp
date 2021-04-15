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
#include "book.h"

#include "tools/otherTools.h"

namespace kiwix
{
/* Constructor */
OPDSDumper::OPDSDumper(Library* library)
  : library(library)
{
}
/* Destructor */
OPDSDumper::~OPDSDumper()
{
}

static std::string gen_date_from_yyyy_mm_dd(const std::string& date)
{
  std::stringstream is;
  is << date << "T00:00:00Z";
  return is.str();
}

void OPDSDumper::setOpenSearchInfo(int totalResults, int startIndex, int count)
{
  m_totalResults = totalResults;
  m_startIndex = startIndex,
  m_count = count;
  m_isSearchResult = true;
}

#define ADD_TEXT_ENTRY(node, child, value) (node).append_child((child)).append_child(pugi::node_pcdata).set_value((value).c_str())

pugi::xml_node OPDSDumper::handleBook(Book book, pugi::xml_node root_node) {
  auto entry_node = root_node.append_child("entry");
  ADD_TEXT_ENTRY(entry_node, "id", "urn:uuid:"+book.getId());
  ADD_TEXT_ENTRY(entry_node, "title", book.getTitle());
  ADD_TEXT_ENTRY(entry_node, "summary", book.getDescription());
  ADD_TEXT_ENTRY(entry_node, "language", book.getLanguage());
  ADD_TEXT_ENTRY(entry_node, "updated", gen_date_from_yyyy_mm_dd(book.getDate()));
  ADD_TEXT_ENTRY(entry_node, "name", book.getName());
  ADD_TEXT_ENTRY(entry_node, "flavour", book.getFlavour());
  ADD_TEXT_ENTRY(entry_node, "category", book.getCategory());
  ADD_TEXT_ENTRY(entry_node, "tags", book.getTags());
  ADD_TEXT_ENTRY(entry_node, "articleCount", to_string(book.getArticleCount()));
  ADD_TEXT_ENTRY(entry_node, "mediaCount", to_string(book.getMediaCount()));
  ADD_TEXT_ENTRY(entry_node, "icon", rootLocation + "/meta?name=favicon&content=" + book.getHumanReadableIdFromPath());

  auto content_node = entry_node.append_child("link");
  content_node.append_attribute("type") = "text/html";
  content_node.append_attribute("href") = (rootLocation + "/" + book.getHumanReadableIdFromPath()).c_str();

  auto author_node = entry_node.append_child("author");
  ADD_TEXT_ENTRY(author_node, "name", book.getCreator());

  auto publisher_node = entry_node.append_child("publisher");
  ADD_TEXT_ENTRY(publisher_node, "name", book.getPublisher());

  if (! book.getUrl().empty()) {
    auto acquisition_link = entry_node.append_child("link");
    acquisition_link.append_attribute("rel") = "http://opds-spec.org/acquisition/open-access";
    acquisition_link.append_attribute("type") = "application/x-zim";
    acquisition_link.append_attribute("href") = book.getUrl().c_str();
    acquisition_link.append_attribute("length") = to_string(book.getSize()).c_str();
  }

  if (! book.getFaviconMimeType().empty() ) {
    auto image_link = entry_node.append_child("link");
    image_link.append_attribute("rel") = "http://opds-spec.org/image/thumbnail";
    image_link.append_attribute("type") = book.getFaviconMimeType().c_str();
    image_link.append_attribute("href") = (rootLocation + "/meta?name=favicon&content=" + book.getHumanReadableIdFromPath()).c_str();
  }
  return entry_node;
}

string OPDSDumper::dumpOPDSFeed(const std::vector<std::string>& bookIds)
{
  date = gen_date_str();
  pugi::xml_document doc;

  auto root_node = doc.append_child("feed");
  root_node.append_attribute("xmlns") = "http://www.w3.org/2005/Atom";
  root_node.append_attribute("xmlns:opds") = "http://opds-spec.org/2010/catalog";

  ADD_TEXT_ENTRY(root_node, "id", id);

  ADD_TEXT_ENTRY(root_node, "title", title);
  ADD_TEXT_ENTRY(root_node, "updated", date);

  if (m_isSearchResult) {
    ADD_TEXT_ENTRY(root_node, "totalResults", to_string(m_totalResults));
    ADD_TEXT_ENTRY(root_node, "startIndex", to_string(m_startIndex));
    ADD_TEXT_ENTRY(root_node, "itemsPerPage", to_string(m_count));
  }

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

  if (library) {
    for (auto& bookId: bookIds) {
      handleBook(library->getBookById(bookId), root_node);
    }
  }

  return nodeToString(root_node);
}

}
