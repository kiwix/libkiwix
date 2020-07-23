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
#include "org_kiwix_kiwixlib_Manager.h"

#include "manager.h"
#include "utils.h"


JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_Manager_allocate(
  JNIEnv* env, jobject thisObj, jobject libraryObj)
{
  auto lib = getPtr<kiwix::Library>(env, libraryObj);
  allocate<kiwix::Manager>(env, thisObj, lib);
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_Manager_dispose(JNIEnv* env, jobject thisObj)
{
  dispose<kiwix::Manager>(env, thisObj);
}

#define MANAGER (getPtr<kiwix::Manager>(env, thisObj))

/* Kiwix manager functions */
JNIEXPORT jboolean JNICALL
Java_org_kiwix_kiwixlib_Manager_readFile(
  JNIEnv* env, jobject thisObj, jstring path)
{
  auto cPath = jni2c(path, env);

  try {
    return MANAGER->readFile(cPath);
  } catch (std::exception& e) {
    LOG("Unable to get readFile");
    LOG(e.what());
  }
  return false;
}

JNIEXPORT jboolean JNICALL
Java_org_kiwix_kiwixlib_Manager_readXml(
  JNIEnv* env, jobject thisObj, jstring content, jstring libraryPath)
{
  auto cContent = jni2c(content, env);
  auto cPath = jni2c(libraryPath, env);

  try {
    return MANAGER->readXml(cContent, false, cPath);
  } catch (std::exception& e) {
    LOG("Unable to get ZIM id");
    LOG(e.what());
  }

  return false;
}

JNIEXPORT jboolean JNICALL
Java_org_kiwix_kiwixlib_Manager_readOpds(
  JNIEnv* env, jobject thisObj, jstring content, jstring urlHost)
{
  auto cContent = jni2c(content, env);
  auto cUrl = jni2c(urlHost, env);

  try {
    return MANAGER->readOpds(cContent, cUrl);
  } catch (std::exception& e) {
    LOG("Unable to get ZIM id");
    LOG(e.what());
  }

  return false;
}

JNIEXPORT jboolean JNICALL
Java_org_kiwix_kiwixlib_Manager_readBookmarkFile(
  JNIEnv* env, jobject thisObj, jstring path)
{
  auto cPath = jni2c(path, env);

  try {
    return MANAGER->readBookmarkFile(cPath);
  } catch (std::exception& e) {
    LOG("Unable to get ZIM id");
    LOG(e.what());
  }

  return false;
}

JNIEXPORT jstring JNICALL
Java_org_kiwix_kiwixlib_Manager_addBookFromPath(
  JNIEnv* env, jobject thisObj,
  jstring pathToOpen, jstring pathToSave, jstring url, jboolean checkMetaData)
{
  auto cPathToOpen = jni2c(pathToOpen, env);
  auto cPathToSave = jni2c(pathToSave, env);
  auto cUrl = jni2c(url, env);
  jstring id = NULL;

  try {
    auto cId = MANAGER->addBookFromPathAndGetId(cPathToOpen, cPathToSave, cUrl, checkMetaData);
    if ( !cId.empty() ) {
      id = c2jni(cId, env);
    }
  } catch (std::exception& e) {
    LOG("Unable to get ZIM file size");
    LOG(e.what());
  }

  return id;
}
