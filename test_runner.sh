#!/bin/bash

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then  xvfb-run ./make.py --enable-long-tests --disable-libkunquatfile --disable-libkunquatfile-dev; fi
if [[ "$TRAVIS_OS_NAME" == "osx"   ]]; then  ./make.py --disable-threads --disable-tests --disable-libkunquatfile --disable-libkunquatfile-dev; fi
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then  xvfb-run ./make.py --enable-long-tests --cc=clang --disable-libkunquatfile --disable-libkunquatfile-dev; fi
if [[ "$TRAVIS_OS_NAME" == "osx"   ]]; then  ./make.py --disable-threads --disable-tests --cc=clang --disable-libkunquatfile --disable-libkunquatfile-dev; fi
