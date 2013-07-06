#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` >> rc.cpp || exit 11
$XGETTEXT -kI18N_NOOP:1 $(find . -name "*.cpp" -o -name "*.h") -o $podir/kdesvn.pot
rm -f rc.cpp
