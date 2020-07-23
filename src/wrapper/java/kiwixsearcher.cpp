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


#include "org_kiwix_kiwixlib_JNIKiwixSearcher.h"
#include "org_kiwix_kiwixlib_JNIKiwixSearcher_Result.h"

#include "reader.h"
#include "searcher.h"
#include "utils.h"

#define SEARCHER (Handle<kiwix::Searcher>::getHandle(env, obj))
#define RESULT (Handle<kiwix::Result>::getHandle(env, obj))


JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixSearcher_dispose(JNIEnv* env, jobject obj)
{
  Handle<kiwix::Searcher>::dispose(env, obj);
}

/* Kiwix Reader JNI functions */
JNIEXPORT jlong JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixSearcher_getNativeHandle(JNIEnv* env,
                                                          jobject obj)
{
  kiwix::Searcher* searcher = new kiwix::Searcher();
  return reinterpret_cast<jlong>(new Handle<kiwix::Searcher>(searcher));
}

/* Kiwix library functions */
JNIEXPORT void JNICALL Java_org_kiwix_kiwixlib_JNIKiwixSearcher_addReader(
    JNIEnv* env, jobject obj, jobject reader)
{
  auto searcher = SEARCHER;

  searcher->add_reader(*(Handle<kiwix::Reader>::getHandle(env, reader)));
}

JNIEXPORT void JNICALL Java_org_kiwix_kiwixlib_JNIKiwixSearcher_search(
    JNIEnv* env, jobject obj, jstring query, jint count)
{
  std::string cquery = jni2c(query, env);
  unsigned int ccount = jni2c(count, env);

  SEARCHER->search(cquery, 0, ccount);
}

JNIEXPORT jobject JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixSearcher_getNextResult(JNIEnv* env,
                                                         jobject obj)
{
  jobject result = nullptr;

  kiwix::Result* cresult = SEARCHER->getNextResult();
  if (cresult != nullptr) {
    jclass resultclass
        = env->FindClass("org/kiwix/kiwixlib/JNIKiwixSearcher$Result");
    jmethodID ctor = env->GetMethodID(
        resultclass, "<init>", "(Lorg/kiwix/kiwixlib/JNIKiwixSearcher;JLorg/kiwix/kiwixlib/JNIKiwixSearcher;)V");
    result = env->NewObject(resultclass, ctor, obj, reinterpret_cast<jlong>(new Handle<kiwix::Result>(cresult)), obj);
  }
  return result;
}

JNIEXPORT void JNICALL Java_org_kiwix_kiwixlib_JNIKiwixSearcher_00024Result_dispose(
    JNIEnv* env, jobject obj)
{
  Handle<kiwix::Result>::dispose(env, obj);
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixSearcher_00024Result_getUrl(JNIEnv* env,
                                                        jobject obj)
{
  try {
    return c2jni(RESULT->get_url(), env);
  } catch (...) {
    return nullptr;
  }
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixSearcher_00024Result_getTitle(JNIEnv* env,
                                                          jobject obj)
{
  try {
    return c2jni(RESULT->get_title(), env);
  } catch (...) {
    return nullptr;
  }
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixSearcher_00024Result_getSnippet(JNIEnv* env,
                                                            jobject obj)
{
  return c2jni(RESULT->get_snippet(), env);
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixSearcher_00024Result_getContent(JNIEnv* env,
                                                            jobject obj)
{
  return c2jni(RESULT->get_content(), env);
}
