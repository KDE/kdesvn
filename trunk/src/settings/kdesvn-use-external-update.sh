#!/bin/sh

display=
lastvalue=

while read line ; do
    if [ "${line#\[}" != "$line" ]; then
       continue;
    fi
    KEY="${line%%=*}"
    VALUE="${line#*=}"
    if echo "$KEY" | grep 'use_kompare_for_diff' >/dev/null 2>/dev/null; then
        display=$VALUE
	echo '# DELETE [general_items]use_kompare_for_diff'
    elif echo "$KEY" | grep 'external_diff_display' > /dev/null 2>/dev/null; then
    	exdisplay=$VALUE
    elif [ "x$KEY" != "x" ]; then
	echo "$KEY=$VALUE"
    fi
done

if [ "x$exdisplay" = "x" -o "x$display" = "x1" ]; then
    exdisplay="kompare -on -"
fi
if [ "x$display" = "x2" -o "x$display" = "x1" ]; then
    display=true
else
    display=false
fi
echo "use_external_diff=$display"
echo "external_diff_display=$exdisplay"
