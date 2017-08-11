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

import org.kiwix.kiwixlib.JNIKiwixString;
import org.kiwix.kiwixlib.JNIKiwixInt;
import org.kiwix.kiwixlib.JNIKiwixSearcher;

public class JNIKiwixReader
{
  public native String getMainPage();

  public native String getTitle();

  public native String getId();

  public native String getLanguage();

  public native String getMimeType(String url);

  public native byte[] getContent(String url,
                                  JNIKiwixString title,
                                  JNIKiwixString mimeType,
                                  JNIKiwixInt size);

  public native boolean searchSuggestions(String prefix, int count);

  public native boolean getNextSuggestion(JNIKiwixString title);

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
    searcher.add_reader(this);
    searcher.search(query, count);
    return searcher;
  }

  public JNIKiwixReader(String filename)
  {
    nativeHandle = getNativeReader(filename);
  }
  public JNIKiwixReader() {

  }
  public native void dispose();

  private native long getNativeReader(String filename);
  private long nativeHandle;
}
