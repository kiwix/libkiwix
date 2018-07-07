Kiwix library
=============

The Kiwix library provides the Kiwix software core. It contains the
code shared by all Kiwix ports (Windows, Linux, OSX, Android, ...).

Disclaimer
----------

This document assumes you have a little knowledge about software
compilation. If you experience difficulties with the dependencies or
with the Kiwix libary compilation itself, we recommend to have a look
to [kiwix-build](https://github.com/kiwix/kiwix-build).

Preamble
--------

Although the Kiwix library can be (cross-)compiled on/for many
sytems, the following documentation explains how to do it on POSIX
ones. It is primarly thought for GNU/Linux systems and has been tested
on recent releases of Ubuntu and Fedora.

Dependencies
------------

The Kiwix library relies on many third parts software libraries. They
are prerequisites to the Kiwix library compilation. Following
libraries need to be available:

* ICU ................................... http://site.icu-project.org/
(package libicu-dev on Ubuntu)
* ZIM ........................................ http://www.openzim.org/
(package libzim-dev on Ubuntu)
* Pugixml ........................................ http://pugixml.org/
(package libpugixml-dev on Ubuntu)
* ctpp2 ........................................ http://ctpp.havoc.ru/
(package libctpp2-dev on Ubuntu)
* Xapian ......................................... https://xapian.org/
(package libxapian-dev on Ubuntu)
* libaria2 .................................. https://aria2.github.io/
(no package on Ubuntu)

These dependencies may or may not be packaged by your operating
system. They may also be packaged but only in an older version. The
compilation script will tell you if one of them is missing or too old.
In the worse case, you will have to download and compile bleeding edge
version by hand.

If you want to install these dependencies locally, then use the
kiwix-lib directory as install prefix.

If you compile ctpp2 from source and want to compile the Kiwix library
statically then you will probably need to rename ctpp2 static library
from ctpp2-st.a to ctpp2.a.

Environment
-------------

The Kiwix library builds using [Meson](http://mesonbuild.com/) version
0.39 or higher. Meson relies itself on Ninja, pkg-config and few other
compilation tools.

Install first the few common compilation tools:
* Meson
* Ninja
* Pkg-config

These tools should be packaged if you use a cutting edge operating
system. If not, have a look to the "Troubleshooting" section.

Compilation
-----------

Once all dependencies are installed, you can compile the Kiwix library
with:
```
meson . build
ninja -C build
```

By default, it will compile dynamic linked libraries. All binary files
will be created in the "build" directory created automatically by
Meson. If you want statically linked libraries, you can add
`--default-library=static` option to the Meson command.

Depending of you system, `ninja` may be called `ninja-build`.

Installation
------------

If you want to install the Kiwix library and the headers you just have
compiled on your system, here we go:

```
ninja -C build install
```

You might need to run the command as root (or using 'sudo'), depending
where you want to install the libraries. After the installation
succeeded, you may need to run ldconfig (as root).

Uninstallation
------------

If you want to uninstall the Kiwix library:

```
ninja -C build uninstall
```

Like for the installation, you might need to run the command as root
(or using 'sudo').

Troubleshooting
---------------

If you need to install Meson "manually":
```
virtualenv -p python3 ./ # Create virtualenv
source bin/activate      # Activate the virtualenv
pip3 install meson       # Install Meson
hash -r                  # Refresh bash paths
```

If you need to install Ninja "manually":
```
git clone git://github.com/ninja-build/ninja.git
cd ninja
git checkout release
./configure.py --bootstrap
mkdir ../bin
cp ninja ../bin
cd ..
```

If the compilation still fails, you might need to get a more recent
version of a dependency than the one packaged by your Linux
distribution. Try then with a source tarball distributed by the
problematic upstream project or even directly from the source code
repository.

License
-------

GPLv3 or later, see COPYING for more details.
