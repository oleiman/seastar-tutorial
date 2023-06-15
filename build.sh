#! /bin/bash

rm -rf build
mkdir -p build

pushd build

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

make -j

cp compile_commands.json ..


popd
