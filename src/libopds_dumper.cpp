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

#include <sstream>
#include "libopds_dumper.h"
#include "book.h"
#include "library_dumper.h"

#include "libkiwix-resources.h"
#include <mustache.hpp>

#include "tools/stringTools.h"
#include "tools/otherTools.h"

namespace kiwix
{

/* Constructor */
LibOPDSDumper::LibOPDSDumper(const Library* library)
  : library(library)
{
}
/* Destructor */
LibOPDSDumper::~LibOPDSDumper()
{
}

std::string LibOPDSDumper::handleBook(const Book& book) const
{
  // Local/offline dump: the book's own id is used as the content id, and
  // there is no content-access URL to link to (no server involved).
  return fullEntryOpds(book, /*rootLocation=*/"", /*contentAccessUrl=*/"", /*contentId=*/book.getId());
}

void LibOPDSDumper::dumpOPDSContent(const std::vector<std::string>& bookIds, std::ostream& os)
{
  kainjow::mustache::list booksData;

  if (library) {
    for (auto& bookId: bookIds) {
      try {
        const Book book = library->getBookByIdThreadSafe(bookId);
        booksData.push_back(kainjow::mustache::object{ {"entry", handleBook(book)} });
      } catch (const std::out_of_range&) {
        // the book was removed from the library since its id was obtained
        // ignore it
      }
    }
  }

  const kainjow::mustache::object template_data{
       {"date", gen_date_str()},
       {"root", ""},
       {"feed_id", gen_uuid("/catalog")},
       {"books", booksData}
  };

  // Render to string and stream it into the provided os reference
  os << render_template(RESOURCE::templates::catalog_entries_xml, template_data);
}

}
