libkiwix
========

libkiwix contains the common code base for all kiwix ports.

You'll need the following dependencies to build libkiwix:

* libicu-dev
* [libzim](http://www.openzim.org/wiki/Zimlib)
* libpugixml-dev - 1.8+, compiled with `-DBUILD_PKGCONFIG=1 -DBUILD_SHARED_LIBS=1`
* aria2c (usually via `aria2` package)
* [meson](http://mesonbuild.com/)
* pkg-config
* libxapian-dev

To build:

```
$ cd kiwix-lib
$ mkdir -p build
$ cd build
$ meson ..
$ ninja
$ ninja install
```

Licensed as GPLv3 or later, see COPYING for more details.
