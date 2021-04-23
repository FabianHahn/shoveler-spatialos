#!/bin/bash

if [ $# -lt 1 ]; then 
	echo "Usage: $0 VERSION"
	exit 0 
fi

csc BashWrapper.cs
cp BashWrapper.exe build/workers/client/assembly/

cp bot.sh build/workers/client/assembly/

cp build/workers/bot_client/ShovelerBotClient build/workers/client/assembly/

spatial alpha cloud upload --assembly_name "shoveler-bots-$1" --environment staging --project_name loom
spatial project history snapshot upload shoveler build/seeders/tiles.snapshot --tags "shoveler-tiles-$1" --environment staging --project_name loom
