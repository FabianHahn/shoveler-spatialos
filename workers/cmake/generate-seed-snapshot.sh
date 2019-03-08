#!/bin/bash

uname=`uname`
if [[ "$uname" == "Linux" ]]; then
  pushd build/seeder
  ./seeder_lights ../../../../snapshots/default.snapshot
  ./seeder_tiles \
    assets/tilesets/spring.png 7 2 \
    assets/characters/player1.png \
    assets/characters/player2.png \
    assets/characters/player3.png \
    assets/characters/player4.png \
    10 \
    ../../../../snapshots/tiles.snapshot
  popd
else
  pushd build/seeder/Release
  ./seeder_lights ../../../../../snapshots/default.snapshot
  ./seeder_tiles \
      assets/tilesets/spring.png 7 2 \
      assets/characters/player1.png \
      assets/characters/player2.png \
      assets/characters/player3.png \
      assets/characters/player4.png \
      10 \
      ../../../../../snapshots/tiles.snapshot
  popd
fi
