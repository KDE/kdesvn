#!/bin/sh

if [ ! -d Build/Debug ]; then
 mkdir -p Build/Debug
fi

if [ ! -d Build/Debug ]; then
  echo "Could not change to build folder"
  exit -1
fi

cd Build/Debug||exit

cmake ../.. -DCMAKE_BUILD_TYPE=Debug -DTESTINGBUILD=On -DBUILD_TESTS=On -DCMAKE_INSTALL_PREFIX=/usr
cd -

