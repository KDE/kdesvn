#/bin/sh

j=`kde-config --version|grep KDE|awk '{print $2}'|sed s/\\\\./\\ /g`
ma=`echo $j | awk '{print $1}'`
mi=`echo $j | awk '{print $2}'`

echo "KDE Minor = ${mi} und KDE Major = ${ma}"


if [ "x$1" = "x" ]; then
   echo "No input"
   exit 0
fi
echo "Edit ${1}"

if [ "x$2" = "x" ]; then
 n=`echo ${1}|sed s/\\\\.template//g`
else
 n="${2}"
fi

echo "Moving to ${n}"

if [ "${mi}" = "5" ]; then
  cat ${1} | sed s/^Actions/\#Actions/g > $n
else
  cp ${1} ${n}
fi

