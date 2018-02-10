#!/bin/bash

pushd build

uname=`uname`
if [[ "$uname" == "Linux" ]]; then
  cmake ..
else
  cmake -DCMAKE_GENERATOR_PLATFORM=x64 ..
fi

popd
