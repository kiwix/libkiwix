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

#ifndef KIWIX_XAPIAN_SEARCHER_H
#define KIWIX_XAPIAN_SEARCHER_H

#include <xapian.h>
#include "searcher.h"
#include <map>
#include <string>

using namespace std;

namespace kiwix {

  class XapianSearcher;

  class XapianResult : public Result {
    public:
      XapianResult(XapianSearcher* searcher, Xapian::MSetIterator& iterator);
      virtual ~XapianResult() {};

      virtual std::string get_url();
      virtual std::string get_title();
      virtual int get_score();
      virtual std::string get_snippet();
      virtual int get_wordCount();
      virtual int get_size();

    private:
      XapianSearcher* searcher;
      Xapian::MSetIterator iterator;
      Xapian::Document document;
  };

  class NoXapianIndexInZim: public exception {
    virtual const char* what() const throw() {
      return "There is no fulltext index in the zim file";
    }
  };

  class XapianSearcher : public Searcher {
    friend class XapianResult;
  public:
    XapianSearcher(const string &xapianDirectoryPath);
    virtual ~XapianSearcher() {};
    void searchInIndex(string &search, const unsigned int resultStart, const unsigned int resultEnd, 
		       const bool verbose=false);
    virtual Result* getNextResult();
    void restart_search();

  protected:
    void closeIndex();
    void openIndex(const string &xapianDirectoryPath);

    Xapian::Database readableDatabase;
    Xapian::Stem stemmer;
    Xapian::MSet results;
    Xapian::MSetIterator current_result;
    std::map<std::string, int> valuesmap;
  };

}

#endif
