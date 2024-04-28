Installation of libbitcoin packages

# installing libbitcoin-system
>>> sudo apt-get install build-essential autoconf automake libtool pkg-config git
>>> cd /Users/legacy/C++_bitcoin
>>> wget https://raw.githubusercontent.com/libbitcoin/libbitcoin/version3/install.sh
>>> chmod +x install.sh
>>> ./install.sh --prefix=/Users/legacy/C++_bitcoin/installation_prefix --build-boost --disable-shared
# seems like boost version 1.84 is incompatible with libbitcoin version 3 so --build-boost option was included

# installing libbitcoin-protocol
>>> cd /Users/legacy/C++_bitcoin/build-libbitcoin-protocol
>>> git clone https://github.com/libbitcoin/libbitcoin-protocol.git
>>> cd libbitcoin-protocol
>>> git branch --all
>>> git checkout remotes/origin/version3
>>> git checkout -b version3
>>> ./autogen.sh
>>> ./configure --with-boost=/Users/legacy/C++_bitcoin/installation_prefix/include --with-boost-libdir=/Users/legacy/C++_bitcoin/installation_prefix/lib LDFLAGS="-L/Users/legacy/C++_bitcoin/installation_prefix/lib" CPPFLAGS="-I/Users/legacy/C++_bitcoin/installation_prefix/include" --prefix=/Users/legacy/C++_bitcoin/installation_prefix
>>> make
>>> sudo make install

# installing libbitcoin-client
>>> cd /Users/legacy/C++_bitcoin/build-libbitcoin-client
>>> git clone https://github.com/libbitcoin/libbitcoin-client.git
>>> cd libbitcoin-client
>>> git branch --all
>>> git checkout remotes/origin/version3
>>> git checkout -b version3
>>> ./autogen.sh
>>> ./configure --with-boost=/Users/legacy/C++_bitcoin/installation_prefix/include --with-boost-libdir=/Users/legacy/C++_bitcoin/installation_prefix/lib LDFLAGS="-L/Users/legacy/C++_bitcoin/installation_prefix/lib" CPPFLAGS="-I/Users/legacy/C++_bitcoin/installation_prefix/include" --prefix=/Users/legacy/C++_bitcoin/installation_prefix
>>> make
>>> sudo make install
