#!/bin/sh


VERSION=0.9.4
RELEASE=kunquat-$VERSION


set -e


# Make sure there are no local changes
# (required for the version substitution below to work correctly)
if [ -n "$(git diff)" ] || [ -n "$(git diff --cached)" ]
then
    echo ""
    echo "Cannot make a release package because of local changes."
    echo ""
    exit 1
fi


# Make version definitions for library and tracker
version_lib_path="src/include/kunquat/version_def.h"
version_tracker_path="kunquat/tracker/version.py"
echo "#define KQT_VERSION \"$VERSION\"" > $version_lib_path
echo "KUNQUAT_VERSION = '$VERSION'" > $version_tracker_path

# Create release package
ver_stash=`git stash create`; git archive -o $RELEASE.tar.gz --prefix=$RELEASE/ ${ver_stash:-HEAD}

# Restore original version files from the repository
git checkout $version_lib_path
git checkout $version_tracker_path

# Update versions and shasums in packaging scripts
for pkg in kunquat kunquat-git; do 
  sed --in-place "s/^pkgver=.*/pkgver=$VERSION/" packaging/arch/$pkg/PKGBUILD
done
shasum=$(sha256sum -b $RELEASE.tar.gz|grep -o '^[a-f0-9]*')
sed --in-place "s/'[^']*' # Release package hash/'$shasum' # Release package hash/" packaging/arch/kunquat/PKGBUILD

exit 0


