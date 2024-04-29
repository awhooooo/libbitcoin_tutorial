# Installation of libbitcoin packages
Installation of libbitcoin on macOS. Make sure that homebrew is installed and updated.
Make sure you install the below packages in order. Packages mentioned below depends on the ones in above.

```bash
$ sudo apt-get update
$ sudo apt-get install build-essential autoconf automake libtool pkg-config git
$ brew --version
$ brew list
```

## Installing libbitcoin-system
Seems like boost version 1.84 is incompatible with libbitcoin version 3 so --build-boost option was included.
Also, the prefix is set to a certain directory but you can change whatever you want.
```bash
$ sudo apt-get install build-essential autoconf automake libtool pkg-config git
$ cd /Users/legacy/C++_bitcoin
$ wget https://raw.githubusercontent.com/libbitcoin/libbitcoin/version3/install.sh
$ chmod +x install.sh
$ ./install.sh --prefix=/Users/legacy/C++_bitcoin/installation_prefix --build-boost --disable-shared
```

## Installing libbitcoin-protocol
```bash
$ cd /Users/legacy/C++_bitcoin/build-libbitcoin-protocol
$ git clone https://github.com/libbitcoin/libbitcoin-protocol.git
$ cd libbitcoin-protocol
$ git branch --all
$ git checkout remotes/origin/version3
$ git checkout -b version3
$ ./autogen.sh
$ ./configure --with-boost=/Users/legacy/C++_bitcoin/installation_prefix/include --with-boost-libdir=/Users/legacy/C++_bitcoin/installation_prefix/lib LDFLAGS="-L/Users/legacy/C++_bitcoin/installation_prefix/lib" CPPFLAGS="-I/Users/legacy/C++_bitcoin/installation_prefix/include" --prefix=/Users/legacy/C++_bitcoin/installation_prefix
$ make
$ sudo make install
```

## Installing libbitcoin-client
```bash
$ cd /Users/legacy/C++_bitcoin/build-libbitcoin-client
$ git clone https://github.com/libbitcoin/libbitcoin-client.git
$ cd libbitcoin-client
$ git branch --all
$ git checkout remotes/origin/version3
$ git checkout -b version3
$ ./autogen.sh
$ ./configure --with-boost=/Users/legacy/C++_bitcoin/installation_prefix/include --with-boost-libdir=/Users/legacy/C++_bitcoin/installation_prefix/lib LDFLAGS="-L/Users/legacy/C++_bitcoin/installation_prefix/lib" CPPFLAGS="-I/Users/legacy/C++_bitcoin/installation_prefix/include" --prefix=/Users/legacy/C++_bitcoin/installation_prefix
$ make
$ sudo make install
```

## Installing libbitcoin-network
```bash
$ cd /Users/legacy/C++_bitcoin/build-libbitcoin-network
$ git clone https://github.com/libbitcoin/libbitcoin-network.git
$ cd libbitcoin-network
$ git branch --all
$ git checkout remotes/origin/version3
$ git checkout -b version3
$ ./autogen.sh
$ ./configure --with-boost=/Users/legacy/C++_bitcoin/installation_prefix/include --with-boost-libdir=/Users/legacy/C++_bitcoin/installation_prefix/lib LDFLAGS="-L/Users/legacy/C++_bitcoin/installation_prefix/lib" CPPFLAGS="-I/Users/legacy/C++_bitcoin/installation_prefix/include" --prefix=/Users/legacy/C++_bitcoin/installation_prefix
$ make
$ sudo make install
```

## Installing libbitcoin-explorer
```bash
$ cd /Users/legacy/C++_bitcoin/build-libbitcoin-explorer
$ git clone https://github.com/libbitcoin/libbitcoin-explorer.git
$ cd libbitcoin-explorer
$ git branch --all
$ git checkout remotes/origin/version3
$ git checkout -b version3
$ ./autogen.sh
$ ./configure --with-boost=/Users/legacy/C++_bitcoin/installation_prefix/include --with-boost-libdir=/Users/legacy/C++_bitcoin/installation_prefix/lib LDFLAGS="-L/Users/legacy/C++_bitcoin/installation_prefix/lib" CPPFLAGS="-I/Users/legacy/C++_bitcoin/installation_prefix/include" --prefix=/Users/legacy/C++_bitcoin/installation_prefix
$ make
$ sudo make install
```

## After installation
After installation, you might need to add the installation path to your system's PATH environment variable to use the installed binaries.
Make sure that every files are in the pkg-config directory (in this case, /Users/legacy/C++_bitcoin/installation_prefix/lib/pkgconfig).
The directory should contain files like libsecp256k1.pc and libbitcoin-system.pc ...etc.

```bash
$ export PKG_CONFIG_PATH=/Users/legacy/C++_bitcoin/installation_prefix/lib/pkgconfig:$PKG_CONFIG_PATH
$ echo $PKG_CONFIG_PATH
```
