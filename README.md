libkiwix
========

libkiwix contains the common code base for all kiwix ports.



Build kiwix-tools
-----------------


Most of the compilation steps (including download and
compilation of dependencies and other tools (kiwix-tools)) are handle
by [kiwix-build](https://github.com/kiwix/kiwix-build) script.
If you don't have any special need, we recommand you to use kiwix-build
instead of doing all the steps yourself.

Dependencies:

You'll need the following dependencies to build libkiwix:

* icu
* libzim
* pugixml
* aria2c
* ctpp2
* xapian (optional)

On debian architecture, you can install the following deb packages:
* libicu-dev
* libxapian-dev
* libctpp2-dev
* aria2c

You will need to install yourself:
* [libzim](http://www.openzim.org/wiki/Zimlib)
* libpugixml - 1.8+, compiled with `-DBUILD_PKGCONFIG=1 -DBUILD_SHARED_LIBS=1`

As we use meson to build kiwix-tools, you will need the common meson tools:
* [meson](http://mesonbuild.com/) >= 0.34
* ninja
* pkg-config

To build:

```
$ cd kiwix-lib
$ meson . build
$ cd build
$ ninja
$ ninja install
```

By default, it will compile dynamic linked libraries.
If you want statically linked libraries, you can add `--default-library=static`
option to the meson command.

Licensed as GPLv3 or later, see COPYING for more details.
