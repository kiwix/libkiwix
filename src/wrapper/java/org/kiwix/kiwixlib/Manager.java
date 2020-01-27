/*
 * Copyright (C) 2020 Matthieu Gautier <mgautier@kymeria.fr>
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

import org.kiwix.kiwixlib.Library;

public class Manager
{
  /**
   * Read a `library.xml` file and add books in the library.
   *
   * @param path The (utf8) path to the `library.xml` file.
   * @return True if the file has been properly parsed.
   */
  public native boolean readFile(String path);

  /**
   * Load a library content stored in a string (at `library.xml` format).
   *
   * @param content The content corresponding of the library xml.
   * @param libraryPath The library path (used to resolve relative paths)
   * @return True if the content has been properly parsed.
   */
  public native boolean readXml(String content, String libraryPath);

  /**
   * Load a library content stored in a string (at OPDS stream format)
   *
   * @param content the content of the OPDS stream.
   * @param urlHost the url of the stream (used to resolve relative url)
   * @return True if the content has been properly parsed.
   */
  public native boolean readOpds(String content, String urlHost);

  /**
   * Load a bookmark file
   *
   * @param path The path of the file to read.
   * @return True if the content has been properly parsed
   */
  public native boolean readBookmarkFile(String path);

  /**
   * Add a book to the library.
   *
   * @param pathToOpen    The path of the zim file to add.
   * @param pathToSave    The path to store in the library in place of
   *                      pathToOpen.
   * @param url           The url of the book to store in the library
   *                      (useful for kiiwix-serve catalog)
   * @param checkMetaData Tell if we check metadata before adding a book to the
   *                      library.
   * @return The id of te book if the book has been added to the library.
   *         Empty string if not.
   */
  public native String addBookFromPath(String pathToOpen,
                                       String pathToSave,
                                       String url,
                                       boolean checkMetaData);

  public Manager(Library library) {
    allocate(library);
    _library = library;
  }

  private Library _library;

  @Override
  protected void finalize() { dispose(); }
  private native void allocate(Library library);
  private native void dispose();
  private long nativeHandle;
}
