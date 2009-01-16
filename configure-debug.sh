#!/bin/sh

if [ ! -d Build/Debug ]; then
 mkdir -p Build/Debug
fi

if [ ! -d Build/Debug ]; then
  echo "Could not change to build folder"
  exit -1
fi

cd Build/Debug||exit

if [ "x${HOSTTYPE}" = "xx86_64" ]; then
  LIBSUFFIX=64
fi

cmake ../.. -DCMAKE_BUILD_TYPE=Debug -DLIB_SUFFIX=${LIBSUFFIX} -DTESTINGBUILD=On -DBUILD_TESTS=On
cd -

