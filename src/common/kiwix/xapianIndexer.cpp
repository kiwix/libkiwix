#include "xapianIndexer.h"

namespace kiwix {

  /* Constructor */
  XapianIndexer::XapianIndexer(const string &zimFilePath, const string &xapianDirectoryPath) :
    Indexer(zimFilePath) {
    
    /* Open the Xapian directory */
    this->writableDatabase = new Xapian::WritableDatabase(xapianDirectoryPath, 
							  Xapian::DB_CREATE_OR_OVERWRITE);

    /* Stemming */
    /*
    stemmer = Xapian::Stem("french");
    indexer.set_stemmer(stemmer);
    */

    /* Stop words
    std::vector<std::string>::const_iterator stopWordsIterator = this->stopWords.begin();
    this->stopper.add("ceci");
    while (stopWordsIterator != this->stopWords.end()) {
      this->stopper.add(*stopWordsIterator);
      stopWordsIterator++;
    }
    indexer.set_stopper(&(this->stopper));
    */
  }
  
  void XapianIndexer::indexNextPercentPre() {
    this->writableDatabase->begin_transaction(true);
  }
  
  void XapianIndexer::indexNextArticle(string &url, string &title, string &unaccentedTitle,
				       string &keywords, string &content) {
    
    /* Put the data in the document */
    currentDocument.clear_values();
    currentDocument.add_value(0, title);
    currentDocument.set_data(url);
    indexer.set_document(currentDocument);

    /* Index the title */
    if (!unaccentedTitle.empty()) {
      indexer.index_text_without_positions(unaccentedTitle, 5);
    }
    
    /* Index the keywords */
    if (!keywords.empty()) {
      indexer.index_text_without_positions(keywords, 3);
    }
    
    /* Index the content */
    if (!content.empty()) {
      indexer.index_text_without_positions(content);
    }
    
    /* add to the database */
    this->writableDatabase->add_document(currentDocument);
  }

  void XapianIndexer::indexNextPercentPost() {
    /* Flush and close Xapian transaction*/
    this->writableDatabase->commit_transaction();
  }
  
  /* Stop indexing. TODO: using it crashs the soft under windows. Have to do it in indexNextPercent() */
  void XapianIndexer::stopIndexing() {
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
}
