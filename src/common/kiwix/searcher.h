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

#ifndef KIWIX_SEARCHER_H
#define KIWIX_SEARCHER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <locale>
#include <cctype>
#include <vector>
#include <unaccent.h>

#include <CTPP2Parser.hpp>
#include <CTPP2FileSourceLoader.hpp>
#include <CTPP2ParserException.hpp>
#include <CTPP2HashTable.hpp>
#include <CTPP2VMDumper.hpp>
#include <CTPP2VMOpcodes.h>

using namespace std;
using namespace CTPP;

struct Result
{
  string url;
  string title;
  int score;
}; 

namespace kiwix {

  class Searcher {
    
  public:
    Searcher();

    void search(std::string &search, const unsigned int resultsCount, const bool verbose=false);
    bool getNextResult(string &url, string &title, unsigned int &score);
    void reset();
    const string searchInIndexAndReturnHtml(string &search, const unsigned int resultsCount, 
			  const string templatePath, const bool verbose=false);

  protected:
    virtual void closeIndex() = 0;
    virtual void searchInIndex(string &search, const unsigned int resultsCount, const bool verbose=false) = 0;

    std::vector<Result> results;
    std::vector<Result>::iterator resultOffset;
  };

}

#endif
