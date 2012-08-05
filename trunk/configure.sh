#!/bin/sh

if [ ! -d Build/Release ]; then
 mkdir -p Build/Release
fi

if [ ! -d Build/Release ]; then
  echo "Could not change to build folder"
  exit -1
fi

cd Build/Release||exit

cmake ../.. -DCMAKE_BUILD_TYPE=RelWithDebInfo 
cd -

