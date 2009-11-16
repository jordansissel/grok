#!/bin/sh
# generate the FILES list we give to rsync for making our release package

svn ls -R | sed -e 's/^/+ /' | sort > FILES
