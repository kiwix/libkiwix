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
    }
  }
  
  /* Destructor */
  Reader::~Reader() {
      if (this->zimFileHandler != NULL) {
	delete this->zimFileHandler;
      }
  }
  
  /* Reset the cursor for GetNextArticle() */
  void Reader::reset() {
    this->currentArticleOffset = this->firstArticleOffset;
  }

/* Get the count of articles which can be indexed/displayed */
unsigned int Reader::getArticleCount() {
  return this->articleCount;
}

/* Return the UID of the ZIM file */
string Reader::getId() {
  std::ostringstream s;
  s << this->zimFileHandler->getFileheader().getUuid();
  return  s.str();
}
 
/* Return a random article URL */
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
   }
   
   return url;
   
 }

/* Return the first page URL */
string Reader::getFirstPageUrl() {
   string url = "";
   
   zim::size_type firstPageOffset = zimFileHandler->getNamespaceBeginOffset('A');
   zim::Article article = zimFileHandler->getArticle(firstPageOffset);
   url = article.getLongUrl();
   
   return url;
 }

/* Get a content from a zim file */
 bool Reader::getContentByUrl(const string &urlStr, string &content, unsigned int &contentLength, string &contentType) {
   bool retVal = false;
   const char *url = urlStr.c_str();

   /* Offset to visit the url */
   unsigned int urlLength = strlen(url);
   unsigned int offset = 0;
   
   /* Ignore the '/' */
   while((offset < urlLength) && (url[offset] == '/')) offset++;
   
   /* Get namespace */
   char ns[1024];
   unsigned int nsOffset = 0;
   while((offset < urlLength) && (url[offset] != '/')) {
     ns[nsOffset] = url[offset];
     offset++;
     nsOffset++;
   }
   ns[nsOffset] = 0;
   
   /* Ignore the '/' */
   while((offset < urlLength) && (url[offset] == '/')) offset++;  
   
   /* Get content title */
   char title[1024];
   unsigned int titleOffset = 0;
   while((offset < urlLength) && (url[offset] != '/')) {
     title[titleOffset] = url[offset];
     offset++;
     titleOffset++;
   }
   title[titleOffset] = 0;
   
   /* unescape url */
   string titleStr = string(title);
   unescapeUrl(titleStr);

   /* Main page */
   if (titleStr == "" && strcmp(ns, "") == 0) {
     if (zimFileHandler->getFileheader().hasMainPage()) {
       zim::Article article = zimFileHandler->getArticle(zimFileHandler->getFileheader().getMainPage());
       ns[0] = article.getNamespace();
       titleStr = article.getUrl();
     }
   }

   /* Extract the content from the zim file */
   std::pair<bool, zim::File::const_iterator> resultPair = zimFileHandler->findx(ns[0], titleStr);
   
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
     
     /* Get the data length */
     contentLength = article.getArticleSize();
     
     /* Set return value */
     retVal = true;
   } else {
     /* The found article is not the good one */
     content="";
     contentType="";
     contentLength = 0;
     retVal = false;
   }
   
  return retVal;
}

/* Search titles by prefix*/
 bool Reader::searchSuggestions(const string &prefix, unsigned int suggestionsCount) {

   bool retVal = true;

  /* Reset the suggestions */
  this->suggestions.clear();

  if (prefix.size()) {
    cout << prefix << endl;

    for (zim::File::const_iterator it = zimFileHandler->findByTitle('A', prefix); 
	 it != zimFileHandler->end() && it->getTitle().compare(0, prefix.size(), prefix) == 0 
	   && this->suggestions.size() < suggestionsCount ; ++it) {
      
      this->suggestions.push_back(it->getTitle());

      cout << "  " << it->getTitle() << endl;      
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
  bool retVal = false;

  if (this->suggestionsOffset != this->suggestions.end()) {
    /* title */
    title = *(this->suggestionsOffset);

    /* increment the cursor for the next call */
    this->suggestionsOffset++;

    retVal = true;
  }

  return retVal;
}

  
}
