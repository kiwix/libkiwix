/*
 * Copyright (C) 2021 Kiwix
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
package org.kiwix.kiwixlib;
class OpdsUriUtills {
    /**
     * Returns the author list of the object
     **/
    public native List<Author> getTheAuthorsList();


    /**
     * returns the [Uri] of the given tags
     *
     * @param author   the author name
     * @param category which category they want to search
     * @param tags  which tags they want to search
     * @return url of the given query
     **/
    public native String getUrlFromTags(String author, String...languageList, String...categories);


    /**
     *  URL of the file to download e.g http://download.kiwix.org/foobar.zim
     * @param book which book they want to download
     * @returns the url of zim file
     */
    public native String getTheDownlaodUrlOfBook(Book book);
}