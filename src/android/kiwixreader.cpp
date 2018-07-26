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
#include <zim/file.h>
#include <android/log.h>
#include "org_kiwix_kiwixlib_JNIKiwixReader.h"

#include "common/base64.h"
#include "reader.h"
#include "utils.h"

/* Kiwix Reader JNI functions */
JNIEXPORT jlong JNICALL Java_org_kiwix_kiwixlib_JNIKiwixReader_getNativeReader(
    JNIEnv* env, jobject obj, jstring filename)
{
  std::string cPath = jni2c(filename, env);

  __android_log_print(ANDROID_LOG_INFO, "kiwix", "Attempting to create reader with: %s", cPath.c_str());
  Lock l;
  try {
    kiwix::Reader* reader = new kiwix::Reader(cPath);
    return reinterpret_cast<jlong>(new Handle<kiwix::Reader>(reader));
  } catch (...) {
    __android_log_print(ANDROID_LOG_WARN, "kiwix", "Error opening ZIM file");
    return 0;
  }
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_dispose(JNIEnv* env, jobject obj)
{
  Handle<kiwix::Reader>::dispose(env, obj);
}

#define READER (Handle<kiwix::Reader>::getHandle(env, obj))

/* Kiwix library functions */
JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getMainPage(JNIEnv* env, jobject obj)
{
  jstring url;

  try {
    std::string cUrl = READER->getMainPage().getPath();
    url = c2jni(cUrl, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get ZIM main page");
    url = NULL;
  }
  return url;
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getId(JNIEnv* env, jobject obj)
{
  jstring id;

  try {
    std::string cId = READER->getId();
    id = c2jni(cId, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get ZIM id");
    id = NULL;
  }

  return id;
}

JNIEXPORT jint JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getFileSize(JNIEnv* env, jobject obj)
{
  jint size;

  try {
    int cSize = READER->getFileSize();
    size = c2jni(cSize);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get ZIM file size");
  }

  return size;
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getCreator(JNIEnv* env, jobject obj)
{
  jstring creator;

  try {
    std::string cCreator = READER->getCreator();
    creator = c2jni(cCreator, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get ZIM creator");
    creator = NULL;
  }

  return creator;
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getPublisher(JNIEnv* env, jobject obj)
{
  jstring publisher;

  try {
    std::string cPublisher = READER->getPublisher();
    publisher = c2jni(cPublisher, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get ZIM publish");
    publisher = NULL;
  }
  return publisher;
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getName(JNIEnv* env, jobject obj)
{
  jstring name;

  try {
    std::string cName = READER->getName();
    name = c2jni(cName, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get ZIM name");
    name = NULL;
  }
  return name;
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getFavicon(JNIEnv* env, jobject obj)
{
  jstring favicon;

  try {
    std::string cContent;
    std::string cMime;
    READER->getFavicon(cContent, cMime);
    favicon = c2jni(
        base64_encode(reinterpret_cast<const unsigned char*>(cContent.c_str()),
                      cContent.length()),
        env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get ZIM favicon");
    favicon = NULL;
  }
  return favicon;
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getDate(JNIEnv* env, jobject obj)
{
  jstring date;

  try {
    std::string cDate = READER->getDate();
    date = c2jni(cDate, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get ZIM date");
    date = NULL;
  }
  return date;
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getLanguage(JNIEnv* env, jobject obj)
{
  jstring language;

  try {
    std::string cLanguage = READER->getLanguage();
    language = c2jni(cLanguage, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get ZIM language");
    language = NULL;
  }

  return language;
}

JNIEXPORT jstring JNICALL Java_org_kiwix_kiwixlib_JNIKiwixReader_getMimeType(
    JNIEnv* env, jobject obj, jstring url)
{
  jstring mimeType;

  std::string cUrl = jni2c(url, env);
  try {
    auto entry = READER->getEntryFromEncodedPath(cUrl);
    auto cMimeType = entry.getMimetype();
    mimeType = c2jni(cMimeType, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get mime-type for url: %s", cUrl.c_str());
    mimeType = NULL;
  }
  return mimeType;
}

JNIEXPORT jbyteArray JNICALL Java_org_kiwix_kiwixlib_JNIKiwixReader_getContent(
    JNIEnv* env, jobject obj, jstring url, jobject titleObj, jobject mimeTypeObj, jobject sizeObj)
{
  /* Default values */
  setStringObjValue("", titleObj, env);
  setStringObjValue("", mimeTypeObj, env);
  setIntObjValue(0, sizeObj, env);
  jbyteArray data = env->NewByteArray(0);

  /* Retrieve the content */
  std::string cUrl = jni2c(url, env);
  unsigned int cSize = 0;

  try {
    auto entry = READER->getEntryFromEncodedPath(cUrl);
    entry = entry.getFinalEntry();
    cSize = entry.getSize();
    setIntObjValue(cSize, sizeObj, env);

    data = env->NewByteArray(cSize);
    env->SetByteArrayRegion(
        data, 0, cSize, reinterpret_cast<const jbyte*>(entry.getBlob().data()));

    setStringObjValue(entry.getMimetype(), mimeTypeObj, env);
    setStringObjValue(entry.getTitle(), titleObj, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get content for url: %s", cUrl.c_str());
  }

  return data;
}

JNIEXPORT jbyteArray JNICALL Java_org_kiwix_kiwixlib_JNIKiwixReader_getContentPart(
    JNIEnv* env, jobject obj, jstring url, jint offset, jint len, jobject sizeObj)
{
  jbyteArray data = env->NewByteArray(0);
  setIntObjValue(0, sizeObj, env);

  /* Default values */
  /* Retrieve the content */
  std::string cUrl = jni2c(url, env);
  unsigned int cOffset = jni2c(offset);
  unsigned int cLen = jni2c(len);
  try {
    auto entry = READER->getEntryFromEncodedPath(cUrl);
    entry = entry.getFinalEntry();

    if (cLen == 0) {
      setIntObjValue(entry.getSize(), sizeObj, env);
    } else if (cOffset+cLen < entry.getSize()) {
      auto blob = entry.getBlob(cOffset, cLen);
      data = env->NewByteArray(cLen);
      env->SetByteArrayRegion(
          data, 0, cLen, reinterpret_cast<const jbyte*>(blob.data()));
      setIntObjValue(cLen, sizeObj, env);
    }
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get partial content for url: %s (%u : %u)", cUrl.c_str(), cOffset, cLen);
  }
  return data;
}

JNIEXPORT jobject JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getDirectAccessInformation(
    JNIEnv* env, jobject obj, jstring url)
{
   jclass classPair = env->FindClass("org/kiwix/kiwixlib/Pair");
   jmethodID midPairinit = env->GetMethodID(classPair, "<init>", "()V");
   jobject pair = env->NewObject(classPair, midPairinit);
   setPairObjValue("", 0, pair, env);

   std::string cUrl = jni2c(url, env);
   try {
    auto entry = READER->getEntryFromEncodedPath(cUrl);
    entry = entry.getFinalEntry();
    auto part_info = entry.getDirectAccessInfo();
    setPairObjValue(part_info.first, part_info.second, pair, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get direct access info for url: %s", cUrl.c_str());
  }
  return pair;
}

JNIEXPORT jboolean JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_searchSuggestions(JNIEnv* env,
                                                         jobject obj,
                                                         jstring prefix,
                                                         jint count)
{
  jboolean retVal = JNI_FALSE;
  std::string cPrefix = jni2c(prefix, env);
  unsigned int cCount = jni2c(count);

  try {
    if (READER->searchSuggestionsSmart(cPrefix, cCount)) {
      retVal = JNI_TRUE;
    }
  } catch (...) {
    __android_log_print(ANDROID_LOG_WARN, "kiwix", "Unable to get search results for pattern: %s", cPrefix.c_str());
  }

  return retVal;
}

JNIEXPORT jboolean JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getNextSuggestion(JNIEnv* env,
                                                         jobject obj,
                                                         jobject titleObj)
{
  jboolean retVal = JNI_FALSE;
  std::string cTitle;

  try {
    if (READER->getNextSuggestion(cTitle)) {
      setStringObjValue(cTitle, titleObj, env);
      retVal = JNI_TRUE;
    }
  } catch (...) {
    __android_log_print(ANDROID_LOG_WARN, "kiwix", "Unable to get next suggestion");
  }

  return retVal;
}

JNIEXPORT jboolean JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getPageUrlFromTitle(JNIEnv* env,
                                                           jobject obj,
                                                           jstring title,
                                                           jobject urlObj)
{
  std::string cTitle = jni2c(title, env);

  try {
    auto entry = READER->getEntryFromTitle(cTitle);
    entry = entry.getFinalEntry();
    setStringObjValue(entry.getPath(), urlObj, env);
    return JNI_TRUE;
  } catch (...) {
    __android_log_print(ANDROID_LOG_WARN, "kiwix", "Unable to get url for title %s: ", cTitle.c_str());
  }

  return JNI_FALSE;
}

JNIEXPORT jstring JNICALL Java_org_kiwix_kiwixlib_JNIKiwixReader_getTitle(
    JNIEnv* env, jobject obj)
{
  jstring title;

  try {
    std::string cTitle = READER->getTitle();
    title = c2jni(cTitle, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get zim title");
    title = NULL;
  }
  return title;
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixReader_getDescription(JNIEnv* env, jobject obj)
{
  jstring description;

  try {
    std::string cDescription = READER->getDescription();
    description = c2jni(cDescription, env);
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get zim description");
    description = NULL;
  }
  return description;
}

JNIEXPORT jboolean JNICALL Java_org_kiwix_kiwixlib_JNIKiwixReader_getRandomPage(
    JNIEnv* env, jobject obj, jobject urlObj)
{
  jboolean retVal = JNI_FALSE;
  std::string cUrl;

  try {
    std::string cUrl = READER->getRandomPage().getPath();
    setStringObjValue(cUrl, urlObj, env);
    retVal = JNI_TRUE;
  } catch (...) {
    __android_log_print(ANDROID_LOG_ERROR, "kiwix", "Unable to get random page");
  }
  return retVal;
}
