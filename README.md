libkiwix
========

libkiwix contains the common code base for all kiwix ports.

Build kiwix-lib
---------------


Most of the compilation steps (including download and
compilation of dependencies and other tools (kiwix-tools)) are handle
by [kiwix-build](https://github.com/kiwix/kiwix-build) script.
If you don't have any special need, we recommend you to use kiwix-build
instead of doing all the steps yourself.

Dependencies:

You'll need the following dependencies to build libkiwix:

* icu
* libzim
* pugixml
* ctpp2
* xapian (optional) (>=1.4)
* meson build system (>=0.34)(and so, ninja, pkg-config, ...)

Once all dependencies are installed, you can compile kiwix-lib with:

```
$ cd kiwix-lib
$ mkdir build
$ meson . build
$ cd build
$ ninja
$ ninja install
```

By default, it will compile dynamic linked libraries.
If you want statically linked libraries, you can add `--default-library=static`
option to the meson command.

(You may need to set PKG_CONFIG_PATH before running meson depending of where
and how you've install dependencies)
(Depending of you system, `ninja` may be called `ninja-build`)


Howto build kiwix-lib on Ubuntu 16.04 (LTS)
-------------------------------------------

If you want to compile yourself kiwix-lib see the specific readme to
[compile kiwix-lib on Ubuntu 16.04](COMPILE_ubuntu-16.04.md).

Licensed as GPLv3 or later, see COPYING for more details.
