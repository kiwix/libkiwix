
#ifndef _KIWIX_COMMON_H_
#define _KIWIX_COMMON_H_

#include <zim/zim.h>

#ifdef __GNUC__
#define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#praga message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED
#endif


namespace kiwix {

typedef zim::size_type size_type;
typedef zim::offset_type offset_type;

}

#endif //_KIWIX_COMMON_H_
