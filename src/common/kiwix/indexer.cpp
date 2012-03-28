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

#include "indexer.h"

namespace kiwix {

  /* Count word */
  unsigned int Indexer::countWords(const string &text) {
    unsigned int numWords = 1;
    for(unsigned int i=0; i<text.size();) {
      while(i<text.size() && text[i] != ' ') {
	i++;
      }
      numWords++;
      i++;
    }
    return numWords;
  }

  /* Constructor */
  Indexer::Indexer(const string &zimFilePath) 
    : zimFileHandler(NULL), 
      articleCount(0), 
      stepSize(0),
      keywordsBoostFactor(3) {
   
    this->setZimFilePath(zimFilePath);

    /* Read the stopwords file */
    //this->readStopWordsFile("/home/kelson/kiwix/moulinkiwix/stopwords/fr");
  }

  bool Indexer::setZimFilePath(const string &zimFilePath) {
    /* Open the ZIM file */
    this->zimFileHandler = new zim::File(zimFilePath);

    /* Define a few values */
    this->firstArticleOffset = this->zimFileHandler->getNamespaceBeginOffset('A');
    this->lastArticleOffset = this->zimFileHandler->getNamespaceEndOffset('A');
    this->currentArticleOffset = this->firstArticleOffset;
    
    /* Compute few things */
    this->articleCount = this->zimFileHandler->getNamespaceCount('A');
    this->stepSize = (float)this->articleCount / (float)100;

    /* Thread mgmt */
    this->runningStatus = 0;
  }

  void *Indexer::extractArticles(void *ptr) {
    kiwix::Indexer *self = (kiwix::Indexer *)ptr;
    self->incrementRunningStatus();
    unsigned int startOffset = self->zimFileHandler->getNamespaceBeginOffset('A');
    unsigned int endOffset = self->zimFileHandler->getNamespaceEndOffset('A');

    /* Goes trough all articles */
    unsigned int currentOffset = startOffset;
    zim::Article currentArticle;

    while (currentOffset <= endOffset) {
      /* Test if the thread should be cancelled */
      pthread_testcancel();

      /* Redirects are not indexed */
      do {
	currentArticle = self->zimFileHandler->getArticle(currentOffset++);
      } while (currentArticle.isRedirect() && currentOffset++ != endOffset);

      cout << currentArticle.getTitle() << endl;

    }

    self->decrementRunningStatus();
    pthread_exit(NULL);
    return NULL;
  }

  void *Indexer::parseArticles(void *ptr) {
    pthread_exit(NULL);
    return NULL;
  }

  void *Indexer::writeIndex(void *ptr) {
    pthread_exit(NULL);
    return NULL;
  }

  bool Indexer::start() {
    pthread_create(&(this->articleExtracter), NULL, Indexer::extractArticles, ( void *)this);
    pthread_detach(this->articleExtracter);
    cout << "end" << endl;

    return true;
  }

  bool Indexer::stop() {
      
      return true;
  }

  void Indexer::incrementRunningStatus() {
    this->runningStatus++;
  }

  void Indexer::decrementRunningStatus() {
    this->runningStatus--;
  }

  unsigned int Indexer::getRunningStatus() {
    return this->runningStatus;
  }

  bool Indexer::isRunning() {
    return this->runningStatus > 0;
  }

  void Indexer::setCurrentArticleOffset(unsigned int offset) {
    this->currentArticleOffset = offset;
  }

  unsigned int Indexer::getCurrentArticleOffset() {
    return this->currentArticleOffset;
  }

  unsigned int Indexer::getProgression() {
    unsigned int progression = 0;
    return progression;
  }

  /* Read the file containing the stopwords */
  bool Indexer::readStopWordsFile(const string path) {
    std::string stopWord;
    std::ifstream file(path.c_str(), std::ios_base::in);

    this->stopWords.clear();

    while (getline(file, stopWord, '\n')) {
      this->stopWords.push_back(stopWord);
    }

    std::cout << "Read " << this->stopWords.size() << " lines.\n";
    return true;
  }

  /* Index next percent */
  bool Indexer::indexNextPercent(const bool &verbose) {
    float thresholdOffset = this->currentArticleOffset + this->stepSize;
    size_t found;

    /* Check if we can start */
    if (this->zimFileHandler == NULL) {
      return false;
    }

    this->indexNextPercentPre();

    while(this->currentArticleOffset < thresholdOffset && 
	  this->currentArticleOffset <= this->lastArticleOffset) {

      zim::Article currentArticle;
      
      /* Get next non redirect article */
      do {
	currentArticle = this->zimFileHandler->getArticle(this->currentArticleOffset);
      } while (this->currentArticleOffset++ &&
	       currentArticle.isRedirect() && 
	       this->currentArticleOffset != this->lastArticleOffset);

      if (!currentArticle.isRedirect()) {
	
	/* Index the content */
	this->htmlParser.reset();
	string content (currentArticle.getData().data(), currentArticle.getData().size());

	/* The parser generate a lot of exceptions which should be avoided */
	try {
	  this->htmlParser.parse_html(content, "UTF-8", true);
	} catch (...) {
	}
	
	/* If content does not have the noindex meta tag */
	/* Seems that the parser generates an exception in such case */
	found = this->htmlParser.dump.find("NOINDEX");
	
	if (found == string::npos) {
	  string url = currentArticle.getLongUrl();
	  
	  /* Debug output */
	  if (verbose) {
	    std::cout << "Indexing " << url << "..." << std::endl;
	  }

	  /* Get the title */
	  string accentedTitle = this->htmlParser.title;
	  if (accentedTitle.empty()) {
	    accentedTitle = currentArticle.getTitle();
	  }

	  /* count words */
	  stringstream countWordStringStream;
	  countWordStringStream << countWords(this->htmlParser.dump);
	  const std::string wordCountString = countWordStringStream.str();

	  /* snippet */
	  std::string snippet = std::string(this->htmlParser.dump, 0, 300);
	  std::string::size_type last = snippet.find_last_of('.');
	  if (last == snippet.npos)
	    last = snippet.find_last_of(' ');
	  if (last != snippet.npos)
	    snippet = snippet.substr(0, last);

	  /* size */
	  stringstream sizeStringStream;
	  sizeStringStream << content.size() / 1024;
	  const std::string size = sizeStringStream.str();

	  this->indexNextArticle(url, 
				 accentedTitle,
				 removeAccents(this->htmlParser.title), 
				 removeAccents(this->htmlParser.keywords),
				 removeAccents(this->htmlParser.dump),
				 snippet,
				 size,
				 wordCountString
				 );

	}
      }
    }

    this->indexNextPercentPost();
    
    /* increment the offset and set returned value */
    if (this->currentArticleOffset <= this->lastArticleOffset) {
      return true;
    } else {
        // commented as it never returns on OSX.
      //this->stopIndexing();
      return false;
    }
  }

}
