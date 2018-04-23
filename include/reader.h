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

#include <stdio.h>
#include <zim/article.h>
#include <zim/file.h>
#include <zim/fileiterator.h>
#include <zim/zim.h>
#include <exception>
#include <map>
#include <sstream>
#include <string>
#include "common.h"
#include "entry.h"
#include "common/pathTools.h"
#include "common/stringTools.h"

using namespace std;

namespace kiwix
{

/**
 * The Reader class is the class who allow to get an entry content from a zim
 * file.
 */
class Reader
{
 public:
  /**
   * Create a Reader to read a zim file specified by zimFilePath.
   *
   * @param zimFilePath The path to the zim file to read.
   *                    The zim file can be splitted (.zimaa, .zimab, ...).
   *                    In this case, the file path must still point to the
   *                    unsplitted path as if the file were not splitted
   *                    (.zim extesion).
   */
  Reader(const string zimFilePath);
  ~Reader();

  /**
   * Get the number of "displayable" entries in the zim file.
   *
   * @return If the zim file has a /M/Counter metadata, return the number of
   *         entries with the 'text/html' MIMEtype specified in the metadata.
   *         Else return the number of entries in the 'A' namespace.
   */
  unsigned int getArticleCount() const;

  /**
   * Get the number of media in the zim file.
   *
   * @return If the zim file has a /M/Counter metadata, return the number of
   *         entries with the 'image/jpeg', 'image/gif' and 'image/png' in
   *         the metadata.
   *         Else return the number of entries in the 'I' namespace.
   */
  unsigned int getMediaCount() const;

  /**
   * Get the number of all entries in the zim file.
   *
   * @return Return the number of all the entries, whatever their MIMEtype or
   *         their namespace.
   */
  unsigned int getGlobalCount() const;

  /**
   * Get the path of the zim file.
   *
   * @return the path of the zim file as given in the constructor.
   */
  string getZimFilePath() const;

  /**
   * Get the Id of the zim file.
   *
   * @return The uuid stored in the zim file.
   */
  string getId() const;

  /**
   * Get the url of a random page.
   *
   * Deprecated : Use `getRandomPage` instead.
   *
   * @return Url of a random page. The page is picked from all entries in
   *         the 'A' namespace.
   *         The main page is excluded from the potential results.
   */
  DEPRECATED string getRandomPageUrl() const;

  /**
   * Get a random page.
   *
   * @return A random Entry. The entry is picked from all entries in
   *         the 'A' namespace.
   *         The main entry is excluded from the potential results.
   */
  Entry getRandomPage() const;

  /**
   * Get the url of the first page.
   *
   * Deprecated : Use `getFirstPage` instead.
   *
   * @return Url of the first entry in the 'A' namespace.
   */
  DEPRECATED string getFirstPageUrl() const;

  /**
   * Get the entry of the first page.
   *
   * @return The first entry in the 'A' namespace.
   */
  Entry getFirstPage() const;

  /**
   * Get the url of the main page.
   *
   * Deprecated : Use `getMainPage` instead.
   *
   * @return Url of the main page as specified in the zim file.
   */
  DEPRECATED string getMainPageUrl() const;

  /**
   * Get the entry of the main page.
   *
   * @return Entry of the main page as specified in the zim file.
   */
  Entry getMainPage() const;

  /**
   * Get the content of a metadata.
   *
   * @param[in]  name The name of the metadata.
   * @param[out] value The value will be set to the content of the metadata.
   * @return True if it was possible to get the content of the metadata.
   */
  bool getMetatag(const string& name, string& value) const;

  /**
   * Get the title of the zim file.
   *
   * @return The title of zim file as specified in the zim metadata.
   *         If no title has been set, return a title computed from the
   *         file path.
   */
  string getTitle() const;

  /**
   * Get the description of the zim file.
   *
   * @return The description of the zim file as specified in the zim metadata.
   *         If no description has been set, return the subtitle.
   */
  string getDescription() const;

  /**
   * Get the language of the zim file.
   *
   * @return The language of the zim file as specified in the zim metadata.
   */
  string getLanguage() const;

  /**
   * Get the name of the zim file.
   *
   * @return The name of the zim file as specified in the zim metadata.
   */
  string getName() const;

  /**
   * Get the tags of the zim file.
   *
   * @return The tags of the zim file as specified in the zim metadata.
   */
  string getTags() const;

  /**
   * Get the date of the zim file.
   *
   * @return The date of the zim file as specified in the zim metadata.
   */
  string getDate() const;

  /**
   * Get the creator of the zim file.
   *
   * @return The creator of the zim file as specified in the zim metadata.
   */
  string getCreator() const;

  /**
   * Get the publisher of the zim file.
   *
   * @return The publisher of the zim file as specified in the zim metadata.
   */
  string getPublisher() const;

  /**
   * Get the origId of the zim file.
   *
   * The origId is only used in the case of patch zim file and is the Id
   * of the original zim file.
   *
   * @return The origId of the zim file as specified in the zim metadata.
   */
  string getOrigId() const;

  /**
   * Get the favicon of the zim file.
   *
   * @param[out] content The content of the favicon.
   * @param[out] mimeType The mimeType of the favicon.
   * @return True if a favicon has been found.
   */
  bool getFavicon(string& content, string& mimeType) const;

  /**
   * Get an entry associated to an path.
   *
   * @param path The path of the entry.
   * @return The entry.
   * @throw NoEntry If no entry correspond to the path.
   */
  Entry getEntryFromPath(const std::string& path) const;

  /**
   * Get an entry associated to an url encoded path.
   *
   * Equivalent to `getEntryFromPath(urlDecode(path));`
   *
   * @param path The url encoded path.
   * @return The entry.
   * @throw NoEntry If no entry correspond to the path.
   */
  Entry getEntryFromEncodedPath(const std::string& path) const;

  /**
   * Get un entry associated to a title.
   *
   * @param title The title.
   * @return The entry
   * throw NoEntry If no entry correspond to the url.
   */
  Entry getEntryFromTitle(const std::string& title) const;

  /**
   * Get the url of a page specified by a title.
   *
   * @param[in] title the title of the page.
   * @param[out] url the url of the page.
   * @return True if the page can be found.
   */
  DEPRECATED bool getPageUrlFromTitle(const string& title, string& url) const;

  /**
   * Get the mimetype of a entry specified by a url.
   *
   * @param[in] url the url of the entry.
   * @param[out] mimeType the mimeType of the entry.
   * @return True if the mimeType has been found.
   */
  DEPRECATED bool getMimeTypeByUrl(const string& url, string& mimeType) const;

  /**
   * Get the content of an entry specifed by a url.
   *
   * Alias to `getContentByEncodedUrl`
   */
  DEPRECATED bool getContentByUrl(const string& url,
                       string& content,
                       string& title,
                       unsigned int& contentLength,
                       string& contentType) const;

  /**
   * Get the content of an entry specified by a url encoded url.
   *
   * Equivalent to getContentByDecodedUrl(urlDecode(url), ...).
   */
  DEPRECATED bool getContentByEncodedUrl(const string& url,
                              string& content,
                              string& title,
                              unsigned int& contentLength,
                              string& contentType,
                              string& baseUrl) const;

  /**
   * Get the content of an entry specified by an url encoded url.
   *
   * Equivalent to getContentByEncodedUrl but without baseUrl.
   */
  DEPRECATED bool getContentByEncodedUrl(const string& url,
                              string& content,
                              string& title,
                              unsigned int& contentLength,
                              string& contentType) const;

  /**
   * Get the content of an entry specified by a url.
   *
   * @param[in] url The url of the entry.
   * @param[out] content The content of the entry.
   * @param[out] title the title of the entry.
   * @param[out] contentLength The size of the entry (size of content).
   * @param[out] contentType The mimeType of the entry.
   * @param[out] baseUrl Return the true url of the entry.
   *                     If the specified entry is a redirection, contains
   *                     the url of the targeted entry.
   * @return True if the entry has been found.
   */
  DEPRECATED bool getContentByDecodedUrl(const string& url,
                              string& content,
                              string& title,
                              unsigned int& contentLength,
                              string& contentType,
                              string& baseUrl) const;
  /**
   * Get the content of an entry specified by a url.
   *
   * Equivalent to getContentByDecodedUrl but withou the baseUrl.
   */
  DEPRECATED bool getContentByDecodedUrl(const string& url,
                              string& content,
                              string& title,
                              unsigned int& contentLength,
                              string& contentType) const;

  /**
   * Search for entries with title starting with prefix (case sensitive).
   *
   * Suggestions are stored in an internal vector and can be retrieved using
   * `getNextSuggestion` method.
   *
   * @param prefix The prefix to search.
   * @param suggestionsCount How many suggestions to search for.
   * @param reset If true, remove previous suggestions in the internal vector.
   *              If false, add suggestions to the internal vector
   *              (until internal vector size is suggestionCount (or no more
   *               suggestion))
   * @return True if some suggestions where added to the internal vector.
   */
  bool searchSuggestions(const string& prefix,
                         unsigned int suggestionsCount,
                         const bool reset = true);

  /**
   * Search for entries for the given prefix.
   *
   * If the zim file has a internal fulltext index, the suggestions will be
   * searched using it.
   * Else the suggestions will be search using `searchSuggestions` while trying
   * to be smart about case sensitivity (using `getTitleVariants`).
   *
   * In any case, suggestions are stored in an internal vector and can be
   * retrieved using `getNextSuggestion` method.
   * The internal vector will be reset.
   *
   * @param prefix The prefix to search for.
   * @param suggestionsCount How many suggestions to search for.
   */
  bool searchSuggestionsSmart(const string& prefix,
                              unsigned int suggestionsCount);

  /**
   * Check if the url exists in the zim file.
   *
   * Deprecated : Use `pathExists` instead.
   *
   * @param url the url to check.
   * @return True if the url exits in the zim file.
   */
  DEPRECATED bool urlExists(const string& url) const;

  /**
   * Check if the path exists in the zim file.
   *
   * @param path the path to check.
   * @return True if the path exists in the zim file.
   */
  bool pathExists(const string& path) const;

  /**
   * Check if the zim file has a embedded fulltext index.
   *
   * @return True if the zim file has a embedded fulltext index
   *         and is not split (else the fulltext is not accessible).
   */
  bool hasFulltextIndex() const;

  /**
   * Get potential case title variations for a title.
   *
   * @param title a title.
   * @return the list of variantions.
   */
  std::vector<std::string> getTitleVariants(const std::string& title) const;

  /**
   * Get the next suggestion title.
   *
   * @param[out] title the title of the suggestion.
   * @return True if title has been set.
   */
  bool getNextSuggestion(string& title);

  /**
   * Get the next suggestion title and url.
   *
   * @param[out] title the title of the suggestion.
   * @param[out] url the url of the suggestion.
   * @return True if title and url have been set.
   */
  bool getNextSuggestion(string& title, string& url);

  /**
   * Get if we can check zim file integrity (has a checksum).
   *
   * @return True if zim file have a checksum.
   */
  bool canCheckIntegrity() const;

  /**
   * Check is zim file is corrupted.
   *
   * @return True if zim file is corrupted.
   */
  bool isCorrupted() const;

  /**
   * Parse a full url into a namespace and url.
   *
   * @param[in] url The full url ("/N/url").
   * @param[out] ns The namespace (N).
   * @param[out] title The url (url).
   * @return True
   */
  DEPRECATED bool parseUrl(const string& url, char* ns, string& title) const;

  /**
   * Return the total size of the zim file.
   *
   * If zim file is split, return the sum of all parts' size.
   *
   * @return Size of the size file is KiB.
   */
  unsigned int getFileSize() const;

  /**
   * Get the zim file handler.
   *
   * @return The libzim file handler.
   */
  zim::File* getZimFileHandler() const;

  /**
   * Get the zim article object associated to a url.
   *
   * @param[in] url The url of the article.
   * @param[out] article The libzim article object.
   * @return True if the url is good (article.good()).
   */
  DEPRECATED bool getArticleObjectByDecodedUrl(const string& url,
                                    zim::Article& article) const;

 protected:
  zim::File* zimFileHandler;
  zim::size_type firstArticleOffset;
  zim::size_type lastArticleOffset;
  zim::size_type nsACount;
  zim::size_type nsICount;
  std::string zimFilePath;

  std::vector<std::vector<std::string>> suggestions;
  std::vector<std::vector<std::string>>::iterator suggestionsOffset;

 private:
  std::map<const std::string, unsigned int> parseCounterMetadata() const;
};
}

#endif
