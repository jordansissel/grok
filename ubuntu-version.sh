#!/bin/sh

if grep Ubuntu /etc/issue > /dev/null
then
  VERSION=`head -n 1 /etc/issue | cut -d' ' -f2-2`
fi

echo $VERSION
