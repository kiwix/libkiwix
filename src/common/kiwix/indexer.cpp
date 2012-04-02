/*
 * Copyright 2011-2012 Emmanuel Engelhart <kelson@kiwix.org>
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
   
    /* Initialize mutex */
    pthread_mutex_init(&threadIdsMutex, NULL);
    pthread_mutex_init(&toParseQueueMutex, NULL);
    pthread_mutex_init(&toIndexQueueMutex, NULL);
    pthread_mutex_init(&articleExtractorRunningMutex, NULL);
    pthread_mutex_init(&articleParserRunningMutex, NULL);
    pthread_mutex_init(&articleIndexerRunningMutex, NULL);

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
  }

  /* Article extractor methods */
  void *Indexer::extractArticles(void *ptr) {
    kiwix::Indexer *self = (kiwix::Indexer *)ptr;
    self->articleExtractorRunning(true);
    unsigned int startOffset = self->zimFileHandler->getNamespaceBeginOffset('A');
    unsigned int endOffset = self->zimFileHandler->getNamespaceEndOffset('A');

    /* Goes trough all articles */
    unsigned int currentOffset = startOffset;
    zim::Article currentArticle;

    while (currentOffset <= endOffset) {
      /* Redirects are not indexed */
      do {
	currentArticle = self->zimFileHandler->getArticle(currentOffset++);
      } while (currentArticle.isRedirect() && currentOffset++ != endOffset);

      /* Add articles to the queue */
      indexerToken token;
      token.title = currentArticle.getTitle();
      token.url = currentArticle.getLongUrl();
      token.content = string(currentArticle.getData().data(), currentArticle.getData().size());
      self->pushToParseQueue(token);

      /* Test if the thread should be cancelled */
      pthread_testcancel();
    }

    self->articleExtractorRunning(false);
    pthread_exit(NULL);
    return NULL;
  }
  
  void Indexer::articleExtractorRunning(bool value) {
    pthread_mutex_lock(&articleExtractorRunningMutex);
    this->articleExtractorRunningFlag = value;
    pthread_mutex_unlock(&articleExtractorRunningMutex); 
  }
  
  bool Indexer::isArticleExtractorRunning() {
    pthread_mutex_lock(&articleExtractorRunningMutex);
    bool retVal = this->articleExtractorRunningFlag;
    pthread_mutex_unlock(&articleExtractorRunningMutex); 
    return retVal;
  }
  
  /* Article parser methods */
  void *Indexer::parseArticles(void *ptr) {
    kiwix::Indexer *self = (kiwix::Indexer *)ptr;
    size_t found;
    indexerToken token;

    while (self->popFromToParseQueue(token)) {
      MyHtmlParser htmlParser;

      /* The parser generate a lot of exceptions which should be avoided */
      try {
	htmlParser.parse_html(token.content, "UTF-8", true);
      } catch (...) {
      }

      /* If content does not have the noindex meta tag */
      /* Seems that the parser generates an exception in such case */
      found = htmlParser.dump.find("NOINDEX");
      
      if (found == string::npos) {

	/* Get the accented title */
	token.accentedTitle = (htmlParser.title.empty() ? token.title : htmlParser.title);

	/* count words */
	stringstream countWordStringStream;
	countWordStringStream << self->countWords(htmlParser.dump);
	token.wordCount = countWordStringStream.str();
	
	/* snippet */
	std::string snippet = std::string(htmlParser.dump, 0, 300);
	std::string::size_type last = snippet.find_last_of('.');
	if (last == snippet.npos)
	  last = snippet.find_last_of(' ');
	if (last != snippet.npos)
	  snippet = snippet.substr(0, last);
	token.snippet = snippet;

	/* size */
	stringstream sizeStringStream;
	sizeStringStream << token.content.size() / 1024;
	token.size = sizeStringStream.str();

	/* Remove accent */
	token.title = removeAccents(token.accentedTitle);
	token.keywords = removeAccents(htmlParser.keywords);
	token.content = removeAccents(htmlParser.dump);
	self->pushToIndexQueue(token);

	/* Test if the thread should be cancelled */
	pthread_testcancel();      }
    }
    
    self->articleParserRunning(false);
    pthread_exit(NULL);
    return NULL;
  }

  void Indexer::articleParserRunning(bool value) {
    pthread_mutex_lock(&articleParserRunningMutex);
    this->articleParserRunningFlag = value;
    pthread_mutex_unlock(&articleParserRunningMutex); 
  }
  
  bool Indexer::isArticleParserRunning() {
    pthread_mutex_lock(&articleParserRunningMutex);
    bool retVal = this->articleParserRunningFlag;
    pthread_mutex_unlock(&articleParserRunningMutex); 
    return retVal;
  }

  /* Article indexer methods */
  void *Indexer::indexArticles(void *ptr) {
    kiwix::Indexer *self = (kiwix::Indexer *)ptr;
    indexerToken token;

    while (self->popFromToIndexQueue(token)) {
      self->indexNextArticle(token.url, 
			     token.accentedTitle,
			     token.title, 
			     token.keywords,
			     token.content,
			     token.snippet,
			     token.size,
			     token.wordCount
			     );
    }

    self->indexNextPercentPost();

    self->articleIndexerRunning(false);
    pthread_exit(NULL);
    return NULL;
  }

  void Indexer::articleIndexerRunning(bool value) {
    pthread_mutex_lock(&articleIndexerRunningMutex);
    this->articleIndexerRunningFlag = value;
    pthread_mutex_unlock(&articleIndexerRunningMutex); 
  }
  
  bool Indexer::isArticleIndexerRunning() {
    pthread_mutex_lock(&articleIndexerRunningMutex);
    bool retVal = this->articleIndexerRunningFlag;
    pthread_mutex_unlock(&articleIndexerRunningMutex); 
    return retVal;
  }

  /* ToParseQueue methods */
  bool Indexer::isToParseQueueEmpty() {
    pthread_mutex_lock(&toParseQueueMutex);
    bool retVal = this->toParseQueue.empty();
    pthread_mutex_unlock(&toParseQueueMutex);
    return retVal;
  }

  void Indexer::pushToParseQueue(indexerToken &token) {
    pthread_mutex_lock(&toParseQueueMutex); 
    this->toParseQueue.push(token);
    pthread_mutex_unlock(&toParseQueueMutex); 
    sleep(int(this->toParseQueue.size() / 200) / 10);
  }

  bool Indexer::popFromToParseQueue(indexerToken &token) {
    while (this->isToParseQueueEmpty() && this->isArticleExtractorRunning()) {
      sleep(0.5);
    }

    if (!this->isToParseQueueEmpty()) {
      pthread_mutex_lock(&toParseQueueMutex); 
      token = this->toParseQueue.front();
      this->toParseQueue.pop();
      pthread_mutex_unlock(&toParseQueueMutex); 
    } else {
      return false;
    }

    return true;
  }

  /* ToIndexQueue methods */
  bool Indexer::isToIndexQueueEmpty() {
    pthread_mutex_lock(&toIndexQueueMutex);
    bool retVal = this->toIndexQueue.empty();
    pthread_mutex_unlock(&toIndexQueueMutex);
    return retVal;
  }

  void Indexer::pushToIndexQueue(indexerToken &token) {
    pthread_mutex_lock(&toIndexQueueMutex); 
    this->toIndexQueue.push(token);
    pthread_mutex_unlock(&toIndexQueueMutex); 
    sleep(int(this->toIndexQueue.size() / 200) / 10);
  }

  bool Indexer::popFromToIndexQueue(indexerToken &token) {
    while (this->isToIndexQueueEmpty() && this->isArticleParserRunning()) {
      sleep(0.5);
    }

    if (!this->isToIndexQueueEmpty()) {
      pthread_mutex_lock(&toIndexQueueMutex); 
      token = this->toIndexQueue.front();
      this->toIndexQueue.pop();
      pthread_mutex_unlock(&toIndexQueueMutex); 
    } else {
      return false;
    }

    return true;
  }

  bool Indexer::start() {
    this->indexNextPercentPre();
    pthread_mutex_lock(&threadIdsMutex); 
    pthread_create(&(this->articleExtractor), NULL, Indexer::extractArticles, (void*)this);
    pthread_detach(this->articleExtractor);
    pthread_create(&(this->articleParser), NULL, Indexer::parseArticles, (void*)this);
    pthread_detach(this->articleParser);
    pthread_create(&(this->articleIndexer), NULL, Indexer::indexArticles, (void*)this);
    pthread_detach(this->articleIndexer);
    pthread_mutex_unlock(&threadIdsMutex);
    return true;
  }

  bool Indexer::stop() {
    pthread_cancel(this->articleExtractor);
    return true;
  }

  bool Indexer::isRunning() {
    return this->isArticleExtractorRunning() || this->isArticleIndexerRunning() || this->isArticleParserRunning();
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
