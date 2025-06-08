Libkiwix
========

The Libkiwix provides the [Kiwix](https://kiwix.org) software suite
core. It contains the code shared by all Kiwix ports (Windows,
GNU/Linux, macOS, Android, iOS, ...).

[![Release](https://img.shields.io/github/v/tag/kiwix/libkiwix?label=release&sort=semver)](https://download.kiwix.org/release/libkiwix/)
[![Repositories](https://img.shields.io/repology/repositories/libkiwix?label=repositories)](https://github.com/kiwix/libkiwix/wiki/Repology)
[![Build Status](https://github.com/kiwix/libkiwix/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/kiwix/libkiwix/actions?query=branch%3Amain)
[![Doc](https://readthedocs.org/projects/libkiwix/badge/?style=flat)](https://libkiwix.readthedocs.org/en/latest/?badge=latest)
[![CodeFactor](https://www.codefactor.io/repository/github/kiwix/libkiwix/badge)](https://www.codefactor.io/repository/github/kiwix/libkiwix)
[![Codecov](https://codecov.io/gh/kiwix/libkiwix/branch/main/graph/badge.svg)](https://codecov.io/gh/kiwix/libkiwix)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Disclaimer
----------

This document assumes you have a little knowledge about software
compilation. If you experience difficulties with the dependencies or
with the Libkiwix compilation itself, we recommend to have a look to
[kiwix-build](https://github.com/kiwix/kiwix-build).

Preamble
--------

Although the Libkiwix can be (cross-)compiled on/for many systems, the
following documentation explains how to do it on POSIX ones. It is
primarily thought for GNU/Linux systems and has been tested on recent
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
In the worst case, you will have to download and compile bleeding edge
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

The android wrapper uses deprecated methods of libkiwix so it cannot be compiled
with `werror=true` (the default). So you must pass `-Dwerror=false` to meson:

```bash
meson . build -Dwrapper=android -Dwerror=false
ninja -C build
```

Static files compilation
------------------------

Libkiwix has a few static files 'compiled' within the binary
code. This is mostly Javascript/HTML/pictures necessary for the HTTP
daemon.

These static files are available in the `static` directory and are
compiled by custom Python code available in this repository `scripts`
directory. This happens automatically at compilation time without any
additional command to run.

To avoid HTTP caching issues, the URLs (to the static content) are
appended with a `cacheid` parameter (this is called "cache
busting"). This `cacheid` value derived from the
[sha1sum](https://en.wikipedia.org/wiki/Sha1sum) of each targeted
static file. As a consequence, each time you change a static file, the
corresponding `cacheid` value will change.

To properly test this feature, this `cacheid` needs to be added
manually to the automated tests and has to be commited. After
modifying the needed static file, [run the automated
tests](#Testing). They will fail, but the inspection of the testing
log will give you the new `cacheid` value(s). Finally update
`test/server.cpp` with the appropriate `cacheid` value(s) which have
changed.

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
--------------

If you want to uninstall the Kiwix library:
```bash
ninja -C build uninstall
```

Like for the installation, you might need to run the command as `root`
(or using `sudo`).

Custom Index Page
-----------------

to use custom welcome page mention `customIndexPage` argument in `kiwix::internalServer()` or use `kiwix::server->setCustomIndexTemplate()`.
(note - while using custom html file please mention all external links as absolute path.)

to create a HTML template with custom JS you need to have a look at various OPDS based endpoints as mentioned [here](https://wiki.kiwix.org/wiki/OPDS) to load books.

To use JS provided by kiwix-serve you can use the following template to start with ->

```
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width,initial-scale=1" />
    <title><-- Custom Tittle --></title>
    <script src="{{root}}/skin/isotope.pkgd.min.js" defer></script>
    <script src="{{root}}/skin/iso6391To3.js"></script>
    <script type="text/javascript" src="{{root}}/skin/index.js" defer></script>
  </head>
  <body>
  </body>
</html>
```

- To get books listed using `index.js` add - `<div class="book__list"></div>` under body tag.
- To get number of books listed add - `<h3 class="kiwixHomeBody__results"></h3>` under body tag.
- To add language select box add - `<select id="languageFilter"></select>` under body tag.
- To add category select box add - `<select id="categoryFilter"></select>` under body tag.
- To add search box for books use following form -
    ```
        <form id='kiwixSearchForm'>
        <input type="text" name="q" placeholder="Search" id="searchFilter" class='kiwixSearch filter'>
        <input type="submit" class="kiwixButton" value="Search"/>
        </form>
    ```


If you compile manually Libmicrohttpd, you might need to compile it
without GNU TLS, a bug here will impeach further compilation
otherwise.

If the compilation still fails, you might need to get a more recent
version of a dependency than the one packaged by your Linux
distribution. Try then with a source tarball distributed by the
problematic upstream project or even directly from the source code
repository.

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

License
-------

[GPLv3](https://www.gnu.org/licenses/gpl-3.0) or later, see
[COPYING](COPYING) for more details.
