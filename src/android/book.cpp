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

#define METHOD(retType, name) JNIEXPORT retType JNICALL \
Java_org_kiwix_kiwixlib_Book_##name (JNIEnv* env, jobject thisObj) \
{ \
  auto cRet = BOOK->name(); \
  retType ret = c2jni(cRet, env); \
  return ret; \
}

METHOD(jstring, getId)
METHOD(jstring, getPath)
METHOD(jboolean, isPathValid)
METHOD(jstring, getTitle)
METHOD(jstring, getDescription)
METHOD(jstring, getLanguage)
METHOD(jstring, getCreator)
METHOD(jstring, getPublisher)
METHOD(jstring, getDate)
METHOD(jstring, getUrl)
METHOD(jstring, getName)
METHOD(jstring, getTags)
METHOD(jlong, getArticleCount)
METHOD(jlong, getMediaCount)
METHOD(jlong, getSize)
METHOD(jstring, getFavicon)
METHOD(jstring, getFaviconUrl)
METHOD(jstring, getFaviconMimeType)
