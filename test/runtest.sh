TEST=$1

if [ -z "$TEST" ] ; then
  echo "Usage: $0 <test>"
  exit 1
fi

echo "=> Building $TEST"
MAKE=make
which gmake > /dev/null 2>&1 && MAKE=gmake
build="$MAKE EXTRA_CFLAGS="$EXTRA_CFLAGS" EXTRA_LDFLAGS="$EXTRA_LDFLAGS" clean $TEST"
cout=`$build 2>&1`
if [ $? -ne 0 ]; then
  echo "$cout"
  echo "Compile for $TEST failed."
else
  ./$TEST | egrep '^  (Test:|  [0-9])'
fi


