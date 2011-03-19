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
  Searcher::Searcher() :
    estimatedResultCount(0),
    resultTemplatePath("") {
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
    this->estimatedResultCount = 0;
    return;
  }

  /* Return the result count estimation */
  const unsigned int Searcher::getEstimatedResultCount() {
    return this->estimatedResultCount;
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

  const bool Searcher::setResultTemplatePath(const std::string path) {
    this->resultTemplatePath = path;
    return true;
  }

  const string Searcher::getHtml() {
    
    const STLW::string & sSourceFile = this->resultTemplatePath;
    VMOpcodeCollector  oVMOpcodeCollector;
    StaticText         oSyscalls;
    StaticData         oStaticData;
    StaticText         oStaticText;
    HashTable          oHashTable;
    CTPP2Compiler oCompiler(oVMOpcodeCollector, oSyscalls, oStaticData, oStaticText, oHashTable);
    
    // Load template
    CTPP2FileSourceLoader oSourceLoader;
    oSourceLoader.LoadTemplate(sSourceFile.c_str());
    
    // Create template parser
    CTPP2Parser oCTPP2Parser(&oSourceLoader, &oCompiler, sSourceFile);
    
    // Compile template
    oCTPP2Parser.Compile();

    // Get program core
    UINT_32 iCodeSize = 0;
    const VMInstruction * oVMInstruction = oVMOpcodeCollector.GetCode(iCodeSize);

    // Dump program
    VMDumper oDumper(iCodeSize, oVMInstruction, oSyscalls, oStaticData, oStaticText, oHashTable);
    UINT_32 iSize = 0;
    const VMExecutable * aProgramCore = oDumper.GetExecutable(iSize);

    // Memory core
    const VMMemoryCore vm_core(aProgramCore);

    // Initiate the VM
    SyscallFactory oSyscallFactory(100);
    // Load standard library
    STDLibInitializer::InitLibrary(oSyscallFactory);

    VM * pVM = new VM(&oSyscallFactory);

    // Initiate the logger
    FileLogger oLogger(stderr);

    // Fill data
    CDT oData;
    CDT resultsCDT(CDT::ARRAY_VAL);

    this->resultOffset = this->results.begin();
    while (this->resultOffset != this->results.end()) {
      CDT result;
      result["title"] = this->resultOffset->title;
      result["url"] = this->resultOffset->url;
      resultsCDT.PushBack(result);
      this->resultOffset++;
    }
    this->resultOffset = this->results.begin();

    oData["results"] = resultsCDT;
    oData["count"] = this->estimatedResultCount;

    STLW::string sResult;
    StringOutputCollector oDataCollector(sResult);

    // Run VM
    pVM->Init(&vm_core, &oDataCollector, &oLogger);
    UINT_32 iIP = 0;
    pVM -> Run(&vm_core, &oDataCollector, iIP, oData, &oLogger);

    return sResult;
  }
  
}
