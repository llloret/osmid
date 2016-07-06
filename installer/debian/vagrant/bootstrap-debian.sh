#!/usr/bin/env bash
apt-get update
apt-get install --yes libboost-system-dev libboost-program-options-dev
apt-get install --yes libasound2-dev
apt-get install --yes build-essential pkg-config
apt-get install --yes git

# fetch and install cmake
[ -e cmake ] || mkdir cmake 
wget --quiet --no-check-certificate -O - https://cmake.org/files/v3.5/cmake-3.5.2-Linux-x86_64.tar.gz | tar --strip-components=1 -xz -C cmake
export PATH=$PWD/cmake/bin:${PATH}

# And now fetch osmid
rm -rf osmid
git clone https://github.com/llloret/osmid.git
cd osmid
[ -e build ] || mkdir build
cd build
cmake ..
make

