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
  Indexer::Indexer(const string &zimFilePath, const string &xapianDirectoryPath) 
    : zimFileHandler(NULL), 
      articleCount(0), 
      stepSize(0) {

    /* Open the ZIM file */
    this->zimFileHandler = new zim::File(zimFilePath);
    
    /* Open the Xapian directory */
    this->writableDatabase = new Xapian::WritableDatabase(xapianDirectoryPath, 
							  Xapian::DB_CREATE_OR_OVERWRITE);

    /* Stemming */
    /*
    stemmer = Xapian::Stem("french");
    indexer.set_stemmer(stemmer);
    */

    /* Read the stopwords file */
    /*
    this->readStopWordsFile("/home/kelson/kiwix/moulinkiwix/stopwords/fr");
    std::vector<std::string>::const_iterator stopWordsIterator = this->stopWords.begin();
    this->stopper.add("ceci");
    while (stopWordsIterator != this->stopWords.end()) {
      this->stopper.add(*stopWordsIterator);
      stopWordsIterator++;
    }
    indexer.set_stopper(&(this->stopper));
    */

    /* Prepare the indexation */
    this->prepareIndexing();
  }
  
  /* Destructor */
  Indexer::~Indexer() {
    this->stopIndexing();
  }
  
  /* Start indexing */
  void Indexer::prepareIndexing() {

    /* Define a few values */
    this->firstArticleOffset = this->zimFileHandler->getNamespaceBeginOffset('A');
    this->lastArticleOffset = this->zimFileHandler->getNamespaceEndOffset('A');
    this->currentArticleOffset = this->firstArticleOffset;
    
    /* Compute few things */
    this->articleCount = this->zimFileHandler->getNamespaceCount('A');
    this->stepSize = (float)this->articleCount / (float)100;
  }
  
  /* Index next percent */
  bool Indexer::indexNextPercent(const bool &verbose) {
    float thresholdOffset = this->currentArticleOffset + this->stepSize;
    size_t found;

    /* Check if we can start */
    if (this->zimFileHandler == NULL || this->writableDatabase == NULL) {
      return false;
    }

    /* Begin the Xapian transation */
    this->writableDatabase->begin_transaction(true);

    while(this->currentArticleOffset < thresholdOffset && 
	  this->currentArticleOffset < this->lastArticleOffset) {

      zim::Article currentArticle;
      Xapian::Document currentDocument;
      
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
	  
	  /* Put the data in the document */
	  currentDocument.clear_values();
	  currentDocument.add_value(0, this->htmlParser.title);
	  currentDocument.set_data(currentArticle.getLongUrl().c_str());
	  indexer.set_document(currentDocument);
	  
	  /* Debug output */
	  if (verbose) {
	    std::cout << "Indexing " << currentArticle.getLongUrl() << "..." << std::endl;
	  }
	  
	  /* Index the title */
	  if (!this->htmlParser.title.empty()) {
	    indexer.index_text_without_positions(removeAccents(this->htmlParser.title), 
						 ((this->htmlParser.dump.size() / 100) + 1) / 
						 countWords(this->htmlParser.title) );
	  }
	  
	  /* Index the keywords */
	  if (!this->htmlParser.keywords.empty()) {
	    indexer.index_text_without_positions(removeAccents(this->htmlParser.keywords), 3);
	  }
	  
	  /* Index the content */
	  if (!this->htmlParser.dump.empty()) {
	    indexer.index_text_without_positions(removeAccents(this->htmlParser.dump));
	  }
	  
	  /* add to the database */
	  this->writableDatabase->add_document(currentDocument);
	}
      }
    }
    
    /* Flush and close Xapian transaction*/
    this->writableDatabase->commit_transaction();

    /* increment the offset and set returned value */
    if (this->currentArticleOffset < this->lastArticleOffset) {
      this->currentArticleOffset++;
      return true;
    } else {
      this->stopIndexing();
      return false;
    }
  }
  
  /* Stop indexing. TODO: using it crashs the soft under windows. Have to do it in indexNextPercent() */
  void Indexer::stopIndexing() {
    /* Delete the zimFileHandler */
    if (this->zimFileHandler != NULL) {
      delete this->zimFileHandler;
      this->zimFileHandler = NULL;
    }
    
    /* Delete the Xapian writableDatabase */
    if (this->writableDatabase != NULL) {
      delete this->writableDatabase;
      this->writableDatabase = NULL;
    }
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
}
