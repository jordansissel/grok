#!/bin/sh

. ./test.subr

rungrok() {
  perl ../grok -m "%$1%" -r "%$1%" < input/${1}.${2} 
}

if [ $# -gt 0 ]; then
  patterns=$1
  types=$2
fi

[ -z "$patterns" ] \
  && patterns=`cat ../grok | sed -ne '/^my %MATCH/,/^);$/p;' | sort | awk '$1 ~ /^[A-Z]+$/ { print $1 }')`

[ -z "$types" ]  \
  && types="match fail"

for pattern in $patterns; do
  for file in $types; do
    if [ -f input/${pattern}.${file} ] ; then
      #rungrok $pattern $file > output/${pattern}.${file}.new
      try $pattern $file
    else
      #echo "Skipping $pattern/$file, no input to test"
    fi
  done
done
