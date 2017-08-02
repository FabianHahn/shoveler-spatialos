#!/bin/bash

uname=`uname`
if [[ "$uname" == "Linux" ]]; then
  pushd build/seeder
  ./seeder ../../../../snapshots/default.snapshot
  popd
else
  pushd build/seeder/Release
  ./seeder ../../../../../snapshots/default.snapshot
  popd
fi
