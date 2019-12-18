#!/bin/bash
for file in $(find /sys/module -name .text) ; do
    addr=`cat ${file}`
    echo $addr " : " ${file}
done
