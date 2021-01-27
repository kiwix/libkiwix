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

#include <mutex>
#include <string>
#include <vector>
#include <iostream>

#if __ANDROID__
 #include <android/log.h>
 #define LOG(...) __android_log_print(ANDROID_LOG_ERROR, "kiwix", __VA_ARGS__)
#else
 #define LOG(...)
#endif

extern std::mutex globalLock;

template<typename T>
void setPtr(JNIEnv* env, jobject thisObj, T* ptr)
{
  jclass thisClass = env->GetObjectClass(thisObj);
  jfieldID fieldId = env->GetFieldID(thisClass, "nativeHandle", "J");
  env->SetLongField(thisObj, fieldId, reinterpret_cast<jlong>(ptr));
}

template<typename T, typename ...Args>
void allocate(JNIEnv* env, jobject thisObj, Args && ...args)
{
  T* ptr = new T(std::forward<Args>(args)...);
  setPtr(env, thisObj, ptr);
}

template<typename T>
T* getPtr(JNIEnv* env, jobject thisObj)
{
  jclass thisClass = env->GetObjectClass(thisObj);
  jfieldID fidNumber = env->GetFieldID(thisClass, "nativeHandle", "J");
  return reinterpret_cast<T*>(env->GetLongField(thisObj, fidNumber));
}

template<typename T>
void dispose(JNIEnv* env, jobject thisObj)
{
  delete getPtr<T>(env, thisObj);
}

#define METHOD0(retType, class, name) \
JNIEXPORT retType JNICALL Java_org_kiwix_kiwixlib_##class##_##name( \
 JNIEnv* env, jobject thisObj)

#define METHOD(retType, class, name, ...) \
JNIEXPORT retType JNICALL Java_org_kiwix_kiwixlib_##class##_##name( \
  JNIEnv* env, jobject thisObj, __VA_ARGS__)

inline jfieldID getHandleField(JNIEnv* env, jobject obj)
{
  jclass c = env->GetObjectClass(obj);
  // J is the type signature for long:
  return env->GetFieldID(c, "nativeHandle", "J");
}

inline jobjectArray createArray(JNIEnv* env, size_t length, const std::string& type_sig)
{
  jclass c = env->FindClass(type_sig.c_str());
  return env->NewObjectArray(length, c, NULL);
}

class Lock : public std::unique_lock<std::mutex>
{
 public:
  Lock() : std::unique_lock<std::mutex>(globalLock) { }
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
  operator T*() const { return h->h; }
};

template<typename T>
struct JType { };

template<> struct JType<bool>{ typedef jboolean type_t; };
template<> struct JType<int>{ typedef jint type_t; };
template<> struct JType<long>{ typedef jlong type_t; };
template<> struct JType<uint64_t> { typedef jlong type_t; };
template<> struct JType<uint32_t> { typedef jlong type_t; };
template<> struct JType<std::string>{ typedef jstring type_t; };
template<> struct JType<std::vector<std::string>>{ typedef jobjectArray type_t; };

template<typename T>
inline typename JType<T>::type_t c2jni(const T& val, JNIEnv* env) {
  return static_cast<typename JType<T>::type_t>(val);
}

template<>
inline jboolean c2jni(const bool& val, JNIEnv* env) { return val ? JNI_TRUE : JNI_FALSE; }

template<>
inline jstring c2jni(const std::string& val, JNIEnv* env)
{
  return env->NewStringUTF(val.c_str());
}

template<>
inline jobjectArray c2jni(const std::vector<std::string>& val, JNIEnv* env)
{
  auto array = createArray(env, val.size(), "java/lang/String");
  size_t index = 0;
  for (auto& elem: val) {
    auto jElem = c2jni(elem, env);
    env->SetObjectArrayElement(array, index++, jElem);
  }
  return array;
}

template<typename T, typename U=T>
struct CType { };

template<> struct CType<jboolean>{ typedef bool type_t; };
template<> struct CType<jint>{ typedef int type_t; };
template<> struct CType<jlong>{ typedef long type_t; };
template<> struct CType<jstring>{ typedef std::string type_t; };

template<typename U>
struct CType<jobjectArray, U>{ typedef std::vector<typename CType<U>::type_t> type_t; };

/* jni2c type conversion functions */
template<typename T, typename U=T>
inline typename CType<T>::type_t jni2c(const T& val, JNIEnv* env) {
  return static_cast<typename CType<T>::type_t>(val);
}

template<>
inline bool jni2c(const jboolean& val, JNIEnv* env) { return val == JNI_TRUE; }

template<>
inline std::string jni2c(const jstring& val, JNIEnv* env)
{
  const char* chars = env->GetStringUTFChars(val, 0);
  std::string ret(chars);
  env->ReleaseStringUTFChars(val, chars);
  return ret;
}

template<typename U>
inline typename CType<jobjectArray, U>::type_t jni2c(const jobjectArray& val, JNIEnv* env)
{
  jsize length = env->GetArrayLength(val);
  typename CType<jobjectArray, U>::type_t v(length);

  int i;
  for (i = 0; i < length; i++) {
    U obj = (U) env->GetObjectArrayElement(val, i);
    auto cobj = jni2c<U>(obj, env);
    v.push_back(cobj);
  }
  return v;
}

/* Method to deal with variable passed by reference */
inline std::string getStringObjValue(const jobject obj, JNIEnv* env)
{
  jclass objClass = env->GetObjectClass(obj);
  jfieldID objFid = env->GetFieldID(objClass, "value", "Ljava/lang/String;");
  jstring jstr = (jstring)env->GetObjectField(obj, objFid);
  return jni2c(jstr, env);
}
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
  env->SetIntField(obj, objFid, c2jni(value, env));
}

inline void setPairObjValue(const std::string& filename, const long offset,
                            const jobject obj, JNIEnv* env)
{
  jclass objClass = env->GetObjectClass(obj);
  jfieldID filenameFid = env->GetFieldID(objClass, "filename", "Ljava/lang/String;");
  env->SetObjectField(obj, filenameFid, c2jni(filename, env));
  jfieldID offsetFid = env->GetFieldID(objClass, "offset", "J");
  env->SetLongField(obj, offsetFid, offset);
}

#endif // _ANDROID_JNI_UTILS_H
