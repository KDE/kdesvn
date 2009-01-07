#!/bin/sh

if [ ! -d Build/Release ]; then
 mkdir -p Build/Release
fi

if [ ! -d Build/Release ]; then
  echo "Could not change to build folder"
  exit -1
fi

cd Build/Release||exit

if [ "x${HOSTTYPE}" = "xx86_64" ]; then
  LIBSUFFIX=64
fi

cmake ../.. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLIB_SUFFIX=${LIBSUFFIX}
cd -

