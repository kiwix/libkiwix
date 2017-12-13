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


#ifndef _ANDROID_JNI_UTILS_H
#define _ANDROID_JNI_UTILS_H

#include <jni.h>

#include <pthread.h>
#include <string>

extern pthread_mutex_t globalLock;

inline jfieldID getHandleField(JNIEnv* env, jobject obj)
{
  jclass c = env->GetObjectClass(obj);
  // J is the type signature for long:
  return env->GetFieldID(c, "nativeHandle", "J");
}

class Lock
{
 protected:
  pthread_mutex_t* lock;

 public:
  Lock() : lock(&globalLock) { pthread_mutex_lock(lock); }
  Lock(const Lock&) = delete;
  Lock& operator=(const Lock&) = delete;
  Lock(Lock&& other) : lock(&globalLock) { other.lock = nullptr; }
  virtual ~Lock()
  {
    if (lock) {
      pthread_mutex_unlock(lock);
    }
  }
};

template <class T>
class LockedHandle;

template <class T>
class Handle
{
 protected:
  T* h;

 public:
  Handle(T* h) : h(h){};

  // No destructor. This must and will be handled by dispose method.

  static LockedHandle<T> getHandle(JNIEnv* env, jobject obj)
  {
    jlong handle = env->GetLongField(obj, getHandleField(env, obj));
    return LockedHandle<T>(reinterpret_cast<Handle<T>*>(handle));
  }

  static void dispose(JNIEnv* env, jobject obj)
  {
    auto lHandle = getHandle(env, obj);
    auto handle = lHandle.h;
    delete handle->h;
    delete handle;
  }
  friend class LockedHandle<T>;
};

template <class T>
struct LockedHandle : public Lock {
  Handle<T>* h;
  LockedHandle(Handle<T>* h) : h(h) {}
  T* operator->() { return h->h; }
  T* operator*() { return h->h; }
  operator bool() const { return (h->h != nullptr); }
};

/* c2jni type conversion functions */
inline jboolean c2jni(const bool& val) { return val ? JNI_TRUE : JNI_FALSE; }
inline jstring c2jni(const std::string& val, JNIEnv* env)
{
  return env->NewStringUTF(val.c_str());
}

inline jint c2jni(const int val) { return (jint)val; }
inline jint c2jni(const unsigned val) { return (unsigned)val; }
/* jni2c type conversion functions */
inline bool jni2c(const jboolean& val) { return val == JNI_TRUE; }
inline std::string jni2c(const jstring& val, JNIEnv* env)
{
  const char* chars = env->GetStringUTFChars(val, 0);
  std::string ret(chars);
  env->ReleaseStringUTFChars(val, chars);
  return ret;
}

inline int jni2c(const jint val) { return (int)val; }
/* Method to deal with variable passed by reference */
inline void setStringObjValue(const std::string& value,
                              const jobject obj,
                              JNIEnv* env)
{
  jclass objClass = env->GetObjectClass(obj);
  jfieldID objFid = env->GetFieldID(objClass, "value", "Ljava/lang/String;");
  env->SetObjectField(obj, objFid, c2jni(value, env));
}

inline void setIntObjValue(const int value, const jobject obj, JNIEnv* env)
{
  jclass objClass = env->GetObjectClass(obj);
  jfieldID objFid = env->GetFieldID(objClass, "value", "I");
  env->SetIntField(obj, objFid, value);
}

inline void setBoolObjValue(const bool value, const jobject obj, JNIEnv* env)
{
  jclass objClass = env->GetObjectClass(obj);
  jfieldID objFid = env->GetFieldID(objClass, "value", "Z");
  env->SetIntField(obj, objFid, c2jni(value));
}

inline void setPairObjValue(const std::string& filename, const int offset,
                            const jobject obj, JNIEnv* env)
{
  jclass objClass = env->GetObjectClass(obj);
  jfieldID filenameFid = env->GetFieldID(objClass, "filename", "Ljava/lang/String;");
  env->SetObjectField(obj, filenameFid, c2jni(filename, env));
  jfieldID offsetFid = env->GetFieldID(objClass, "offset", "I");
  env->SetIntField(obj, offsetFid, offset);
}

#endif // _ANDROID_JNI_UTILS_H
