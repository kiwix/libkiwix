Howto build kiwix-lib on Ubuntu 16.04 (LTS)
===========================================

Some dependencies packaged on Ubuntu 16.04 are too old or don't provide
pkg-config files.

The [kiwix-build](https://github.com/kiwix/kiwix-build) script takes care of
downloading, compiling and installing all the dependencies and kiwix-* projects.
We recommend you to use this script.

If you still want to compile yourself kiwix-lib... here we go !

This script describe step by step how to compile kiwix-lib on a fresh
Ubuntu 16.04 system.

# Install needed packages

```
sudo apt install uuid-dev libicu-dev libctpp2-dev automake libtool
```

# Prepare the environment

We will install all our custom dependencies in a separated directory to avoid
polluting the system.

```
KIWIX_DIR=</some/dir>
mkdir $KIWIX_DIR
virtualenv -p python3 $KIWIX_DIR
# Activate the virtualenv
source $KIWIX_DIR/bin/activate
# The virtualenv can be deactivated at anytime with
deactivate
```

## Libzim

```
git clone https://gerrit.wikimedia.org/r/p/openzim.git
cd openzim/zimlib
./autoconf.sh
./configure --prefix=$KIWIX_DIR
make
make install
```

## libpugixml

```
wget http://github.com/zeux/pugixml/releases/download/v1.8/pugixml-1.8.tar.gz && tar xf pugixml-1.8.tar.gz
cd pugixml-1.8
cmake -DCMAKE_INSTALL_PREFIX=$KIWIX_DIR -DBUILD_PKGCONFIG=1 -DBUILD_SHARED_LIBS=1
make
make install
```

## Xapian

The libxapian provided by ubuntu package is too old and do not have the GLASS_BACKEND we use.
So we need to compile our own xapian version:

```
wget http://download.kiwix.org/dev/xapian-core-1.4.0.tar.xz && tar xf xapian-core-1.4.0.tar.xz
cd xapian-core-1.4.0
./configure --enable-shared --enable-static --disable-sse --disable-backend-inmemory --disable-documentation --prefix=$KIWIX_DIR
make
make install
```

## meson

Meson is a python package, just install it with pip.
(Be sure you are using the virtualenv created before)

```
pip install meson
hash -r # let's bash renew its hashes and use the meson we've just installed
```

## ninja

```
git clone git://github.com/ninja-build/ninja.git && cd ninja
git checkout release
./configure.py --bootstrap
cp ninja $KIWIX_DIR/bin
```

The virtualenv automatically add the $KIWIX_DIR/bin in PATH, so there
is nothing more to do.


# Compile kiwix-lib


Then, it's time to compile kiwix-lib :

```
$ cd kiwix-lib
$ mkdir build
$ PKG_CONFIG_PATH=~/KIWIX/lib/pkgconfig meson . build --prefix=$KIWIX_DIR
$ cd build
$ ninja
$ ninja install
```

By default, it will compile dynamic linked libraries.
If you want statically linked libraries, you can add `--default-library=static`
option to the meson command.

Licensed as GPLv3 or later, see COPYING for more details.
