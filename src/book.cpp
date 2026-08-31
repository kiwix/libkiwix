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
/* Constructor */
Book::Book() :
  m_pathValid(false),
  m_readOnly(false)
{
}

/* Destructor */
Book::~Book()
{
}

Book::Illustrations Book::getIllustrations() const
{
  return m_illustrations;
}

bool Book::update(const kiwix::Book& other)
{
  if (m_readOnly)
    return false;

  if (m_id != other.m_id)
    return false;

  *this = other;
  return true;
}

void Book::update(const zim::Archive& archive) {
  m_path = archive.getFilename();
  m_pathValid = true;
  m_id = std::string(archive.getUuid());
  m_title = getArchiveTitle(archive);
  m_description = getMetaDescription(archive);
  m_language = getMetaLanguage(archive);
  m_creator = getMetaCreator(archive);
  m_publisher = getMetaPublisher(archive);
  m_date = getMetaDate(archive);
  m_name = getMetaName(archive);
  m_flavour = getMetaFlavour(archive);
  m_tags = getMetaTags(archive);
  m_category = getCategoryFromTags();
  m_articleCount = archive.getArticleCount();
  m_mediaCount = archive.getMediaCount();
  m_size = archive.getFilesize();

  m_illustrations.clear();
  for ( const auto& illustrationInfo : archive.getIllustrationInfos() ) {
    const auto illustration = std::make_shared<Illustration>();
    const zim::Item illustrationItem = archive.getIllustrationItem(illustrationInfo);
    illustration->width = illustrationInfo.width;
    illustration->height = illustrationInfo.height;
    illustration->mimeType = illustrationItem.getMimetype();
    illustration->data = illustrationItem.getData();
    // NOTE: illustration->url is left uninitialized
    m_illustrations.push_back(illustration);
  }
}

#define ATTR(name) node.attribute(name).value()
void Book::updateFromXml(const pugi::xml_node& node, const std::string& baseDir)
{
  m_id = ATTR("id");
  std::string path = ATTR("path");
  if (isRelativePath(path)) {
    path = computeAbsolutePath(baseDir, path);
  }
  m_path = path;
  m_pathValid = fileReadable(path);
  m_title = ATTR("title");
  m_description = ATTR("description");
  m_language = ATTR("language");
  m_creator = ATTR("creator");
  m_publisher = ATTR("publisher");
  m_date = ATTR("date");
  m_url = ATTR("url");
  m_name = ATTR("name");
  m_flavour = ATTR("flavour");
  m_tags = ATTR("tags");
  m_origId = ATTR("origId");
  m_articleCount = strtoull(ATTR("articleCount"), 0, 0);
  m_mediaCount = strtoull(ATTR("mediaCount"), 0, 0);
  m_size = strtoull(ATTR("size"), 0, 0) << 10;
  const std::string faviconMimeType = ATTR("faviconMimeType");
  const std::string faviconBase64EncodedData = ATTR("favicon");
  if ( !faviconMimeType.empty() && !faviconBase64EncodedData.empty() ) {
    const auto favicon = std::make_shared<Illustration>();
    favicon->data = base64_decode(faviconBase64EncodedData);
    favicon->mimeType = faviconMimeType;
    favicon->url = ATTR("faviconUrl");
    m_illustrations.assign(1, favicon);
  }
  try {
    m_downloadId = ATTR("downloadId");
  } catch(...) {}
  const auto catattr = node.attribute("category");
  m_category = catattr.empty() ? getCategoryFromTags() : catattr.value();
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
  m_id = VALUE("id");
  if (!m_id.compare(0, 9, "urn:uuid:")) {
    m_id.erase(0, 9);
  }
  m_title = VALUE("title");
  m_description = VALUE("summary");
  m_language = VALUE("language");
  m_creator = node.child("author").child("name").child_value();
  m_publisher = node.child("publisher").child("name").child_value();
  const std::string dcIssuedDate = VALUE("dc:issued");
  m_date = dcIssuedDate.empty() ? VALUE("updated") : dcIssuedDate;
  m_date = fromOpdsDate(m_date);
  m_name = VALUE("name");
  m_flavour = VALUE("flavour");
  m_tags = VALUE("tags");
  const auto catnode = node.child("category");
  m_category = catnode.empty() ? getCategoryFromTags() : catnode.child_value();
  m_articleCount = strtoull(VALUE("articleCount"), 0, 0);
  m_mediaCount = strtoull(VALUE("mediaCount"), 0, 0);
  m_illustrations.clear();
  for(auto linkNode = node.child("link"); linkNode;
           linkNode = linkNode.next_sibling("link")) {
    std::string rel = linkNode.attribute("rel").value();

    if (rel == "http://opds-spec.org/acquisition/open-access") {
      // The href tells us whether this link points at a remote copy of the
      // book (an absolute URL) or a local one (a filesystem path, absolute
      // or relative to baseDir) - a single entry may carry one of each.
      const std::string href = linkNode.attribute("href").value();
      if (isAbsoluteUrl(href)) {
        m_url = href;
      } else {
        m_path = isRelativePath(href)? computeAbsolutePath(baseDir, href): href;
        m_pathValid = fileReadable(m_path);
      }
      const std::string length = linkNode.attribute("length").value();
      if (!length.empty()) {
        m_size = strtoull(length.c_str(), 0, 0);
      }
    }
    if (rel == "http://opds-spec.org/image/thumbnail") {
      const auto favicon = std::make_shared<Illustration>();
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
        m_illustrations.push_back(favicon);
      }
    }
  }
}
#undef VALUE

std::string Book::getHumanReadableIdFromPath() const
{
  std::string id = m_path;
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
 m_path = isRelativePath(path)
   ? computeAbsolutePath(getCurrentDirectory(), path)
   : path;
}

const Book::Illustration Book::missingDefaultIllustration;

std::shared_ptr<const Book::Illustration> Book::getIllustration(unsigned int size) const
{
  for ( const auto& ilPtr : m_illustrations ) {
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
  return getTagValueFromTagList(convertTags(m_tags), tagName);
}

bool Book::getTagBool(const std::string& tagName) const {
  return convertStrToBool(getTagStr(tagName));
}

std::string Book::getCategory() const
{
  return m_category;
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
  return kiwix::split(m_language, ",");
}

}
