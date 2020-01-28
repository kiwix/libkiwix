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


#include <jni.h>
#include "org_kiwix_kiwixlib_Library.h"

#include "library.h"
#include "reader.h"
#include "utils.h"

/* Kiwix Reader JNI functions */
JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_Library_allocate(
    JNIEnv* env, jobject thisObj)
{
  allocate<kiwix::Library>(env, thisObj);
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_Library_dispose(JNIEnv* env, jobject thisObj)
{
  dispose<kiwix::Library>(env, thisObj);
}

#define LIBRARY (getPtr<kiwix::Library>(env, thisObj))

/* Kiwix library functions */
JNIEXPORT jboolean JNICALL
Java_org_kiwix_kiwixlib_Library_addBook(
  JNIEnv* env, jobject thisObj, jstring path)
{
  auto cPath = jni2c(path, env);

  try {
    kiwix::Reader reader(cPath);
    kiwix::Book book;
    book.update(reader);
    return LIBRARY->addBook(book);
  } catch (std::exception& e) {
    LOG("Unable to add the book");
    LOG(e.what()); }
  return false;
}

METHOD(jobject, Library, getBookById, jstring id) {
  auto cId = jni2c(id, env);
  auto cBook = new kiwix::Book(LIBRARY->getBookById(cId));
  jclass cls = env->FindClass("org/kiwix/kiwixlib/Book");
  jmethodID constructorId = env->GetMethodID(cls, "<init>", "()V");
  jobject book = env->NewObject(cls, constructorId);
  setPtr(env, book, cBook);
  return book;
}

METHOD(jint, Library, getBookCount, jboolean localBooks, jboolean remoteBooks) {
  return LIBRARY->getBookCount(localBooks, remoteBooks);
}

METHOD0(jobjectArray, Library, getBooksIds) {
  return c2jni(LIBRARY->getBooksIds(), env);
}

METHOD(jobjectArray, Library, filter, jobject filterObj) {
  auto filter = getPtr<kiwix::Filter>(env, filterObj);
  return c2jni(LIBRARY->filter(*filter), env);
}

METHOD0(jobjectArray, Library, getBooksLanguages) {
  return c2jni(LIBRARY->getBooksLanguages(), env);
}

METHOD0(jobjectArray, Library, getBooksCreators) {
  return c2jni(LIBRARY->getBooksCreators(), env);
}

METHOD0(jobjectArray, Library, getBooksPublisher) {
  return c2jni(LIBRARY->getBooksPublishers(), env);
}

