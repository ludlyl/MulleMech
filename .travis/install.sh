#!/bin/bash
#
# The MIT License (MIT)
#
# Copyright (c) 2017-2018 Alexander Kurbatov

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    sudo apt-get install -y g++-5
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 90
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 90
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    # NOTE (alkurbatov): Apply compilation fixes for OS X.
    git apply hacks/civetweb_compilation_fix.patch
fi

curl -o cpplint.py https://raw.githubusercontent.com/google/styleguide/gh-pages/cpplint/cpplint.py
