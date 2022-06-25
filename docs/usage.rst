Libkiwix programming
====================

Introduction
------------

libkiwix is written in C++. To use the library, you need the include files of libkiwix have
to link against libzim.

Errors are handled with exceptions. When something goes wrong, libkiwix throws an error,
which is always derived from std::exception.

All classes are defined in the namespace kiwix.

libkiwix is a set of tools to manage zim files and provide some common functionnality.
