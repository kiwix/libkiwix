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

#ifndef KIWIX_READER_H
#define KIWIX_READER_H

#include <zim/zim.h>
#include <zim/file.h>
#include <zim/article.h>
#include <zim/fileiterator.h>
#include <stdio.h>
#include <string>
#include <exception>
#include <sstream>
#include <map>
#include "common/pathTools.h"
#include "common/stringTools.h"

using namespace std;

namespace kiwix {

  class Reader {

  public:
    Reader(const string zimFilePath);
    ~Reader();

    void reset();
    unsigned int getArticleCount() const;
    unsigned int getMediaCount() const;
    unsigned int getGlobalCount() const;
    string getZimFilePath() const;
    string getId() const;
    string getRandomPageUrl() const;
    string getFirstPageUrl() const;
    string getMainPageUrl() const;
    bool getMetatag(const string &url, string &content) const;
    string getTitle() const;
    string getDescription() const;
    string getLanguage() const;
    string getName() const;
    string getTags() const;
    string getDate() const;
    string getCreator() const;
    string getPublisher() const;
    string getOrigId() const;
    bool getFavicon(string &content, string &mimeType) const;
    bool getPageUrlFromTitle(const string &title, string &url) const;
    bool getMimeTypeByUrl(const string &url, string &mimeType) const;
    bool getContentByUrl(const string &url, string &content, unsigned int &contentLength, string &contentType) const;
    bool getContentByEncodedUrl(const string &url, string &content, unsigned int &contentLength, string &contentType, string &baseUrl) const;
    bool getContentByEncodedUrl(const string &url, string &content, unsigned int &contentLength, string &contentType) const;
    bool getContentByDecodedUrl(const string &url, string &content, unsigned int &contentLength, string &contentType, string &baseUrl) const;
    bool getContentByDecodedUrl(const string &url, string &content, unsigned int &contentLength, string &contentType) const;
    bool searchSuggestions(const string &prefix, unsigned int suggestionsCount, const bool reset = true);
    bool searchSuggestionsSmart(const string &prefix, unsigned int suggestionsCount);
    bool urlExists(const string &url) const;
    bool hasFulltextIndex() const;
    std::vector<std::string> getTitleVariants(const std::string &title) const;
    bool getNextSuggestion(string &title);
    bool getNextSuggestion(string &title, string &url);
    bool canCheckIntegrity() const;
    bool isCorrupted() const;
    bool parseUrl(const string &url, char *ns, string &title) const;
    unsigned int getFileSize() const;
    zim::File* getZimFileHandler() const;
    bool getArticleObjectByDecodedUrl(const string &url, zim::Article &article) const;

  protected:
    zim::File* zimFileHandler;
    zim::size_type firstArticleOffset;
    zim::size_type lastArticleOffset;
    zim::size_type currentArticleOffset;
    zim::size_type nsACount;
    zim::size_type nsICount;
    std::string zimFilePath;
    
    std::vector< std::vector<std::string> > suggestions;
    std::vector< std::vector<std::string> >::iterator suggestionsOffset;

  private:
    std::map<const std::string, unsigned int> parseCounterMetadata() const;
  };

}

#endif
