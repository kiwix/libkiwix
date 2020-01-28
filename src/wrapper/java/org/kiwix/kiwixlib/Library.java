/*
 * Copyright (C) 2019-2020 Matthieu Gautier <mgautier@kymeria.fr>
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

import org.kiwix.kiwixlib.Book;
import org.kiwix.kiwixlib.JNIKiwixException;

public class Library
{
  public native boolean addBook(String path) throws JNIKiwixException;

  public native Book getBookById(String id);
  public native int  getBookCount(boolean localBooks, boolean remoteBooks);

  public native String[] getBooksIds();
  public native String[] filter(Filter filter);

  public native String[] getBooksLanguages();
  public native String[] getBooksCreators();
  public native String[] getBooksPublishers();

  public Library()
  {
    allocate();
  }

  @Override
  protected void finalize() { dispose(); }
  private native void allocate();
  private native void dispose();
  private long nativeHandle;
}
