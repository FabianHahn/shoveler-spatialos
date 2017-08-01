#!/bin/bash

pushd build

uname=`uname`
if [[ "$uname" == "Linux" ]]; then
  cmake --build . --target $@
else
  cmake --build . --config release --target $@
fi

popd
