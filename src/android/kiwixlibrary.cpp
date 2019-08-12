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
#include <android/log.h>
#include "org_kiwix_kiwixlib_JNIKiwixLibrary.h"

#include "library.h"
#include "reader.h"
#include "utils.h"

/* Kiwix Reader JNI functions */
JNIEXPORT jlong JNICALL Java_org_kiwix_kiwixlib_JNIKiwixLibrary_getNativeLibrary(
    JNIEnv* env, jobject obj)
{
  __android_log_print(ANDROID_LOG_INFO, "kiwix", "Attempting to create library");
  Lock l;
  try {
    kiwix::Library* library = new kiwix::Library();
    return reinterpret_cast<jlong>(new Handle<kiwix::Library>(library));
  } catch (std::exception& e) {
    __android_log_print(ANDROID_LOG_WARN, "kiwix", "Error creating ZIM library");
    __android_log_print(ANDROID_LOG_WARN, "kiwix", e.what());
    return 0;
  }
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixLibrary_dispose(JNIEnv* env, jobject obj)
{
  Handle<kiwix::Library>::dispose(env, obj);
}

#define LIBRARY (Handle<kiwix::Library>::getHandle(env, obj))

/* Kiwix library functions */
JNIEXPORT jboolean JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixLibrary_addBook(JNIEnv* env, jobject obj, jstring path)
{
  std::string cPath = jni2c(path, env);
  bool ret;

  try {
    kiwix::Reader reader(cPath);
    kiwix::Book book;
    book.update(reader);
    ret = LIBRARY->addBook(book);
  } catch (std::exception& e) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to add the book");
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", e.what());
    ret = false;
  }
  return ret;
}
