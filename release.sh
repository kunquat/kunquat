#!/bin/sh


VERSION=0.6.2
RELEASE=kunquat-$VERSION


echo "#define KQT_VERSION \"$VERSION\"" > src/include/kunquat/version_def.h
echo "KUNQUAT_VERSION = '$VERSION'" > kunquat/tracker/version.py

git archive -o $RELEASE.tar.gz --prefix=$RELEASE/ HEAD

exit 0


