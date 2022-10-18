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
#include "library.h"
#include "name_mapper.h"

#include "libkiwix-resources.h"
#include <mustache.hpp>

#include "tools/stringTools.h"
#include "tools/otherTools.h"

namespace kiwix
{

/* Constructor */
OPDSDumper::OPDSDumper(Server::Configuration configuration)
  : m_configuration(configuration)
{
}
/* Destructor */
OPDSDumper::~OPDSDumper()
{
}

void OPDSDumper::setOpenSearchInfo(int totalResults, int startIndex, int count)
{
  m_totalResults = totalResults;
  m_startIndex = startIndex,
  m_count = count;
}

namespace
{

const std::string XML_HEADER(R"(<?xml version="1.0" encoding="UTF-8"?>)");

typedef kainjow::mustache::data MustacheData;
typedef kainjow::mustache::list BooksData;
typedef kainjow::mustache::list IllustrationInfo;

IllustrationInfo getBookIllustrationInfo(const Book& book)
{
    kainjow::mustache::list illustrations;
    if ( book.isPathValid() ) {
      for ( const auto& illustration : book.getIllustrations() ) {
        // For now, we are handling only sizexsize@1 illustration.
        // So we can simply pass one size to mustache.
        illustrations.push_back(kainjow::mustache::object{
          {"icon_size", to_string(illustration->width)},
          {"icon_mimetype", illustration->mimeType}
        });
      }
    }
    return illustrations;
}

std::string fullEntryXML(const Server::Configuration& configuration, const Book& book)
{
    const auto bookDate = book.getDate() + "T00:00:00Z";
    const kainjow::mustache::object data{
      {"root", configuration.m_root},
      {"id", book.getId()},
      {"name", book.getName()},
      {"title", book.getTitle()},
      {"description", book.getDescription()},
      {"language", book.getLanguage()},
      {"content_id",  urlEncode(configuration.mp_nameMapper->getNameForId(book.getId()), true)},
      {"updated", bookDate}, // XXX: this should be the entry update datetime
      {"book_date", bookDate},
      {"category", book.getCategory()},
      {"flavour", book.getFlavour()},
      {"tags", book.getTags()},
      {"article_count", to_string(book.getArticleCount())},
      {"media_count", to_string(book.getMediaCount())},
      {"author_name", book.getCreator()},
      {"publisher_name", book.getPublisher()},
      {"url", onlyAsNonEmptyMustacheValue(book.getUrl())},
      {"size", to_string(book.getSize())},
      {"icons", getBookIllustrationInfo(book)},
    };
    return render_template(RESOURCE::templates::catalog_v2_entry_xml, data);
}

std::string partialEntryXML(const Server::Configuration& configuration, const Book& book)
{
    const auto bookDate = book.getDate() + "T00:00:00Z";
    const kainjow::mustache::object data{
      {"root",  configuration.m_root},
      {"endpoint_root", configuration.m_root + "/catalog/v2"},
      {"id", book.getId()},
      {"title", book.getTitle()},
      {"updated", bookDate}, // XXX: this should be the entry update datetime
    };
    const auto xmlTemplate = RESOURCE::templates::catalog_v2_partial_entry_xml;
    return render_template(xmlTemplate, data);
}

BooksData getBooksData(const Server::Configuration& configuration, const std::vector<std::string>& bookIds, bool partial)
{
  BooksData booksData;
  for ( const auto& bookId : bookIds ) {
    try {
      const Book book = configuration.mp_library->getBookByIdThreadSafe(bookId);
      const auto entryXML = partial
                          ? partialEntryXML(configuration, book)
                          : fullEntryXML(configuration, book);
      booksData.push_back(kainjow::mustache::object{ {"entry", entryXML} });
    } catch ( const std::out_of_range& ) {
      // the book was removed from the library since its id was obtained
      // ignore it
    }
  }

  return booksData;
}

std::map<std::string, std::string> iso639_3 = {
  {"atj", "atikamekw"},
  {"azb", "آذربایجان دیلی"},
  {"bcl", "central bikol"},
  {"bgs", "tagabawa"},
  {"bxr", "буряад хэлэн"},
  {"cbk", "chavacano"},
  {"cdo", "閩東語"},
  {"dag", "Dagbani"},
  {"diq", "dimli"},
  {"dty", "डोटेली"},
  {"eml", "emiliân-rumagnōl"},
  {"fbs", "српскохрватски"},
  {"ido", "ido"},
  {"kbp", "kabɩyɛ"},
  {"kld", "Gamilaraay"},
  {"lbe", "лакку маз"},
  {"lbj", "ལ་དྭགས་སྐད་"},
  {"map", "Austronesian"},
  {"mhr", "марий йылме"},
  {"mnw", "ဘာသာမန်"},
  {"myn", "mayan"},
  {"nah", "nahuatl"},
  {"nai", "north American Indian"},
  {"nds", "plattdütsch"},
  {"nrm", "bhasa narom"},
  {"olo", "livvi"},
  {"pih", "Pitcairn-Norfolk"},
  {"pnb", "Western Panjabi"},
  {"rmr", "Caló"},
  {"rmy", "romani shib"},
  {"roa", "romance languages"},
  {"twi", "twi"}
};

std::once_flag fillLanguagesFlag;

void fillLanguagesMap()
{
  for (auto icuLangPtr = icu::Locale::getISOLanguages(); *icuLangPtr != NULL; ++icuLangPtr) {
    const ICULanguageInfo lang(*icuLangPtr);
    iso639_3.insert({lang.iso3Code(), lang.selfName()});
  }
}

std::string getLanguageSelfName(const std::string& lang) {
  const auto itr = iso639_3.find(lang);
  if (itr != iso639_3.end()) {
    return itr->second;
  }
  return lang;
};

} // unnamed namespace

string OPDSDumper::dumpOPDSFeed(const std::vector<std::string>& bookIds, const std::string& query) const
{
  const auto booksData = getBooksData(m_configuration, bookIds, false);
  const kainjow::mustache::object template_data{
     {"date", gen_date_str()},
     {"root", m_configuration.m_root},
     {"feed_id", gen_uuid(libraryId + "/catalog/search?"+query)},
     {"filter", onlyAsNonEmptyMustacheValue(query)},
     {"totalResults", to_string(m_totalResults)},
     {"startIndex", to_string(m_startIndex)},
     {"itemsPerPage", to_string(m_count)},
     {"books", booksData }
  };

  return render_template(RESOURCE::templates::catalog_entries_xml, template_data);
}

string OPDSDumper::dumpOPDSFeedV2(const std::vector<std::string>& bookIds, const std::string& query, bool partial) const
{
  const auto endpointRoot = m_configuration.m_root + "/catalog/v2";
  const auto booksData = getBooksData(m_configuration, bookIds, partial);

  const char* const endpoint = partial ? "/partial_entries" : "/entries";
  const kainjow::mustache::object template_data{
     {"date", gen_date_str()},
     {"endpoint_root", endpointRoot},
     {"feed_id", gen_uuid(libraryId + endpoint + "?" + query)},
     {"filter", onlyAsNonEmptyMustacheValue(query)},
     {"query", query.empty() ? "" : "?" + urlEncode(query)},
     {"totalResults", to_string(m_totalResults)},
     {"startIndex", to_string(m_startIndex)},
     {"itemsPerPage", to_string(m_count)},
     {"books", booksData },
     {"dump_partial_entries", MustacheData(partial)}
  };

  return render_template(RESOURCE::templates::catalog_v2_entries_xml, template_data);
}

std::string OPDSDumper::dumpOPDSCompleteEntry(const std::string& bookId) const
{
  const auto book = m_configuration.mp_library->getBookById(bookId);
  return XML_HEADER
         + "\n"
         + fullEntryXML(m_configuration, book);
}

std::string OPDSDumper::categoriesOPDSFeed() const
{
  const auto now = gen_date_str();
  kainjow::mustache::list categoryData;
  for ( const auto& category : m_configuration.mp_library->getBooksCategories() ) {
    const auto urlencodedCategoryName = urlEncode(category);
    categoryData.push_back(kainjow::mustache::object{
      {"name", category},
      {"urlencoded_name",  urlencodedCategoryName},
      {"updated", now},
      {"id", gen_uuid(libraryId + "/categories/" + urlencodedCategoryName)}
    });
  }

  return render_template(
             RESOURCE::templates::catalog_v2_categories_xml,
             kainjow::mustache::object{
               {"date", now},
               {"endpoint_root", m_configuration.m_root + "/catalog/v2"},
               {"feed_id", gen_uuid(libraryId + "/categories")},
               {"categories", categoryData }
             }
  );
}

std::string OPDSDumper::languagesOPDSFeed() const
{
  const auto now = gen_date_str();
  kainjow::mustache::list languageData;
  std::call_once(fillLanguagesFlag, fillLanguagesMap);
  for ( const auto& langAndBookCount : m_configuration.mp_library->getBooksLanguagesWithCounts() ) {
    const std::string languageCode = langAndBookCount.first;
    const int bookCount = langAndBookCount.second;
    const auto languageSelfName = getLanguageSelfName(languageCode);
    languageData.push_back(kainjow::mustache::object{
      {"lang_code",  languageCode},
      {"lang_self_name", languageSelfName},
      {"book_count", to_string(bookCount)},
      {"updated", now},
      {"id", gen_uuid(libraryId + "/languages/" + languageCode)}
    });
  }

  return render_template(
             RESOURCE::templates::catalog_v2_languages_xml,
             kainjow::mustache::object{
               {"date", now},
               {"endpoint_root", m_configuration.m_root + "/catalog/v2"},
               {"feed_id", gen_uuid(libraryId + "/languages")},
               {"languages", languageData }
             }
  );
}

}
