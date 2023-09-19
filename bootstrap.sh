#!/bin/bash
cd ./kernel/src/external_libs
rm -rf flanterm
mkdir -p flanterm
echo + downloading flanterm...
git clone https://github.com/mintsuki/flanterm
cd ../../../ # root
echo + downloading limine
make limine
echo OK
