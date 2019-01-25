#!/bin/bash

uname=`uname`
if [[ "$uname" == "Linux" ]]; then
  pushd build/$1
  cmake -E tar "cfv" "../../../../build/assembly/worker/$2@Linux.zip" --format=zip $2
  popd
else
  pushd build/$1/Release
  cmake -E tar "cfv" "../../../../../build/assembly/worker/$2@Windows.zip" --format=zip $2.exe
  popd
fi

