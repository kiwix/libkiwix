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
#include "org_kiwix_kiwixlib_JNIKiwixServer.h"

#include "tools/base64.h"
#include "server.h"
#include "utils.h"

/* Kiwix Reader JNI functions */
JNIEXPORT jlong JNICALL Java_org_kiwix_kiwixlib_JNIKiwixServer_getNativeServer(
    JNIEnv* env, jobject obj, jobject jLibrary)
{
  LOG("Attempting to create server");
  Lock l;
  try {
    auto library = getPtr<kiwix::Library>(env, jLibrary);
    kiwix::Server* server = new kiwix::Server(library);
    return reinterpret_cast<jlong>(new Handle<kiwix::Server>(server));
  } catch (std::exception& e) {
    LOG("Error creating the server");
    LOG(e.what());
    return 0;
  }
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixServer_dispose(JNIEnv* env, jobject obj)
{
  Handle<kiwix::Server>::dispose(env, obj);
}

#define SERVER (Handle<kiwix::Server>::getHandle(env, obj))

/* Kiwix library functions */
JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixServer_setRoot(JNIEnv* env, jobject obj, jstring jRoot)
{
  std::string root = jni2c(jRoot, env);
  SERVER->setRoot(root);
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixServer_setAddress(JNIEnv* env, jobject obj, jstring jAddress)
{
  std::string address = jni2c(jAddress, env);
  SERVER->setAddress(address);
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixServer_setPort(JNIEnv* env, jobject obj, int port)
{
  SERVER->setPort(port);
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixServer_setNbThreads(JNIEnv* env, jobject obj, int threads)
{
  SERVER->setNbThreads(threads);
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixServer_setTaskbar(JNIEnv* env, jobject obj, jboolean withTaskbar, jboolean withLibraryButton)
{
  SERVER->setTaskbar(withTaskbar, withLibraryButton);
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixServer_setBlockExternalLinks(JNIEnv* env, jobject obj, jboolean blockExternalLinks)
{
  SERVER->setBlockExternalLinks(blockExternalLinks);
}

JNIEXPORT jboolean JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixServer_start(JNIEnv* env, jobject obj)
{
  return SERVER->start();
}

JNIEXPORT void JNICALL
Java_org_kiwix_kiwixlib_JNIKiwixServer_stop(JNIEnv* env, jobject obj)
{
  SERVER->stop();
}
