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

#include "book.h"

#include "tools.h"
#include "tools/base64.h"
#include "tools/regexTools.h"
#include "tools/networkTools.h"
#include "tools/otherTools.h"
#include "tools/stringTools.h"
#include "tools/archiveTools.h"

#include <zim/archive.h>
#include <zim/item.h>
#include <pugixml.hpp>

#include <sstream>
#include <cctype>

namespace
{
/**
 * Tells whether a URL string is already absolute (contains a scheme,
 * e.g. "https://example.com/x.png") as opposed to being a relative path
 * (e.g. "/x.png"). Only a relative URL should be prefixed with a base host
 * or URL—doing so unconditionally would garble an already-absolute one.
 */
bool isAbsoluteUrl(const std::string& url)
{
  // Find the scheme separator
  size_t pos = url.find("://");
  if (pos == 0 || pos == std::string::npos) {
    return false; // No scheme or empty scheme
  }

  // RFC 3986: Scheme must begin with a letter, followed by letters, digits, '+', '.', or '-'
  if (!std::isalpha(static_cast<unsigned char>(url[0]))) {
    return false;
  }

  // Validate that all remaining characters in the scheme comply with RFC 3986
  for (size_t i = 1; i < pos; ++i) {
    char c = url[i];
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '+' && c != '-' && c != '.') {
      return false;
    }
  }

  return true;
}

/**
 * Joins a host and a reference into a single URL without producing a double slash,
 * respecting the host's format.
 */
std::string joinUrl(const std::string& host, const std::string& ref)
{
  if (host.empty()) {
    return ref;
  }
  if (ref.empty()) {
    return host;
  }

  const bool hostEndsWithSlash = (host.back() == '/');
  const bool refStartsWithSlash = (ref.front() == '/');

  if (hostEndsWithSlash && refStartsWithSlash) {
    // Both have a slash; omit one to avoid a double slash
    return host + ref.substr(1);
  } else if (!hostEndsWithSlash && !refStartsWithSlash) {
    // Neither has a slash; insert one
    return host + "/" + ref;
  } else {
    // Exactly one has a slash; simple concatenation is correct
    return host + ref;
  }
}

/**
 * Splits an OPDS thumbnail link's "type" attribute value into its base MIME
 * type and its "width"/"height" parameters, mirroring the
 * "<mimetype>;width=<w>;height=<h>;scale=<s>" convention that
 * getIllustrationMimeTypeStr() (library_dumper.cpp) writes on the way out.
 * "scale" is parsed away but otherwise unused, since Illustration has no
 * such field. A width/height left at 0 (i.e. absent from the type string)
 * means "unspecified" and must not overwrite the Illustration's own default.
 */
struct ParsedIllustrationType
{
  std::string mimeType;
  uint16_t width = 0;
  uint16_t height = 0;
};

ParsedIllustrationType parseIllustrationType(const std::string& type)
{
  ParsedIllustrationType result;
  const auto parts = kiwix::split(type, ";");
  if (parts.empty()) {
    return result;
  }

  const std::string potentialMime = kiwix::trim(parts[0]);

  // A basic check: MIME types typically contain a slash (e.g., "image/jpeg")
  if (potentialMime.find('/') != std::string::npos) {
    result.mimeType = potentialMime;
  }

  for (auto it = parts.begin() + 1; it != parts.end(); ++it) {
    const auto eqPos = it->find('=');
    if (eqPos == std::string::npos) {
      continue;
    }

    // Trim both key and value to handle spaces safely (e.g., " width=100")
    const std::string key = kiwix::trim(it->substr(0, eqPos));
    const std::string value = kiwix::trim(it->substr(eqPos + 1));

    if (!value.empty()) {
      const uint16_t numericValue =
        static_cast<uint16_t>(strtoul(value.c_str(), nullptr, 10));

      if (key == "width") {
        result.width = numericValue;
      } else if (key == "height") {
        result.height = numericValue;
      }
    }
  }

  return result;
}

} // anonymous namespace

namespace kiwix
{

class Book::Impl
{
 public:
  std::string m_id;
  std::string m_downloadId;
  std::string m_path;
  bool m_pathValid = false;
  std::string m_title;
  std::string m_description;
  std::string m_category;
  std::string m_language;
  std::string m_creator;
  std::string m_publisher;
  std::string m_date;
  std::string m_url;
  std::string m_name;
  std::string m_flavour;
  std::string m_tags;
  std::string m_origId;
  uint64_t m_articleCount = 0;
  uint64_t m_mediaCount = 0;
  bool m_readOnly = false;
  uint64_t m_size = 0;
  Book::Illustrations m_illustrations;
};

/* Constructor */
Book::Book() :
  mp_impl(new Impl())
{
}

Book::Book(const Book& other) :
  mp_impl(new Impl(*other.mp_impl))
{
}

Book::Book(Book&& other) noexcept = default;

Book& Book::operator=(const Book& other)
{
  *mp_impl = *other.mp_impl;
  return *this;
}

Book& Book::operator=(Book&& other) noexcept = default;

/* Destructor */
Book::~Book() = default;

Book::Illustrations Book::getIllustrations() const
{
  return mp_impl->m_illustrations;
}

bool Book::update(const kiwix::Book& other)
{
  if (mp_impl->m_readOnly)
    return false;

  if (mp_impl->m_id != other.mp_impl->m_id)
    return false;

  *this = other;
  return true;
}

void Book::update(const zim::Archive& archive) {
  mp_impl->m_path = archive.getFilename();
  mp_impl->m_pathValid = true;
  mp_impl->m_id = std::string(archive.getUuid());
  mp_impl->m_title = getArchiveTitle(archive);
  mp_impl->m_description = getMetaDescription(archive);
  mp_impl->m_language = getMetaLanguage(archive);
  mp_impl->m_creator = getMetaCreator(archive);
  mp_impl->m_publisher = getMetaPublisher(archive);
  mp_impl->m_date = getMetaDate(archive);
  mp_impl->m_name = getMetaName(archive);
  mp_impl->m_flavour = getMetaFlavour(archive);
  mp_impl->m_tags = getMetaTags(archive);
  mp_impl->m_category = getCategoryFromTags();
  mp_impl->m_articleCount = archive.getArticleCount();
  mp_impl->m_mediaCount = archive.getMediaCount();
  mp_impl->m_size = archive.getFilesize();

  mp_impl->m_illustrations.clear();
  for ( const auto& illustrationInfo : archive.getIllustrationInfos() ) {
    const auto illustration = std::shared_ptr<Illustration>(new Illustration());
    const zim::Item illustrationItem = archive.getIllustrationItem(illustrationInfo);
    illustration->width = illustrationInfo.width;
    illustration->height = illustrationInfo.height;
    illustration->mimeType = illustrationItem.getMimetype();
    illustration->data = illustrationItem.getData();
    // NOTE: illustration->url is left uninitialized
    mp_impl->m_illustrations.push_back(illustration);
  }
}

#define ATTR(name) node.attribute(name).value()
void Book::updateFromXml(const pugi::xml_node& node, const std::string& baseDir)
{
  mp_impl->m_id = ATTR("id");
  std::string path = ATTR("path");
  if (isRelativePath(path)) {
    path = computeAbsolutePath(baseDir, path);
  }
  mp_impl->m_path = path;
  mp_impl->m_pathValid = fileReadable(path);
  mp_impl->m_title = ATTR("title");
  mp_impl->m_description = ATTR("description");
  mp_impl->m_language = ATTR("language");
  mp_impl->m_creator = ATTR("creator");
  mp_impl->m_publisher = ATTR("publisher");
  mp_impl->m_date = ATTR("date");
  mp_impl->m_url = ATTR("url");
  mp_impl->m_name = ATTR("name");
  mp_impl->m_flavour = ATTR("flavour");
  mp_impl->m_tags = ATTR("tags");
  mp_impl->m_origId = ATTR("origId");
  mp_impl->m_articleCount = strtoull(ATTR("articleCount"), 0, 0);
  mp_impl->m_mediaCount = strtoull(ATTR("mediaCount"), 0, 0);
  mp_impl->m_size = strtoull(ATTR("size"), 0, 0) << 10;
  const std::string faviconMimeType = ATTR("faviconMimeType");
  const std::string faviconBase64EncodedData = ATTR("favicon");
  if ( !faviconMimeType.empty() && !faviconBase64EncodedData.empty() ) {
    const auto favicon = std::shared_ptr<Illustration>(new Illustration());
    favicon->data = base64_decode(faviconBase64EncodedData);
    favicon->mimeType = faviconMimeType;
    favicon->url = ATTR("faviconUrl");
    mp_impl->m_illustrations.assign(1, favicon);
  }
  try {
    mp_impl->m_downloadId = ATTR("downloadId");
  } catch(...) {}
  const auto catattr = node.attribute("category");
  mp_impl->m_category = catattr.empty() ? getCategoryFromTags() : catattr.value();
}
#undef ATTR


static std::string fromOpdsDate(const std::string& date)
{
  //The opds date use the standard <YYYY>-<MM>-<DD>T<HH>:<mm>:<SS>Z
  //and we want <YYYY>-<MM>-<DD>. That's easy, let's take the first 10 char
  return date.substr(0, 10);
}


void Book::updateFromOpds(const pugi::xml_node& node, const std::string& urlHost)
{
  updateFromOpds(node, urlHost, "");
}

#define VALUE(name) node.child(name).child_value()
void Book::updateFromOpds(const pugi::xml_node& node, const std::string& urlHost, const std::string& baseDir)
{
  mp_impl->m_id = VALUE("id");
  if (!mp_impl->m_id.compare(0, 9, "urn:uuid:")) {
    mp_impl->m_id.erase(0, 9);
  }
  mp_impl->m_title = VALUE("title");
  mp_impl->m_description = VALUE("summary");
  mp_impl->m_language = VALUE("language");
  mp_impl->m_creator = node.child("author").child("name").child_value();
  mp_impl->m_publisher = node.child("publisher").child("name").child_value();
  const std::string dcIssuedDate = VALUE("dc:issued");
  mp_impl->m_date = dcIssuedDate.empty() ? VALUE("updated") : dcIssuedDate;
  mp_impl->m_date = fromOpdsDate(mp_impl->m_date);
  mp_impl->m_name = VALUE("name");
  mp_impl->m_flavour = VALUE("flavour");
  mp_impl->m_tags = VALUE("tags");
  const auto catnode = node.child("category");
  mp_impl->m_category = catnode.empty() ? getCategoryFromTags() : catnode.child_value();
  mp_impl->m_articleCount = strtoull(VALUE("articleCount"), 0, 0);
  mp_impl->m_mediaCount = strtoull(VALUE("mediaCount"), 0, 0);
  mp_impl->m_illustrations.clear();
  std::string firstAcquisitionHref;
  std::string firstLength;
  for(auto linkNode = node.child("link"); linkNode;
           linkNode = linkNode.next_sibling("link")) {
    std::string rel = linkNode.attribute("rel").value();

    if (rel == "http://opds-spec.org/acquisition/open-access") {
      // The href tells us whether this link points at a remote copy of the
      // book (an absolute URL) or a local one (a filesystem path, absolute
      // or relative to baseDir) - a single entry may carry one of each.
      const std::string href = linkNode.attribute("href").value();
      if (isAbsoluteUrl(href)) {
        mp_impl->m_url = href;
      } else {
        mp_impl->m_path = isRelativePath(href)? computeAbsolutePath(baseDir, href): href;
        mp_impl->m_pathValid = fileReadable(mp_impl->m_path);
      }
      const std::string length = linkNode.attribute("length").value();
      if (!length.empty()) {
        if (!firstLength.empty() && length != firstLength) {
          std::cerr << "Book '" << mp_impl->m_id << "': acquisition links '"
                    << firstAcquisitionHref << "' (length " << firstLength
                    << ") and '" << href << "' (length " << length
                    << ") disagree on length." << std::endl;
        }
        mp_impl->m_size = strtoull(length.c_str(), 0, 0);
        firstAcquisitionHref = href;
        firstLength = length;
      }
    }
    if (rel == "http://opds-spec.org/image/thumbnail") {
      const auto favicon = std::shared_ptr<Illustration>(new Illustration());
      const std::string thumbnailUrl = linkNode.attribute("href").value();
      if (startsWith(thumbnailUrl, "data:")) {
        // OPDS 1.2's "data" URL scheme (spec 5.2.2): the payload is
        // whatever follows the first comma, regardless of what media-type
        // text (if any) precedes it - the link's own "type" attribute is
        // authoritative for that.
        const auto commaPos = thumbnailUrl.find(',');
        if (commaPos != std::string::npos) {
          favicon->data = base64_decode(thumbnailUrl.substr(commaPos + 1));
        }
      } else {
        // XXX non-absolute URL is expected to be an absolute-path.
        favicon->url = isAbsoluteUrl(thumbnailUrl)? thumbnailUrl: joinUrl(urlHost, thumbnailUrl);
      }
      const auto parsedType = parseIllustrationType(linkNode.attribute("type").value());
      favicon->mimeType = parsedType.mimeType;
      if (parsedType.width) {
        favicon->width = parsedType.width;
      }
      if (parsedType.height) {
        favicon->height = parsedType.height;
      }
      if (!favicon->mimeType.empty()) {
        mp_impl->m_illustrations.push_back(favicon);
      }
    }
  }
}
#undef VALUE

std::string Book::getHumanReadableIdFromPath() const
{
  std::string id = mp_impl->m_path;
  if (!id.empty()) {
    id = kiwix::removeAccents(id);

#ifdef _WIN32
    id = replaceRegex(id, "", "^.*\\\\");
#else
    id = replaceRegex(id, "", "^.*/");
#endif

    id = replaceRegex(id, "", "\\.zim[a-z]*$");
    id = replaceRegex(id, "_", " ");
    id = replaceRegex(id, "plus", "\\+");
  }
  return id;
}

void Book::setPath(const std::string& path)
{
 mp_impl->m_path = isRelativePath(path)
   ? computeAbsolutePath(getCurrentDirectory(), path)
   : path;
}

const Book::Illustration Book::missingDefaultIllustration;

std::shared_ptr<const Book::Illustration> Book::getIllustration(unsigned int size) const
{
  for ( const auto& ilPtr : mp_impl->m_illustrations ) {
    if (ilPtr->width == size && ilPtr->height == size) {
      return ilPtr;
    }
  }
  throw std::runtime_error("Cannot find illustration");
}

const Book::Illustration& Book::getDefaultIllustration() const
{
  try {
    return *getIllustration(48);
  } catch (...) {
    return missingDefaultIllustration;
  }
}

uint16_t Book::Illustration::getWidth() const
{
  return width;
}

uint16_t Book::Illustration::getHeight() const
{
  return height;
}

const std::string& Book::Illustration::getMimeType() const
{
  return mimeType;
}

const std::string& Book::Illustration::getUrl() const
{
  return url;
}

const std::string& Book::Illustration::getData() const
{
  if (data.empty() && !url.empty()) {
    const std::lock_guard<std::mutex> l(mutex);
    if ( data.empty() ) {
      try {
        data = download(url);
      } catch(...) {
        std::cerr << "Cannot download favicon from " << url << std::endl;
      }
    }
  }
  return data;
}

const std::string& Book::getFavicon() const {
  return getDefaultIllustration().getData();
}

const std::string& Book::getFaviconUrl() const
{
  return getDefaultIllustration().url;
}

const std::string& Book::getFaviconMimeType() const
{
  return getDefaultIllustration().mimeType;
}

std::string Book::getTagStr(const std::string& tagName) const {
  return getTagValueFromTagList(convertTags(mp_impl->m_tags), tagName);
}

bool Book::getTagBool(const std::string& tagName) const {
  return convertStrToBool(getTagStr(tagName));
}

std::string Book::getCategory() const
{
  return mp_impl->m_category;
}

std::string Book::getCategoryFromTags() const
{
  try
  {
    return getTagStr("category");
  }
  catch ( const std::out_of_range& )
  {
    return "";
  }
}

const std::vector<std::string> Book::getLanguages() const
{
  return kiwix::split(mp_impl->m_language, ",");
}

bool Book::readOnly() const { return mp_impl->m_readOnly; }
const std::string& Book::getId() const { return mp_impl->m_id; }
const std::string& Book::getPath() const { return mp_impl->m_path; }
bool Book::isPathValid() const { return mp_impl->m_pathValid; }
const std::string& Book::getTitle() const { return mp_impl->m_title; }
const std::string& Book::getDescription() const { return mp_impl->m_description; }
const std::string& Book::getLanguage() const { return mp_impl->m_language; }
const std::string& Book::getCommaSeparatedLanguages() const { return mp_impl->m_language; }
const std::string& Book::getCreator() const { return mp_impl->m_creator; }
const std::string& Book::getPublisher() const { return mp_impl->m_publisher; }
const std::string& Book::getDate() const { return mp_impl->m_date; }
const std::string& Book::getUrl() const { return mp_impl->m_url; }
const std::string& Book::getName() const { return mp_impl->m_name; }
const std::string& Book::getTags() const { return mp_impl->m_tags; }
const std::string& Book::getFlavour() const { return mp_impl->m_flavour; }
const std::string& Book::getOrigId() const { return mp_impl->m_origId; }
const uint64_t Book::getArticleCount() const { return mp_impl->m_articleCount; }
const uint64_t Book::getMediaCount() const { return mp_impl->m_mediaCount; }
const uint64_t Book::getSize() const { return mp_impl->m_size; }
const std::string& Book::getDownloadId() const { return mp_impl->m_downloadId; }

void Book::setReadOnly(bool readOnly) { mp_impl->m_readOnly = readOnly; }
void Book::setId(const std::string& id) { mp_impl->m_id = id; }
void Book::setPathValid(bool valid) { mp_impl->m_pathValid = valid; }
void Book::setTitle(const std::string& title) { mp_impl->m_title = title; }
void Book::setDescription(const std::string& description) { mp_impl->m_description = description; }
void Book::setLanguage(const std::string& language) { mp_impl->m_language = language; }
void Book::setCreator(const std::string& creator) { mp_impl->m_creator = creator; }
void Book::setPublisher(const std::string& publisher) { mp_impl->m_publisher = publisher; }
void Book::setDate(const std::string& date) { mp_impl->m_date = date; }
void Book::setUrl(const std::string& url) { mp_impl->m_url = url; }
void Book::setName(const std::string& name) { mp_impl->m_name = name; }
void Book::setFlavour(const std::string& flavour) { mp_impl->m_flavour = flavour; }
void Book::setTags(const std::string& tags) { mp_impl->m_tags = tags; }
void Book::setOrigId(const std::string& origId) { mp_impl->m_origId = origId; }
void Book::setArticleCount(uint64_t articleCount) { mp_impl->m_articleCount = articleCount; }
void Book::setMediaCount(uint64_t mediaCount) { mp_impl->m_mediaCount = mediaCount; }
void Book::setSize(uint64_t size) { mp_impl->m_size = size; }
void Book::setDownloadId(const std::string& downloadId) { mp_impl->m_downloadId = downloadId; }

}
