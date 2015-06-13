#!/bin/sh


set -e

VERSION=0.6.2
RELEASE=kunquat-$VERSION


# Make version definitions for library and tracker
echo "#define KQT_VERSION \"$VERSION\"" > src/include/kunquat/version_def.h
echo "KUNQUAT_VERSION = '$VERSION'" > kunquat/tracker/version.py

ver_stash=`git stash create`; git archive -o $RELEASE.tar.gz --prefix=$RELEASE/ ${ver_stash:-HEAD}

exit 0


