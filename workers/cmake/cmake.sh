#!/bin/bash

pushd build

uname=`uname`
if [[ "$uname" == "Linux" ]]; then
  cmake -DCMAKE_EXE_LINKER_FLAGS="-no-pie" ..
else
  cmake -DCMAKE_GENERATOR_PLATFORM=x64 ..
fi

popd
