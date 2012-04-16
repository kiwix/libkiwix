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

#include "xapianIndexer.h"

namespace kiwix {

  /* Constructor */
  XapianIndexer::XapianIndexer() {
    /* Stemming */
    /*
    stemmer = Xapian::Stem("french");
    indexer.set_stemmer(stemmer);
    */

    /* Stop words */
    /*
    std::vector<std::string>::const_iterator stopWordsIterator = this->stopWords.begin();
    this->stopper.add("ceci");
    while (stopWordsIterator != this->stopWords.end()) {
      this->stopper.add(*stopWordsIterator);
      stopWordsIterator++;
    }
    indexer.set_stopper(&(this->stopper));
    */
  }
  
  void XapianIndexer::indexingPrelude(const string &indexPath) {
    this->writableDatabase = Xapian::WritableDatabase(indexPath, Xapian::DB_CREATE_OR_OVERWRITE);
    this->writableDatabase.begin_transaction(true);
  }
  
  void XapianIndexer::index(const string &url, 
			    const string &title, 
			    const string &unaccentedTitle,
			    const string &keywords, 
			    const string &content,
			    const string &snippet,
			    const string &size,
			    const string &wordCount) {
    
    /* Put the data in the document */
    Xapian::Document currentDocument; 
    currentDocument.clear_values();
    currentDocument.add_value(0, title);
    currentDocument.add_value(1, snippet);
    currentDocument.add_value(2, size);
    currentDocument.add_value(3, wordCount);
    currentDocument.set_data(url);
    indexer.set_document(currentDocument);

    /* Index the title */
    if (!unaccentedTitle.empty()) {
      this->indexer.index_text_without_positions(unaccentedTitle, this->getTitleBoostFactor(content.size()));
    }
    
    /* Index the keywords */
    if (!keywords.empty()) {
      this->indexer.index_text_without_positions(keywords, keywordsBoostFactor);
    }
    
    /* Index the content */
    if (!content.empty()) {
      this->indexer.index_text_without_positions(content);
    }
    
    /* add to the database */
    this->writableDatabase.add_document(currentDocument);
  }
  
  void XapianIndexer::flush() {
    this->writableDatabase.commit_transaction();
    this->writableDatabase.begin_transaction(true);
  }

  void XapianIndexer::indexingPostlude() {
    this->flush();
    this->writableDatabase.commit_transaction();
    
    // commit is not available is old version of xapian and seems not mandatory there
    // this->writableDatabase.commit();
  }
}
