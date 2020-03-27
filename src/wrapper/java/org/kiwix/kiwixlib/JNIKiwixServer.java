/*
 * Copyright (C) 2019 Matthieu Gautier <mgautier@kymeria.fr>
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
import org.kiwix.kiwixlib.Library;

public class JNIKiwixServer
{
  public native void setRoot(String root);

  public native void setAddress(String address);

  public native void setPort(int port);

  public native void setNbThreads(int nbTreads);

  public native void setTaskbar(boolean withTaskBar, boolean witLibraryButton);

  public native void setBlockExternalLinks(boolean blockExternalLinks);

  public native boolean start();

  public native void stop();

  public JNIKiwixServer(Library library)
  {
    nativeHandle = getNativeServer(library);
  }

  private native long getNativeServer(Library library);
  private long nativeHandle;
}
