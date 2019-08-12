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
#include "reader.h"

#include "tools/base64.h"
#include "tools/regexTools.h"
#include "tools/networkTools.h"

#include <pugixml.hpp>

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

bool Book::update(const kiwix::Book& other)
{
  if (m_readOnly)
    return false;

  m_readOnly = other.m_readOnly;

  if (m_path.empty()) {
    m_path = other.m_path;
    m_pathValid = other.m_pathValid;
  }

  if (m_url.empty()) {
    m_url = other.m_url;
  }

  if (m_tags.empty()) {
    m_tags = other.m_tags;
  }

  if (m_name.empty()) {
    m_name = other.m_name;
  }

  if (m_faviconMimeType.empty()) {
    m_favicon = other.m_favicon;
    m_faviconMimeType = other.m_faviconMimeType;
  }
  return true;
}

void Book::update(const kiwix::Reader& reader)
{
  m_path = reader.getZimFilePath();
  m_id = reader.getId();
  m_description = reader.getDescription();
  m_language = reader.getLanguage();
  m_date = reader.getDate();
  m_creator = reader.getCreator();
  m_publisher = reader.getPublisher();
  m_title = reader.getTitle();
  m_name = reader.getName();
  m_tags = reader.getTags();
  m_origId = reader.getOrigId();
  m_articleCount = reader.getArticleCount();
  m_mediaCount = reader.getMediaCount();
  m_size = static_cast<uint64_t>(reader.getFileSize()) << 10;
  m_pathValid = true;

  reader.getFavicon(m_favicon, m_faviconMimeType);
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
  m_title = ATTR("title");
  m_name = ATTR("name");
  m_tags = ATTR("tags");
  m_description = ATTR("description");
  m_language = ATTR("language");
  m_date = ATTR("date");
  m_creator = ATTR("creator");
  m_publisher = ATTR("publisher");
  m_url = ATTR("url");
  m_origId = ATTR("origId");
  m_articleCount = strtoull(ATTR("articleCount"), 0, 0);
  m_mediaCount = strtoull(ATTR("mediaCount"), 0, 0);
  m_size = strtoull(ATTR("size"), 0, 0) << 10;
  m_favicon = base64_decode(ATTR("favicon"));
  m_faviconMimeType = ATTR("faviconMimeType");
  try {
    m_downloadId = ATTR("downloadId");
  } catch(...) {}
}
#undef ATTR


static std::string fromOpdsDate(const std::string& date)
{
  //The opds date use the standard <YYYY>-<MM>-<DD>T<HH>:<mm>:<SS>Z
  //and we want <YYYY>-<MM>-<DD>. That's easy, let's take the first 10 char
  return date.substr(0, 10);
}


#define VALUE(name) node.child(name).child_value()
void Book::updateFromOpds(const pugi::xml_node& node, const std::string& urlHost)
{
  m_id = VALUE("id");
  if (!m_id.compare(0, 9, "urn:uuid:")) {
    m_id.erase(0, 9);
  }
  m_title = VALUE("title");
  m_description = VALUE("description");
  m_language = VALUE("language");
  m_date = fromOpdsDate(VALUE("updated"));
  m_creator = node.child("author").child("name").child_value();
  m_tags = VALUE("tags");
  for(auto linkNode = node.child("link"); linkNode;
           linkNode = linkNode.next_sibling("link")) {
    std::string rel = linkNode.attribute("rel").value();

    if (rel == "http://opds-spec.org/acquisition/open-access") {
      m_url = linkNode.attribute("href").value();
      m_size = strtoull(linkNode.attribute("length").value(), 0, 0);
    }
    if (rel == "http://opds-spec.org/image/thumbnail") {
      m_faviconUrl = urlHost + linkNode.attribute("href").value();
      m_faviconMimeType = linkNode.attribute("type").value();
    }
 }

}
#undef VALUE

std::string Book::getHumanReadableIdFromPath() const
{
  std::string id = m_path;
  if (!id.empty()) {
    kiwix::removeAccents(id);

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

const std::string& Book::getFavicon() const {
  if (m_favicon.empty() && !m_faviconUrl.empty()) {
    try {
      m_favicon = download(m_faviconUrl);
    } catch(...) {
      std::cerr << "Cannot download favicon from " << m_faviconUrl;
    }
  }
  return m_favicon;
}

}
