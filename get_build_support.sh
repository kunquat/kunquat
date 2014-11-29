#!/bin/sh


set -e

verified_path="support/fabricate.py"
unverified_path="support/fabricate_unverified.py"
known_sha1sum_output="7510ec65a6135be611f1b311eb2851e74141859f  $unverified_path"

mkdir -p support
touch support/__init__.py
wget -O $unverified_path https://fabricate.googlecode.com/git/fabricate.py

retrieved_sha1sum_output=$(sha1sum $unverified_path)

if [ "$retrieved_sha1sum_output" = "$known_sha1sum_output" ]
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


