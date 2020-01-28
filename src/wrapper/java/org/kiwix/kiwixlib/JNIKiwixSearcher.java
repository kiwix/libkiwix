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

import org.kiwix.kiwixlib.JNIKiwixReader;
import java.util.Vector;

public class JNIKiwixSearcher
{
  public class Result
  {
    private long nativeHandle;
    private JNIKiwixSearcher searcher;
    public Result(long handle, JNIKiwixSearcher _searcher)
    {
      nativeHandle = handle;
      searcher = _searcher;
    }
    public native String getUrl();
    public native String getTitle();
    public native String getContent();
    public native String getSnippet();
    public native void dispose();
  }

  public JNIKiwixSearcher()
  {
    nativeHandle = getNativeHandle();
    usedReaders = new Vector();
  }
  public native void dispose();

  private native long getNativeHandle();
  private long nativeHandle;
  private Vector usedReaders;

  public native void addReader(JNIKiwixReader reader);
  public void addKiwixReader(JNIKiwixReader reader)
  {
    addReader(reader);
    usedReaders.addElement(reader);
  };

  public native void search(String query, int count);

  public native Result getNextResult();
  public native boolean hasMoreResult();
}
