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

#ifndef KIWIX_XAPIAN_INDEXER_H
#define KIWIX_XAPIAN_INDEXER_H

#include <xapian.h>
#include "indexer.h"

using namespace std;

namespace kiwix {
  
  class XapianIndexer : public Indexer {
    
  public:
    XapianIndexer();
    
  protected:
    void indexingPrelude(const string &indexPath);
    void index(const string &url, 
	       const string &title, 
	       const string &unaccentedTitle,
	       const string &keywords, 
	       const string &content,
	       const string &snippet,
	       const string &size,
	       const string &wordCount);
    void flush();
    void indexingPostlude();
    
    Xapian::WritableDatabase writableDatabase;
    Xapian::Stem stemmer;
    Xapian::SimpleStopper stopper;
    Xapian::TermGenerator indexer;
  };

}

#endif
