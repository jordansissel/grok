#!/bin/sh

LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:$PWD/../../" RUBYLIB="$PWD/../ext:$PWD/../lib" ruby "$@"
