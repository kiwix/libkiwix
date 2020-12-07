/*
 * Copyright 2018-2020 Matthieu Gautier <mgautier@kymeria.fr>
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

#ifndef KIWIX_ENTRY_H
#define KIWIX_ENTRY_H

#include <stdio.h>
#include <zim/entry.h>
#include <zim/item.h>
#include <exception>
#include <string>

#include "common.h"

using namespace std;

namespace kiwix
{


class NoEntry : public std::exception {};

/**
 * A entry represent an.. entry in a zim file.
 */
class Entry
{
  public:
    /**
     * Construct an entry making reference to an zim article.
     *
     * @param article a zim::Article object
     */
    Entry(zim::Entry entry);
    virtual ~Entry() = default;

    /**
     * Get the path of the entry.
     *
     * The path is the "key" of an entry.
     *
     * @return the path of the entry.
     */
    std::string getPath() const { return entry.getPath(); }

    /**
     * Get the title of the entry.
     *
     * @return the title of the entry.
     */
    std::string getTitle() const { return entry.getTitle(); }

    /**
     * Get the content of the entry.
     *
     * The string is a copy of the content.
     * If you don't want to do a copy, use get_blob.
     *
     * @return the content of the entry.
     */
    std::string getContent() const { return entry.getItem().getData(); }

    /**
     * Get the blob of the entry.
     *
     * A blob make reference to the content without copying it.
     *
     * @param offset The starting offset of the blob.
     * @return the blob of the entry.
     */
    zim::Blob   getBlob(offset_type offset = 0) const { return entry.getItem().getData(offset); }

    /**
     * Get the blob of the entry.
     *
     * A blob make reference to the content without copying it.
     *
     * @param offset The starting offset of the blob.
     * @param size The size of the blob.
     * @return the blob of the entry.
     */
    zim::Blob   getBlob(offset_type offset, size_type size) const { return entry.getItem().getData(offset, size); }

    /**
     * Get the info for direct access to the content of the entry.
     *
     * Some entry (ie binary ones) have their content plain stored
     * in the zim file. Knowing the offset where the content is stored
     * an user can directly read the content in the zim file bypassing the
     * kiwix-lib/libzim.
     *
     * @return A pair specifying where to read the content.
     *         The string is the real file to read (may be different that .zim
     *         file if zim is cut).
     *         The offset is the offset to read in the file.
     *         Return <"",0> if is not possible to read directly.
     */
    zim::Item::DirectAccessInfo getDirectAccessInfo() const { return entry.getItem().getDirectAccessInformation(); }

    /**
     * Get the size of the entry.
     *
     * @return the size of the entry.
     */
    size_type   getSize() const;

    /**
     * Get the mime_type of the entry.
     *
     * @return the mime_type of the entry.
     */
    std::string getMimetype() const;


    /**
     * Get if the entry is a redirect entry.
     *
     * @return True if the entry is a redirect.
     */
    bool isRedirect() const;

    /**
     * Get if the entry is a link target entry.
     *
     * @return True if the entry is a link target.
     */
    bool isLinkTarget() const;

    /**
     * Get if the entry is a deleted entry.
     *
     * @return True if the entry is a deleted entry.
     */
    bool isDeleted() const;

    /**
     * Get the entry pointed by this entry.
     *
     * @return the entry pointed.
     * @throw NoEntry if the entry is not a redirected entry.
     */
    Entry getRedirectEntry() const;

    /**
     * Get the final entry pointed by this entry.
     *
     * Follow the redirection until a "not redirecting" entry is found.
     * If the entry is not a redirected entry, return the entry itself.
     *
     * @return the final entry.
     */
    Entry getFinalEntry() const;

    /**
     * Get the zim entry wrapped by this (kiwix) entry
     *
     * @return the zim entry
     */
    const zim::Entry& getZimEntry() const { return entry; }

  private:
    zim::Entry entry;
};

}

#endif // KIWIX_ENTRY_H
