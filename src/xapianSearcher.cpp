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

#include "xapianSearcher.h"
#include <sys/types.h>
#include <unicode/locid.h>
#include <unistd.h>
#include <zim/article.h>
#include <zim/error.h>
#include <zim/file.h>
#include <zim/zim.h>
#include "xapian/myhtmlparse.h"

#include <vector>

namespace kiwix
{
std::map<std::string, int> read_valuesmap(const std::string& s)
{
  std::map<std::string, int> result;
  std::vector<std::string> elems = split(s, ";");
  for (std::vector<std::string>::iterator elem = elems.begin();
       elem != elems.end();
       elem++) {
    std::vector<std::string> tmp_elems = split(*elem, ":");
    result.insert(
        std::pair<std::string, int>(tmp_elems[0], atoi(tmp_elems[1].c_str())));
  }
  return result;
}

/* Constructor */
XapianSearcher::XapianSearcher(const string& xapianDirectoryPath,
                               Reader* reader)
    : reader(reader)
{
  this->openIndex(xapianDirectoryPath);
}

/* Open Xapian readable database */
void XapianSearcher::openIndex(const string& directoryPath)
{
  this->readableDatabase = Xapian::Database(directoryPath);
  this->valuesmap
      = read_valuesmap(this->readableDatabase.get_metadata("valuesmap"));
  this->language = this->readableDatabase.get_metadata("language");
  this->stopwords = this->readableDatabase.get_metadata("stopwords");
  setup_queryParser();
}

/* Close Xapian writable database */
void XapianSearcher::closeIndex()
{
  return;
}
void XapianSearcher::setup_queryParser()
{
  queryParser.set_database(readableDatabase);
  if (!language.empty()) {
    /* Build ICU Local object to retrieve ISO-639 language code (from
       ISO-639-3) */
    icu::Locale languageLocale(language.c_str());

    /* Configuring language base steemming */
    try {
      stemmer = Xapian::Stem(languageLocale.getLanguage());
      queryParser.set_stemmer(stemmer);
      queryParser.set_stemming_strategy(Xapian::QueryParser::STEM_ALL);
    } catch (...) {
      std::cout << "No steemming for language '" << languageLocale.getLanguage()
                << "'" << std::endl;
    }
  }

  if (!stopwords.empty()) {
    std::string stopWord;
    std::istringstream file(this->stopwords);
    while (std::getline(file, stopWord, '\n')) {
      this->stopper.add(stopWord);
    }
    queryParser.set_stopper(&(this->stopper));
  }
}

/* Search strings in the database */
void XapianSearcher::searchInIndex(string& search,
                                   const unsigned int resultStart,
                                   const unsigned int resultEnd,
                                   const bool verbose)
{
  /* Create the query */
  Xapian::Query query = queryParser.parse_query(search);

  /* Create the enquire object */
  Xapian::Enquire enquire(this->readableDatabase);
  enquire.set_query(query);

  /* Get the results */
  this->results = enquire.get_mset(resultStart, resultEnd - resultStart);
  this->current_result = this->results.begin();
}

/* Get next result */
Result* XapianSearcher::getNextResult()
{
  if (this->current_result != this->results.end()) {
    XapianResult* result = new XapianResult(this, this->current_result);
    this->current_result++;
    return result;
  }
  return NULL;
}

void XapianSearcher::restart_search()
{
  this->current_result = this->results.begin();
}

XapianResult::XapianResult(XapianSearcher* searcher,
                           Xapian::MSetIterator& iterator)
    : searcher(searcher), iterator(iterator), document(iterator.get_document())
{
}

std::string XapianResult::get_url()
{
  return document.get_data();
}
std::string XapianResult::get_title()
{
  if (searcher->valuesmap.empty()) {
    /* This is the old legacy version. Guess and try */
    return document.get_value(0);
  } else if (searcher->valuesmap.find("title") != searcher->valuesmap.end()) {
    return document.get_value(searcher->valuesmap["title"]);
  }
  return "";
}

int XapianResult::get_score()
{
  return iterator.get_percent();
}
std::string XapianResult::get_snippet()
{
  if (searcher->valuesmap.empty()) {
    /* This is the old legacy version. Guess and try */
    std::string stored_snippet = document.get_value(1);
    if (!stored_snippet.empty()) {
      return stored_snippet;
    }
    /* Let's continue here, and see if we can genenate one */
  } else if (searcher->valuesmap.find("snippet") != searcher->valuesmap.end()) {
    return document.get_value(searcher->valuesmap["snippet"]);
  }
  /* No reader, no snippet */
  if (!searcher->reader) {
    return "";
  }
  /* Get the content of the article to generate a snippet.
     We parse it and use the html dump to avoid remove html tags in the
     content and be able to nicely cut the text at random place. */
  MyHtmlParser htmlParser;
  std::string content = get_content();
  if (content.empty()) {
    return content;
  }
  try {
    htmlParser.parse_html(content, "UTF-8", true);
  } catch (...) {
  }
  return searcher->results.snippet(htmlParser.dump, 500);
}

std::string XapianResult::get_content()
{
  if (!searcher->reader) {
    return "";
  }
  auto entry = searcher->reader->getEntryFromEncodedPath(get_url());
  return entry.getContent();
}

int XapianResult::get_size()
{
  if (searcher->valuesmap.empty()) {
    /* This is the old legacy version. Guess and try */
    return document.get_value(2).empty() == true
               ? -1
               : atoi(document.get_value(2).c_str());
  } else if (searcher->valuesmap.find("size") != searcher->valuesmap.end()) {
    return atoi(document.get_value(searcher->valuesmap["size"]).c_str());
  }
  /* The size is never used. Do we really want to get the content and
     calculate the size ? */
  return -1;
}

int XapianResult::get_wordCount()
{
  if (searcher->valuesmap.empty()) {
    /* This is the old legacy version. Guess and try */
    return document.get_value(3).empty() == true
               ? -1
               : atoi(document.get_value(3).c_str());
  } else if (searcher->valuesmap.find("wordcount")
             != searcher->valuesmap.end()) {
    return atoi(document.get_value(searcher->valuesmap["wordcount"]).c_str());
  }
  return -1;
}

}  // Kiwix namespace
