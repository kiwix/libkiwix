/*
 * Copyright 2021 Maneesh P M <manu.pm55@gmail.com>
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

#include <tools/archiveTools.h>
#include <tools/pathTools.h>
#include <tools/otherTools.h>
#include <tools/stringTools.h>

#include <zim/error.h>
#include <zim/item.h>

namespace kiwix
{
std::string getMetadata(const zim::Archive* const archive, const std::string& name) {
    try {
        return archive->getMetadata(name);
    } catch (zim::EntryNotFound& e) {
        return "";
    }
}

std::string getArchiveTitle(const zim::Archive* const archive) {
  std::string value = getMetadata(archive, "Title");
  if (value.empty()) {
    value = getLastPathElement(archive->getFilename());
    std::replace(value.begin(), value.end(), '_', ' ');
    size_t pos = value.find(".zim");
    value = value.substr(0, pos);
  }
  return value;
}

std::string getMetaDescription(const zim::Archive* const archive) {
  std::string value;
  value = getMetadata(archive, "Description");

  /* Mediawiki Collection tends to use the "Subtitle" name */
  if (value.empty()) {
    value = getMetadata(archive, "Subtitle");
  }

  return value;
}

std::string getMetaTags(const zim::Archive* const archive, bool original) {
  std::string tags_str = getMetadata(archive, "Tags");
  if (original) {
    return tags_str;
  }
  auto tags = convertTags(tags_str);
  return join(tags, ";");
}

bool getArchiveFavicon(const zim::Archive* const archive,
                           std::string& content, std::string& mimeType){
  try {
    auto entry = archive->getFaviconEntry();
    auto item = entry.getItem(true);
    content = item.getData();
    mimeType = item.getMimetype();
    return true;
  } catch(zim::EntryNotFound& e) {};

  return false;
}

std::string getMetaLanguage(const zim::Archive* const archive) {
  return getMetadata(archive, "Language");
}

std::string getMetaName(const zim::Archive* const archive) {
  return getMetadata(archive, "Name");
}

std::string getMetaDate(const zim::Archive* const archive) {
  return getMetadata(archive, "Date");
}

std::string getMetaCreator(const zim::Archive* const archive) {
  return getMetadata(archive, "Creator");
}

std::string getMetaPublisher(const zim::Archive* const archive) {
  return getMetadata(archive, "Publisher");
}

} // kiwix
