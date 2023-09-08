#!/bin/bash
cd ./kernel/src/external_libs
rm -rf flanterm
mkdir -p flanterm
echo Downloading flanterm...
git clone https://github.com/mintsuki/flanterm
cd ../../../ # root
echo OK
