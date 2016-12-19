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
    /*
    stemmer(Xapian::Stem("french")) {
    this->indexer.set_stemmer(this->stemmer);
    */ 
  }

  void XapianIndexer::indexingPrelude(const string indexPath) {
    this->writableDatabase = Xapian::WritableDatabase(indexPath+".tmp", Xapian::DB_CREATE_OR_OVERWRITE | Xapian::DB_BACKEND_GLASS);
    this->writableDatabase.begin_transaction(true);

    /* Insert the stopwords */
    if (!this->stopWords.empty()) {
      std::vector<std::string>::iterator it = this->stopWords.begin();
      for( ; it != this->stopWords.end(); ++it) {
	this->stopper.add(*it);
      }

      this->indexer.set_stopper(&(this->stopper));
    }
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

  void XapianIndexer::indexingPostlude(const string indexPath) {
    this->flush();
    this->writableDatabase.commit_transaction();
#ifdef _WIN32
    this->writableDatabase.close();
#endif
    
    /* Compacting the index */
    Xapian::Compactor compactor;
    try {
      Xapian::Database src;
      src.add_database(Xapian::Database(indexPath+".tmp"));
      src.compact(indexPath, Xapian::Compactor::FULL | Xapian::DBCOMPACT_SINGLE_FILE, 0, compactor);
    } catch (const Xapian::Error &error) {
      cerr << indexPath << ": " << error.get_description() << endl;
      exit(1);
    } catch (const char * msg) {
      cerr << indexPath << ": " << msg << endl;
      exit(1);
    }
  }
}
