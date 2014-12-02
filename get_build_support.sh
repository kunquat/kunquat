#!/bin/sh


set -e

verified_path="support/fabricate.py"
unverified_path="support/fabricate_unverified.py"
known_sha1sum="7510ec65a6135be611f1b311eb2851e74141859f"


# Check for existing verified version

if [ -f $verified_path ]
then
    existing_sha1sum=$(sha1sum $verified_path | cut -d" " -f1)
    if [ "$existing_sha1sum" = "$known_sha1sum" ]
    then
        echo ""
        echo "Fabricate is already downloaded."
        echo "You can build Kunquat by running ./make.py"
        echo ""
        exit 0
    fi
fi


# Fabricate not found yet, so download it

mkdir -p support
touch support/__init__.py
wget -O $unverified_path https://fabricate.googlecode.com/git/fabricate.py

retrieved_sha1sum=$(sha1sum $unverified_path | cut -d" " -f1)

if [ "$retrieved_sha1sum" = "$known_sha1sum" ]
then
    echo "SHA1 checksum verified successfully."
    mv -v $unverified_path $verified_path
    echo "You can now build Kunquat by running ./make.py"
    echo ""
else
    echo "WARNING: Retrieved build support has unexpected contents."
    echo "It is possible that Fabricate has been updated to a new version;"
    echo "however, the downloaded file may also have been altered maliciously."
    echo ""
    echo "The downloaded file is located at $unverified_path"
    echo "If you choose to trust this file, you can build Kunquat by running ./make.py --unsafe"
    echo "Alternatively, you can download fabricate.py manually from"
    echo "http://fabricate.googlecode.com/ and store it in $verified_path"
    echo ""
    exit 1
fi


