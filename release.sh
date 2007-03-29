#!/bin/sh

usage() {
	echo "$0 [version]"
	exit 1;
}

if [ $# -ne 1 ]; then
  set -- $(date "+%Y%m%d")
  echo "No version specified, using $1" 
fi

TMP="/tmp"
GROK="grok-$1"
DIR="${TMP}/${GROK}"
mkdir "$DIR"
cp -v grok grok.1 grok.conf CHANGELIST "$DIR"
rsync -a --exclude 'CVS' t "$DIR"

tar -C /tmp -vcf grok-$1.tar.gz "$GROK"
rm -r $DIR
