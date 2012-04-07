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

#include "reader.h"

static char charFromHex(std::string a) {
  std::istringstream Blat (a);
  int Z;
  Blat >> std::hex >> Z;
  return char (Z);
}

void unescapeUrl(string &url) {
  std::string::size_type pos;
  std::string hex;
  while (std::string::npos != (pos = url.find('%'))) {
      hex = url.substr(pos + 1, 2);
      url.replace(pos, 3, 1, charFromHex(hex));
  }
  return;
}

namespace kiwix {

  /* Constructor */
  Reader::Reader(const string &zimFilePath) 
    : zimFileHandler(NULL) {
    
    this->zimFileHandler = new zim::File(zimFilePath);
    
    if (this->zimFileHandler != NULL) {
      this->firstArticleOffset = this->zimFileHandler->getNamespaceBeginOffset('A');
      this->lastArticleOffset = this->zimFileHandler->getNamespaceEndOffset('A');
      this->currentArticleOffset = this->firstArticleOffset;
      this->articleCount = this->zimFileHandler->getNamespaceCount('A');
      this->mediaCount = this->zimFileHandler->getNamespaceCount('I');
    }

    /* initialize random seed: */
    srand ( time(NULL) );
  }
  
  /* Destructor */
  Reader::~Reader() {
    if (this->zimFileHandler != NULL) {
      delete this->zimFileHandler;
    }
  }

  zim::File* Reader::getZimFileHandler() {
    return this->zimFileHandler;
  }
  
  /* Reset the cursor for GetNextArticle() */
  void Reader::reset() {
    this->currentArticleOffset = this->firstArticleOffset;
  }

  std::map<std::string, unsigned int> Reader::parseCounterMetadata() {
    std::map<std::string, unsigned int> counters;
    string content, mimeType, item, counterString;
    unsigned int contentLength, counter;
    string counterUrl = "/M/Counter";

    this->getContentByUrl(counterUrl, content, contentLength, mimeType);
    cout << "Counter URL: " << content << endl;
    stringstream ssContent(content);

    while(getline(ssContent, item,  ';')) {
      stringstream ssItem(item);
      getline(ssItem, mimeType, '=');
      getline(ssItem, counterString, '=');
      if (!counterString.empty() && !mimeType.empty()) {
	sscanf(counterString.c_str(), "%u", &counter);
	counters.insert(pair<string, int>(mimeType, counter));
      }
    }

    return counters;
  }
  
  /* Get the count of articles which can be indexed/displayed */
  unsigned int Reader::getArticleCount() {
    std::map<std::string, unsigned int> counterMap = this->parseCounterMetadata();
    unsigned int counter = 0;
    
    if (counterMap.empty()) {
      counter = this->articleCount;
      cout << "Article count from offsets: " << counter << endl;
    } else {
      std::map<std::string, unsigned int>::const_iterator it = counterMap.find("text/html");
      if (it != counterMap.end())
	counter = it->second;
      cout << "Article count from metadata: " << counter << endl;
    }
    
    return counter;
  }

  unsigned int Reader::getMediaCount() {
    std::map<std::string, unsigned int> counterMap = this->parseCounterMetadata();
    unsigned int counter = 0;

    if (counterMap.empty())
      counter = this->mediaCount;
    else {
      std::map<std::string, unsigned int>::const_iterator it;

      it = counterMap.find("image/jpeg");
      if (it != counterMap.end())
	counter += it->second;

      it = counterMap.find("image/gif");
      if (it != counterMap.end())
	counter += it->second;

      it = counterMap.find("image/png");
      if (it != counterMap.end())
	counter += it->second;
    }
    
    return counter;
  }
  
  /* Return the UID of the ZIM file */
  string Reader::getId() {
    std::ostringstream s;
    s << this->zimFileHandler->getFileheader().getUuid();
    return  s.str();
  }
  
  /* Return a page url from a title */
  bool Reader::getPageUrlFromTitle(const string &title, string &url) {
    /* Extract the content from the zim file */
    std::pair<bool, zim::File::const_iterator> resultPair = zimFileHandler->findxByTitle('A', title);

    /* Test if the article was found */
    if (resultPair.first == true) {
      
      /* Get the article */
      zim::Article article = *resultPair.second;

      /* If redirect */
      unsigned int loopCounter = 0;
      while (article.isRedirect() && loopCounter++<42) {
	article = article.getRedirectArticle();
      }
      
      url = article.getLongUrl();
      return true;
    }

    return false;
  }

  /* Return an URL from a title*/
  string Reader::getRandomPageUrl() {
    zim::size_type idx = this->firstArticleOffset + 
      (zim::size_type)((double)rand() / ((double)RAND_MAX + 1) * this->articleCount); 
    zim::Article article = zimFileHandler->getArticle(idx);

    return article.getLongUrl().c_str();
  }
  
  /* Return the welcome page URL */
  string Reader::getMainPageUrl() {
    string url = "";
    
    if (this->zimFileHandler->getFileheader().hasMainPage()) {
      zim::Article article = zimFileHandler->getArticle(this->zimFileHandler->getFileheader().getMainPage());
      url = article.getLongUrl();

      if (url.empty()) {
	url = getFirstPageUrl(); 
      }
    } else {
	url = getFirstPageUrl(); 
    }
    
    return url;
  }
  
  bool Reader::getFavicon(string &content, string &mimeType) {
    unsigned int contentLength = 0;
    
    this->getContentByUrl( "/-/favicon.png", content, 
			   contentLength, mimeType);
    
    if (content.empty()) {
      this->getContentByUrl( "/I/favicon.png", content, 
			     contentLength, mimeType);


      if (content.empty()) {
	this->getContentByUrl( "/I/favicon", content, 
			       contentLength, mimeType);
	
	if (content.empty()) {
	  this->getContentByUrl( "/-/favicon", content, 
				 contentLength, mimeType);
	}
      }
    }
    
    return content.empty() ? false : true;
  }

  /* Return a metatag value */
  bool Reader::getMetatag(const string &name, string &value) {
    unsigned int contentLength = 0;
    string contentType = "";
    
    return this->getContentByUrl( "/M/" + name, value, 
				  contentLength, contentType);
  }
  
  string Reader::getTitle() {
    string value;
    this->getMetatag("Title", value);
    return value;
  }

  string Reader::getDescription() {
    string value;
    this->getMetatag("Description", value);
    return value;
  }

  string Reader::getLanguage() {
    string value;
    this->getMetatag("Language", value);
    return value;
  }

  string Reader::getDate() {
    string value;
    this->getMetatag("Date", value);
    return value;
  }

  string Reader::getCreator() {
    string value;
    this->getMetatag("Creator", value);
    return value;
  }

  string Reader::getPublisher() {
    string value;
    this->getMetatag("Publisher", value);
    return value;
  }

  /* Return the first page URL */
  string Reader::getFirstPageUrl() {
    string url;
    
    zim::size_type firstPageOffset = zimFileHandler->getNamespaceBeginOffset('A');
    zim::Article article = zimFileHandler->getArticle(firstPageOffset);
    url = article.getLongUrl();
    
    return url;
  }
  
  bool Reader::parseUrl(const string &urlStr, char *ns, string &titleStr) {
    const char *url = urlStr.c_str();
    
    /* Offset to visit the url */
    unsigned int urlLength = strlen(url);
    unsigned int offset = 0;
    
    /* Ignore the '/' */
    while ((offset < urlLength) && (url[offset] == '/')) offset++;
    
    /* Get namespace */
    while ((offset < urlLength) && (url[offset] != '/')) {
      *ns= url[offset];
      offset++;
    }
    
    /* Ignore the '/' */
    while ((offset < urlLength) && (url[offset] == '/')) offset++;  
      
    /* Get content title */
    char title[1024];
    unsigned int titleOffset = 0;
    while (offset < urlLength) {
      title[titleOffset] = url[offset];
      offset++;
      titleOffset++;
    }
    title[titleOffset] = 0;
    
    /* unescape url */
    titleStr = string(title);
    unescapeUrl(titleStr);

    return true;
  }

  /* Get a content from a zim file */
  bool Reader::getContentByUrl(const string &urlStr, string &content, unsigned int &contentLength, string &contentType) {
    bool retVal = false;
    content="";
    contentType="";
    contentLength = 0;

    if (this->zimFileHandler != NULL) {
      
      /* Parse the url */
      char ns = 0;
      string titleStr;
      this->parseUrl(urlStr, &ns, titleStr);

      /* Main page */
      if (titleStr.empty() && ns == 0) {
	this->parseUrl(this->getMainPageUrl(), &ns, titleStr);
      }
      
      /* Extract the content from the zim file */
      std::pair<bool, zim::File::const_iterator> resultPair = zimFileHandler->findx(ns, titleStr);
      
      /* Test if the article was found */
      if (resultPair.first == true) {
	
	/* Get the article */
	zim::Article article = zimFileHandler->getArticle(resultPair.second.getIndex());
	
	/* If redirect */
	unsigned int loopCounter = 0;
	while (article.isRedirect() && loopCounter++<42) {
	  article = article.getRedirectArticle();
	}
	
	/* Get the content mime-type */
	contentType = string(article.getMimeType().data(), article.getMimeType().size()); 
	
	/* Get the data */
	content = string(article.getData().data(), article.getArticleSize());
	
	/* Try to set a stub HTML header/footer if necesssary */
	if (contentType == "text/html" && std::string::npos == content.find("<body>")) {
	  content = "<html><head><title>" + article.getTitle() + "</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /></head><body>" + content + "</body></html>";
	}
	
	/* Get the data length */
	contentLength = article.getArticleSize();
	
	/* Set return value */
	retVal = true;
      }
    }
    
    return retVal;
  }
  
  /* Search titles by prefix*/
  bool Reader::searchSuggestions(const string &prefix, unsigned int suggestionsCount) {
    bool retVal = true;
    
    /* Reset the suggestions */
    this->suggestions.clear();
    
    if (prefix.size()) {
      for (zim::File::const_iterator it = zimFileHandler->findByTitle('A', prefix); 
	   it != zimFileHandler->end() && it->getTitle().compare(0, prefix.size(), prefix) == 0 
	     && this->suggestions.size() < suggestionsCount ; ++it) {
	
	this->suggestions.push_back(it->getTitle());
      }
    } else {
      retVal = false;
    }
    
    /* Set the cursor to the begining */
    this->suggestionsOffset = this->suggestions.begin();
    
    return retVal;
  }
  
  /* Get next suggestion */
  bool Reader::getNextSuggestion(string &title) {
    if (this->suggestionsOffset != this->suggestions.end()) {
      /* title */
      title = *(this->suggestionsOffset);
      
      /* increment the cursor for the next call */
      this->suggestionsOffset++;
      
      return true;
    }

    return false;
  }

  /* Check if the file has as checksum */
  bool Reader::canCheckIntegrity() {
    return this->zimFileHandler->getChecksum() != "";
  }

  /* Return true if corrupted, false otherwise */
  bool Reader::isCorrupted() {
    try {
      if (this->zimFileHandler->verify() == true)
	return false;
    } catch (exception &e) {
      cerr << e.what() << endl;
      return true;
    }

    return true;
  }
}
