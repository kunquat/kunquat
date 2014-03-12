#!/bin/sh


VERSION=0.6.1
RELEASE=kunquat-$VERSION


git archive -o $RELEASE.tar.gz --prefix=$RELEASE/ HEAD

exit 0


