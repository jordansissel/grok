#!/bin/sh

. ./test.subr

rungrok() {
  perl ../grok -m "%GREEDYDATA%" -r "%GREEDYDATA|$1%" < input/filter.$1
}

if [ $# -gt 0 ]; then
  filters=$1
fi

[ -z "$filters" ] \
  && filters=`ls input/filter.* | cut -d. -f2`

for filter in $filters; do
  if [ -f input/filter.${filter} ] ; then
    rungrok $filter
    #try $filter
  else
    #echo "Skipping $filters/$file, no input to test"
  fi
done
