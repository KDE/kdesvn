#!/bin/sh

if [ "x$1" = "x" ]; then
   echo "No source"
   exit 0
fi
if [ "x$2" = "x" ]; then
   echo "No target"
   exit 0
fi

if [ ! -h $1 ]; then
 echo "linking $2 -> $1"
 ln -fs "$2" "$1"
fi

