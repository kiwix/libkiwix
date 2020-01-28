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
#include "org_kiwix_kiwixlib_Filter.h"

#include "library.h"
#include "utils.h"

/* Kiwix Reader JNI functions */
METHOD0(void, Filter, allocate) {
  allocate<kiwix::Filter>(env, thisObj);
}

METHOD0(void, Filter, dispose) {
  dispose<kiwix::Library>(env, thisObj);
}

#define FILTER (getPtr<kiwix::Filter>(env, thisObj))

#define FORWARD(name, args_type) \
METHOD(jobject, Filter, name, args_type value) { \
  FILTER->name(jni2c(value, env)); \
  return thisObj; \
}

#define FORWARDA(name, args_type) \
METHOD(jobject, Filter, name, jobjectArray value) { \
  FILTER->name(jni2c<args_type>(value, env)); \
  return thisObj; \
}



FORWARD(local, jboolean)
FORWARD(remote, jboolean)
FORWARD(valid, jboolean)
FORWARDA(acceptTags, jstring)
FORWARDA(rejectTags, jstring)
FORWARD(lang, jstring)
FORWARD(publisher, jstring)
FORWARD(creator, jstring)
FORWARD(maxSize, jlong)
FORWARD(query, jstring)


