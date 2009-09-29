#!/bin/sh

if [ $# -ne 1 ] ; then 
  set -- $(date "+%Y%m%d")
  echo "No version specified, using $1"
fi

if [ -z "$PACKAGE" ] ; then
  echo "Must specify package name in environment."
  echo "PACKAGE= ???"
  exit 1
fi

if [ ! -f "FILES" ] ; then
  echo "Missing FILES file which should contain the list of things to put in this package"
  exit 1
fi

TMP="/tmp"
PACKAGE="$PACKAGE-$1"
DIR="${TMP}/${PACKAGE}"
rm -rf "$DIR"
mkdir "$DIR"
rsync -a --filter '. FILES' . "$DIR"

# gnu tar sucks, tar -C /tmp doesn't actually change directories for tar
# creation?
find $DIR -name '*.o' -delete
tar -C /tmp/ -zcf "${PACKAGE}.tar.gz" "$PACKAGE"
rm -rf "$DIR"

