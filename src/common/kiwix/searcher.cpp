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

#include "searcher.h"

namespace kiwix {

  /* Constructor */
  Searcher::Searcher() {
  }
  
  /* Search strings in the database */
  void Searcher::search(std::string &search, const unsigned int resultsCount, const bool verbose) {

    this->reset();
    
    if (verbose == true) {
      cout << "Performing query `" << search << "'" << endl;
    }

    searchInIndex(removeAccents(search), resultsCount, verbose);

    this->resultOffset = this->results.begin();

    return;
  }
  
  /* Reset the results */
  void Searcher::reset() {
    this->results.clear();
    this->resultOffset = this->results.begin();
    return;
  }
  
  /* Get next result */
  bool Searcher::getNextResult(string &url, string &title, unsigned int &score) {
    bool retVal = false;
    
    if (this->resultOffset != this->results.end()) {
      
      /* url */
      url = this->resultOffset->url;
      
      /* title */
      title = this->resultOffset->title;
      
      /* score */
      score =  this->resultOffset->score;
      
      /* increment the cursor for the next call */
      this->resultOffset++;
      
      retVal = true;
    }

    return retVal;
  }

  const string Searcher::searchInIndexAndReturnHtml(string &search, const unsigned int resultsCount, 
					const string templatePath, const bool verbose) {

	VMOpcodeCollector  oVMOpcodeCollector;
	StaticText         oSyscalls;
	StaticData         oStaticData;
	StaticText         oStaticText;
	HashTable          oHashTable;
	CTPP2Compiler oCompiler(oVMOpcodeCollector, oSyscalls, oStaticData, oStaticText, oHashTable);

	try
	{
		// Load template
		CTPP2FileSourceLoader oSourceLoader;
		oSourceLoader.LoadTemplate(search.c_str());

		// Create template parser
		CTPP2Parser oCTPP2Parser(&oSourceLoader, &oCompiler, search.c_str());

		// Compile template
		oCTPP2Parser.Compile();
	}
	catch(...)
	{
	}

    return "";
  }
  
}
