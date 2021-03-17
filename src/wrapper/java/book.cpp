/*
 * Copyright (C) 2020 Matthieu Gautier <mgautier@kymeria.fr>
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
#include "org_kiwix_kiwixlib_Book.h"

#include "utils.h"
#include "book.h"

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_Book_allocate(
  JNIEnv* env, jobject thisObj)
{
  allocate<kiwix::Book>(env, thisObj);
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_Book_dispose(JNIEnv* env, jobject thisObj)
{
  dispose<kiwix::Book>(env, thisObj);
}

#define BOOK (getPtr<kiwix::Book>(env, thisObj))

METHOD(void, Book, update__Lorg_kiwix_kiwixlib_Book_2, jobject otherBook)
{
  BOOK->update(*getPtr<kiwix::Book>(env, otherBook));
}

METHOD(void, Book, update__Lorg_kiwix_kiwixlib_JNIKiwixReader_2, jobject reader)
{
  BOOK->update(**Handle<kiwix::Reader>::getHandle(env, reader));
}

#define GETTER(retType, name) JNIEXPORT retType JNICALL \
Java_org_kiwix_kiwixlib_Book_##name (JNIEnv* env, jobject thisObj) \
{ \
  auto cRet = BOOK->name(); \
  retType ret = c2jni(cRet, env); \
  return ret; \
}

GETTER(jstring, getId)
GETTER(jstring, getPath)
GETTER(jboolean, isPathValid)
GETTER(jstring, getTitle)
GETTER(jstring, getDescription)
GETTER(jstring, getLanguage)
GETTER(jstring, getCreator)
GETTER(jstring, getPublisher)
GETTER(jstring, getDate)
GETTER(jstring, getUrl)
GETTER(jstring, getName)
GETTER(jstring, getFlavour)
GETTER(jstring, getCategory)
GETTER(jstring, getTags)
GETTER(jlong, getArticleCount)
GETTER(jlong, getMediaCount)
GETTER(jlong, getSize)
GETTER(jstring, getFavicon)
GETTER(jstring, getFaviconUrl)
GETTER(jstring, getFaviconMimeType)

METHOD(jstring, Book, getTagStr, jstring tagName) try {
  auto cRet = BOOK->getTagStr(jni2c(tagName, env));
  return c2jni(cRet, env);
} catch(...) {
  return c2jni<std::string>("", env);
}

#undef GETTER
