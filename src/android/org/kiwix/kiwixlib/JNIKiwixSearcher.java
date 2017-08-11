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
    public native String get_url();
    public native String get_title();
    public native String get_content();
    public native void dispose();
  }

  public JNIKiwixSearcher()
  {
    nativeHandle = get_nativeHandle();
    usedReaders = new Vector();
  }
  public native void dispose();

  private native long get_nativeHandle();
  private long nativeHandle;
  private Vector usedReaders;

  public native void _add_reader(JNIKiwixReader reader);
  public void add_reader(JNIKiwixReader reader)
  {
    _add_reader(reader);
    usedReaders.addElement(reader);
  };

  public native void search(String query, int count);

  public native Result get_next_result();
  public native boolean has_more_result();
}
