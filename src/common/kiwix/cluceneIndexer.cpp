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

#include "cluceneIndexer.h"

namespace kiwix {

  TCHAR buffer[MAX_BUFFER_SIZE];

  CluceneIndexer::CluceneIndexer() {
  }

  void CluceneIndexer::indexingPrelude(const string indexPath) {
    this->dir = FSDirectory::getDirectory(indexPath.c_str(), true);
    this->writer = new IndexWriter(this->dir, &analyzer, true);
    this->writer->setUseCompoundFile(false);
  }
  
  void CluceneIndexer::index(const string &url,
			     const string &title,
			     const string &unaccentedTitle,
			     const string &keywords,
			     const string &content,
			     const string &snippet,
			     const string &size,
			     const string &wordCount) {
    
    Document doc;
    
    /* Not indexed but stored */
    //STRCPY_AtoT(buffer, title.c_str(), MAX_BUFFER_SIZE);
    ::mbstowcs(buffer,title.c_str(),MAX_BUFFER_SIZE);
    doc.add(*_CLNEW Field(_T("title"), buffer, Field::STORE_YES | Field::INDEX_UNTOKENIZED)); // TODO: Why store, not analyzed? what is utitle?

    //STRCPY_AtoT(buffer, url.c_str(), MAX_BUFFER_SIZE);
    ::mbstowcs(buffer,url.c_str(),MAX_BUFFER_SIZE);
    doc.add(*_CLNEW Field(_T("url"), buffer, Field::STORE_YES | Field::INDEX_UNTOKENIZED));

    /* indexed but not stored */
    //STRCPY_AtoT(buffer, unaccentedTitle.c_str(), MAX_BUFFER_SIZE);
    ::mbstowcs(buffer,unaccentedTitle.c_str(),MAX_BUFFER_SIZE);
    Field *titleField = new Field(_T("utitle"), buffer, Field::STORE_NO | Field::INDEX_TOKENIZED);
    titleField->setBoost(getTitleBoostFactor(content.size()));
    doc.add(*titleField);

    //STRCPY_AtoT(buffer, keywords.c_str(), MAX_BUFFER_SIZE);
    ::mbstowcs(buffer,keywords.c_str(),MAX_BUFFER_SIZE);
    Field *keywordsField = new Field(_T("keywords"), buffer, Field::STORE_NO | Field::INDEX_TOKENIZED);
    keywordsField->setBoost(keywordsBoostFactor);
    doc.add(*keywordsField);

    //STRCPY_AtoT(buffer, content.c_str(), MAX_BUFFER_SIZE);
    ::mbstowcs(buffer,content.c_str(),MAX_BUFFER_SIZE);
    doc.add(*_CLNEW Field(_T("content"), buffer, Field::STORE_NO | Field::INDEX_TOKENIZED)); // TODO: TermVectors if you want to highlight

    /* Add the document to the index */
    this->writer->addDocument(&doc);
  }

  void CluceneIndexer::flush() {
  }

  void CluceneIndexer::indexingPostlude() {
    this->writer->setUseCompoundFile(true);
    this->writer->optimize();
    this->writer->close();
    delete this->writer;
    _CLDECDELETE(this->dir);
  }
}
