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

#include <jni.h>
#include "org_kiwix_kiwixlib_JNIICU.h"

#include <iostream>
#include <string>

#include "unicode/putil.h"

#include "utils.h"

std::mutex globalLock;

JNIEXPORT void JNICALL Java_org_kiwix_kiwixlib_JNIICU_setDataDirectory(
    JNIEnv* env, jclass kclass, jstring dirStr)
{
  std::string cPath = jni2c(dirStr, env);

  Lock l;
  try {
    u_setDataDirectory(cPath.c_str());
  } catch (...) {
    std::cerr << "Unable to set data directory " << cPath << std::endl;
  }
}
