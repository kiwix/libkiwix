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

public class Filter
{

  public native Filter local(boolean accept);
  public native Filter remote(boolean accept);
  public native Filter valid(boolean accept);
  public native Filter acceptTags(String[] tags);
  public native Filter rejectTags(String[] tags);
  public native Filter lang(String lang);
  public native Filter publisher(String publisher);
  public native Filter creator(String creator);
  public native Filter maxSize(long size);
  public native Filter query(String query);


  public Filter() { allocate(); }

  @Override
  protected void finalize() { dispose(); }
  private native void allocate();
  private native void dispose();
  private long nativeHandle;
}
