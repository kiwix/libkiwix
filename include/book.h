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
#include "smartptr.h"

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
    const std::string getMimeType() const;
    const std::string getUrl() const;
    const std::string getData() const;

    ~Illustration();

   private:
    Illustration();
    Illustration(const Illustration&) = delete;

    struct Impl;
    ValuePtr<Impl> mp_impl;
  };

  typedef std::vector<std::shared_ptr<const Illustration>> Illustrations;

  enum class AcquisitionLinkKind { DIRECT = 0, MAGNET, META4, BITTORRENT, COUNT };

  // Acquisition URLs indexed by AcquisitionLinkKind (empty = not set).
  // Always holds one entry per link kind; its size cannot be changed.
  class AcquisitionLinks
  {
   public:
    AcquisitionLinks();

    size_t size() const { return m_urls.size(); }
    std::string& operator[](size_t i) { return m_urls[i]; }
    const std::string& operator[](size_t i) const { return m_urls[i]; }

   private:
    std::vector<std::string> m_urls;
  };

 public: // functions
  Book();
  Book(const Book& other);
  Book& operator=(const Book& other);
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
  std::string getId() const;
  std::string getPath() const;
  bool isPathValid() const;
  std::string getTitle() const;
  std::string getDescription() const;
  std::string getCommaSeparatedLanguages() const;
  const std::vector<std::string> getLanguages() const;
  std::string getCreator() const;
  std::string getPublisher() const;
  std::string getDate() const;

  // Returns the acquisition link of the given kind (empty if not set).
  std::string getUrl(AcquisitionLinkKind linkKind) const;
  std::string getName() const;
  std::string getCategory() const;
  std::string getTags() const;
  std::string getTagStr(const std::string& tagName) const;
  bool getTagBool(const std::string& tagName) const;
  std::string getFlavour() const;
  std::string getOrigId() const;
  const uint64_t getArticleCount() const;
  const uint64_t getMediaCount() const;
  const uint64_t getSize() const;

  Illustrations getIllustrations() const;
  std::shared_ptr<const Illustration> getIllustration(unsigned int size) const;

  std::string getDownloadId() const;

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
  // Sets the acquisition link of the given kind.
  // Setting an empty url string clears the link of that kind.
  void setUrl(AcquisitionLinkKind linkKind, const std::string& url);
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

 protected:
  class Impl;
  ValuePtr<Impl> mp_impl;

  // Used as the return value of getDefaultIllustration() when no default
  // illustration is found in the book
  static const Illustration missingDefaultIllustration;
};

}

#endif
