#!/bin/bash

pushd () {
    command pushd "$@" > /dev/null
}

popd () {
    command popd "$@" > /dev/null
}

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
SRCPATH=$SCRIPTPATH/..

make clean
phpize clean

pushd $SRCPATH

echo "[+] Building Extension....\n"




phpize
./configure --enable-piof

make
make install

popd