Libkiwix
========

The Libkiwix provides the [Kiwix](https://kiwix.org) software suite
core. It contains the code shared by all Kiwix ports (Windows,
GNU/Linux, macOS, Android, iOS, ...).

[![Download](https://api.bintray.com/packages/kiwix/kiwix/kiwixlib/images/download.svg)](https://bintray.com/kiwix/kiwix/kiwixlib/_latestVersion)
[![Repositories](https://img.shields.io/repology/repositories/libkiwix?label=repositories)](https://github.com/kiwix/libkiwix/wiki/Repology)
[![Build Status](https://github.com/kiwix/libkiwix/workflows/CI/badge.svg?query=branch%3Amaster)](https://github.com/kiwix/libkiwix/actions?query=branch%3Amaster)
[![CodeFactor](https://www.codefactor.io/repository/github/kiwix/libkiwix/badge)](https://www.codefactor.io/repository/github/kiwix/libkiwix)
[![Codecov](https://codecov.io/gh/kiwix/libkiwix/branch/master/graph/badge.svg)](https://codecov.io/gh/kiwix/libkiwix)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Disclaimer
----------

This document assumes you have a little knowledge about software
compilation. If you experience difficulties with the dependencies or
with the Libkiwix compilation itself, we recommend to have a look to
[kiwix-build](https://github.com/kiwix/kiwix-build).

Preamble
--------

Although the Libkiwix can be (cross-)compiled on/for many sytems, the
following documentation explains how to do it on POSIX ones. It is
primarly thought for GNU/Linux systems and has been tested on recent
releases of Ubuntu and Fedora.

Dependencies
------------

The Libkiwix relies on many third party software libraries. They are
prerequisites to the Libkiwix compilation. Following libraries need to
be available:
* [ICU](https://site.icu-project.org/) (package `libicu-dev` on Ubuntu)
* [ZIM](https://openzim.org/) (package `libzim-dev` on Ubuntu)
* [Pugixml](https://pugixml.org/) (package `libpugixml-dev` on Ubuntu)
* [Mustache](https://github.com/kainjow/Mustache) (Just copy the
header `mustache.hpp` somewhere it can be found by the compiler and/or
set CPPFLAGS with correct `-I` option). Use Mustache version 4.1 or above.
* [Libcurl](https://curl.se/libcurl) (`libcurl4-gnutls-dev`, `libcurl4-nss-dev` or `libcurl4-openssl-dev` on Ubuntu)
* [Microhttpd](https://www.gnu.org/software/libmicrohttpd) (package `libmicrohttpd-dev` on Ubuntu)
* [Zlib](https://zlib.net/) (package `zlib1g-dev` on Ubuntu)

To test the code:
* [Google Test](https://github.com/google/googletest) (package `googletest` on Ubuntu)

The following dependency needs to be available at runtime:
* [Aria2](https://aria2.github.io/) (package `aria2` on Ubuntu)

These dependencies may or may not be packaged by your operating
system. They may also be packaged but only in an older version. The
compilation script will tell you if one of them is missing or too old.
In the worse case, you will have to download and compile bleeding edge
version by hand.

If you want to install these dependencies locally, then use the
`libkiwix` directory as install prefix.

Environment
-------------

The Libkiwix builds using [Meson](https://mesonbuild.com/) version
0.45 or higher. Meson relies itself on Ninja, pkg-config and few other
compilation tools.

Install first the few common compilation tools:
* [Meson](https://mesonbuild.com/)
* [Ninja](https://ninja-build.org/)
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)

These tools should be packaged if you use a cutting edge operating
system. If not, have a look to the [Troubleshooting](#Troubleshooting)
section.

Compilation
-----------

Once all dependencies are installed, you can compile the Libkiwix
with:
```bash
meson . build
ninja -C build
```

By default, it will compile dynamic linked libraries. All binary files
will be created in the `build` directory created automatically by
Meson. If you want statically linked libraries, you can add
`--default-library=static` option to the Meson command.

Depending of you system, `ninja` may be called `ninja-build`.

Testing
-------

To run the automated tests:
```bash
cd build
meson test
```

Installation
------------

If you want to install the Libkiwix and the headers you just have
compiled on your system, here we go:
```bash
ninja -C build install
```

You might need to run the command as root (or using `sudo`), depending
where you want to install the libraries. After the installation
succeeded, you may need to run `ldconfig` (as `root`).

Uninstallation
------------

If you want to uninstall the Kiwix library:
```bash
ninja -C build uninstall
```

Like for the installation, you might need to run the command as `root`
(or using `sudo`).

Troubleshooting
---------------

If you need to install Meson "manually":
```bash
virtualenv -p python3 ./ # Create virtualenv
source bin/activate      # Activate the virtualenv
pip3 install meson       # Install Meson
hash -r                  # Refresh bash paths
```

If you need to install Ninja "manually":
```bash
git clone git://github.com/ninja-build/ninja.git
cd ninja
git checkout release
./configure.py --bootstrap
mkdir ../bin
cp ninja ../bin
cd ..
```

If you compile manually Libmicrohttpd, you might need to compile it
without GNU TLS, a bug here will empeach further compilation
otherwise.

If the compilation still fails, you might need to get a more recent
version of a dependency than the one packaged by your Linux
distribution. Try then with a source tarball distributed by the
problematic upstream project or even directly from the source code
repository.

License
-------

[GPLv3](https://www.gnu.org/licenses/gpl-3.0) or later, see
[COPYING](COPYING) for more details.
