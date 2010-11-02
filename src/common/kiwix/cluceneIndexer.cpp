#include "cluceneIndexer.h"

namespace kiwix {

  CluceneIndexer::CluceneIndexer(const string &zimFilePath, const string &cluceneDirectoryPath) :
    Indexer(zimFilePath) {

    this->dir = FSDirectory::getDirectory(cluceneDirectoryPath.c_str(), true);
    this->writer = new IndexWriter(dir, &analyzer, true);
  }
  
  void CluceneIndexer::indexNextPercentPre() {
  }
  
  void CluceneIndexer::indexNextArticle(const string &url,
					const string &title,
					const string &unaccentedTitle,
					const string &keywords,
					const string &content) {
    
    Document doc;
    
    /* Not indexed */
    doc.add(*_CLNEW Field((const wchar_t*)("title"), (const wchar_t*)(title.c_str()), 
			  Field::STORE_YES | Field::INDEX_UNTOKENIZED));
    doc.add(*_CLNEW Field((const wchar_t*)("url"), (const wchar_t*)(url.c_str()), 
			  Field::STORE_YES | Field::INDEX_UNTOKENIZED));

    /* indexed */
    doc.add(*_CLNEW Field((const wchar_t*)("unaccentedTitle"), (const wchar_t*)(unaccentedTitle.c_str()), 
			  Field::STORE_NO | Field::INDEX_TOKENIZED));
    doc.add(*_CLNEW Field((const wchar_t*)("keywords"), (const wchar_t*)(keywords.c_str()), 
			  Field::STORE_NO | Field::INDEX_TOKENIZED));
    doc.add(*_CLNEW Field((const wchar_t*)("content"), (const wchar_t*)(content.c_str()), 
			  Field::STORE_NO | Field::INDEX_TOKENIZED));
    this->writer->addDocument(&doc);
  }

  void CluceneIndexer::indexNextPercentPost() {
  }
  
  void CluceneIndexer::stopIndexing() {
    this->writer->close();
  }
}
