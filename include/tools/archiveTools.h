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

#ifndef KIWIX_ARCHIVETOOLS_H
#define KIWIX_ARCHIVETOOLS_H

#include <zim/archive.h>

/**
 * This file contains all the functions that would make handling data related to
 * an archive easier.
 **/

namespace kiwix
{
    std::string getMetadata(const zim::Archive* const archive, const std::string& name);
    std::string getArchiveTitle(const zim::Archive* const archive);
    std::string getMetaDescription(const zim::Archive* const archive);
    std::string getMetaTags(const zim::Archive* const archive, bool original = false);
    bool getArchiveFavicon(const zim::Archive* const archive,
                           std::string& content, std::string& mimeType);
    std::string getMetaLanguage(const zim::Archive* const archive);
    std::string getMetaName(const zim::Archive* const archive);
    std::string getMetaDate(const zim::Archive* const archive);
    std::string getMetaCreator(const zim::Archive* const archive);
    std::string getMetaPublisher(const zim::Archive* const archive);
    zim::Entry getFinalEntry(const zim::Archive* const archive, const zim::Entry& entry);
}

#endif
