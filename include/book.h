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

#ifndef KIWIX_BOOK_H
#define KIWIX_BOOK_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "common.h"

namespace pugi {
class xml_node;
}

namespace zim {
class Archive;
}

namespace kiwix
{

class OPDSDumper;

/**
 * A class to store information about a book (a zim file)
 */
class Book
{
 public: // types
  class Illustration
  {
    friend class Book;
   public:
    uint16_t getWidth() const;
    uint16_t getHeight() const;
    const std::string& getMimeType() const;
    const std::string& getUrl() const;
    const std::string& getData() const;

   private:
    Illustration() = default;

    uint16_t width = 48;
    uint16_t height = 48;
    std::string mimeType;
    std::string url;
    mutable std::string data;
    mutable std::mutex mutex;
  };

  typedef std::vector<std::shared_ptr<const Illustration>> Illustrations;

 public: // functions
  Book();
  Book(const Book& other);
  Book(Book&& other) noexcept;
  Book& operator=(const Book& other);
  Book& operator=(Book&& other) noexcept;
  ~Book();

  bool update(const Book& other);
  void update(const zim::Archive& archive);
  void updateFromXml(const pugi::xml_node& node, const std::string& baseDir);
  /**
   * Update the book's metadata from an OPDS entry XML node.
   *
   * @param node the `<entry>` node of an OPDS feed describing the book.
   * @param urlHost host to prepend to relative illustration/thumbnail URLs.
   * @param baseDir base directory used to resolve a relative local-path
   *                acquisition link (`rel="http://opds-spec.org/acquisition/
   *                open-access"` with a non-URL href) into an absolute book
   *                path.
   */
  void updateFromOpds(const pugi::xml_node& node, const std::string& urlHost, const std::string& baseDir);

  /**
   * Update the book's metadata from an OPDS entry XML node.
   *
   * A simple wrapper around the three-parameter updateFromOpds() above, kept
   * for backward compatibility. Equivalent to calling
   * updateFromOpds(node, urlHost, "").
   *
   * @param node the `<entry>` node of an OPDS feed describing the book.
   * @param urlHost host to prepend to relative illustration/thumbnail URLs.
   */
  void updateFromOpds(const pugi::xml_node& node, const std::string& urlHost);
  std::string getHumanReadableIdFromPath() const;

  bool readOnly() const;
  const std::string& getId() const;
  const std::string& getPath() const;
  bool isPathValid() const;
  const std::string& getTitle() const;
  const std::string& getDescription() const;
  DEPRECATED const std::string& getLanguage() const;
  const std::string& getCommaSeparatedLanguages() const;
  const std::vector<std::string> getLanguages() const;
  const std::string& getCreator() const;
  const std::string& getPublisher() const;
  const std::string& getDate() const;
  const std::string& getUrl() const;
  const std::string& getName() const;
  std::string getCategory() const;
  const std::string& getTags() const;
  std::string getTagStr(const std::string& tagName) const;
  bool getTagBool(const std::string& tagName) const;
  const std::string& getFlavour() const;
  const std::string& getOrigId() const;
  const uint64_t getArticleCount() const;
  const uint64_t getMediaCount() const;
  const uint64_t getSize() const;
  DEPRECATED const std::string& getFavicon() const;
  DEPRECATED const std::string& getFaviconUrl() const;
  DEPRECATED const std::string& getFaviconMimeType() const;

  Illustrations getIllustrations() const;
  std::shared_ptr<const Illustration> getIllustration(unsigned int size) const;

  const std::string& getDownloadId() const;

  void setReadOnly(bool readOnly);
  void setId(const std::string& id);
  void setPath(const std::string& path);
  void setPathValid(bool valid);
  void setTitle(const std::string& title);
  void setDescription(const std::string& description);
  void setLanguage(const std::string& language);
  void setCreator(const std::string& creator);
  void setPublisher(const std::string& publisher);
  void setDate(const std::string& date);
  void setUrl(const std::string& url);
  void setName(const std::string& name);
  void setFlavour(const std::string& flavour);
  void setTags(const std::string& tags);
  void setOrigId(const std::string& origId);
  void setArticleCount(uint64_t articleCount);
  void setMediaCount(uint64_t mediaCount);
  void setSize(uint64_t size);
  void setDownloadId(const std::string& downloadId);

 private: // functions
  std::string getCategoryFromTags() const;
  const Illustration& getDefaultIllustration() const;

 private: // data
  class Impl;
  std::unique_ptr<Impl> mp_impl;

  // Used as the return value of getDefaultIllustration() when no default
  // illustration is found in the book
  static const Illustration missingDefaultIllustration;
};

}

#endif
