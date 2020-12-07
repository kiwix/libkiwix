/*
 * Copyright (C) 2013 Emmanuel Engelhart <kelson@kiwix.org>
 * Copyright (C) 2017 Matthieu Gautier <mgautier@kymeria.fr>
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

import org.kiwix.kiwixlib.JNIKiwixException;
import org.kiwix.kiwixlib.JNIKiwixString;
import org.kiwix.kiwixlib.JNIKiwixInt;
import org.kiwix.kiwixlib.JNIKiwixSearcher;
import org.kiwix.kiwixlib.DirectAccessInfo;
import java.io.FileDescriptor;

public class JNIKiwixReader
{
  public native String getMainPage();

  public native String getTitle();

  public native String getId();

  public native String getLanguage();

  public native String getMimeType(String url);

  /**
   * Check if a url exists and is a redirect or not.
   *
   * Return an empty string if the url doesn't exist in the reader.
   * Return the url of the "final" entry.
   * - equal to the input url if the entry is not a redirection.
   * - different if the url is a redirection (and the webview should redirect to it).
   */
  public native String checkUrl(String url);

  /**
   * Get the content of a article.
   *
   * Return a byte array of the content of the article.
   * Set the title, mimeType to the title and mimeType of the article.
   * Set the size to the size of the returned array.
   *
   * If the entry doesn't exist :
   *  - return a empty byte array
   *  - set all arguments (except url) to empty/0.
   * If the entry exist but is a redirection :
   *  - return an empty byte array
   *  - set all arguments (including url) to information of the targeted article.
   */
  public native byte[] getContent(JNIKiwixString url,
                                  JNIKiwixString title,
                                  JNIKiwixString mimeType,
                                  JNIKiwixInt size);

  /**
   * getContentPart.
   *
   * Get only a part of the content of the article.
   * Return a byte array of `len` size starting from offset `offset`.
   * Set `size` to the number of bytes read
   * (`len` if everything is ok, 0 in case of error).
   * If `len` == 0, no bytes are read but `size` is set to the total size of the
   * article.
   */
  public native byte[] getContentPart(String url,
                                      int offest,
                                      int len,
                                      JNIKiwixInt size);

  /**
   *
   * Get the size of an article.
   *
   * @param   url The url of the article.
   * @return  The size of the final (redirections are resolved) article (in byte).
   *          Return 0 if the article is not found.
   */
  public native long getArticleSize(String url);

  /**
   * getDirectAccessInformation.
   *
   * Return information giving where the content is located in the zim file.
   *
   * Some contents (binary content) are stored uncompressed in the zim file.
   * Knowing this information, it could be interesting to directly open
   * the zim file (or zim part) and directly read the content from it (and so
   * bypassing the libzim).
   *
   * Return a `DirectAccessInfo` (filename, offset) where the content is located.
   *
   * If the content cannot be directly accessed (content is compressed or zim
   * file is cut in the middle of the content), the filename is an empty string
   * and offset is zero.
   */
  public native DirectAccessInfo getDirectAccessInformation(String url);

  public native boolean searchSuggestions(String prefix, int count);

  public native boolean getNextSuggestion(JNIKiwixString title, JNIKiwixString url);

  public native boolean getPageUrlFromTitle(String title, JNIKiwixString url);

  public native String getDescription();

  public native String getDate();

  public native String getFavicon();

  public native String getCreator();

  public native String getPublisher();

  public native String getName();

  public native int getFileSize();

  public native int getArticleCount();

  public native int getMediaCount();

  public native boolean getRandomPage(JNIKiwixString url);

  public JNIKiwixSearcher search(String query, int count)
  {
    JNIKiwixSearcher searcher = new JNIKiwixSearcher();
    searcher.addKiwixReader(this);
    searcher.search(query, count);
    return searcher;
  }

  public JNIKiwixReader(String filename) throws JNIKiwixException
  {
    nativeHandle = getNativeReader(filename);
    if (nativeHandle == 0) {
        throw new JNIKiwixException("Cannot open zimfile "+filename);
    }
  }

  public JNIKiwixReader(FileDescriptor fd) throws JNIKiwixException
  {
    nativeHandle = getNativeReaderByFD(fd);
    if (nativeHandle == 0) {
        throw new JNIKiwixException("Cannot open zimfile by fd "+fd.toString());
    }
  }

  public JNIKiwixReader(FileDescriptor fd, long offset, long size)
      throws JNIKiwixException
  {
    nativeHandle = getNativeReaderEmbedded(fd, offset, size);
    if (nativeHandle == 0) {
        throw new JNIKiwixException(String.format("Cannot open embedded zimfile (fd=%s, offset=%d, size=%d)", fd, offset, size));
    }
  }

  public JNIKiwixReader() {

  }
  public native void dispose();

  private native long getNativeReader(String filename);
  private native long getNativeReaderByFD(FileDescriptor fd);
  private native long getNativeReaderEmbedded(FileDescriptor fd, long offset, long size);
  private long nativeHandle;
}
