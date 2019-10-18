#!/bin/bash

if [ $# -lt 1 ]; then 
	echo "Usage: $0 VERSION"
	exit 0 
fi


csc BashWrapper.cs
cp BashWrapper.exe ../shoveler-spatialos-release/build/shoveler-spatialos/build/workers/client/assembly/

cp bot.sh ../shoveler-spatialos-release/build/shoveler-spatialos/build/workers/client/assembly/

cp ../shoveler-spatialos-release/build/shoveler-spatialos/build/workers/bot_client/ShovelerBotClient ../shoveler-spatialos-release/build/shoveler-spatialos/build/workers/client/assembly/

pushd ../shoveler-spatialos-release/build/shoveler-spatialos

spatial alpha cloud upload --assembly_name "shoveler-bots-$1" --environment staging --project_name loom
spatial project history snapshot upload shoveler build/seeders/tiles.snapshot --tags "shoveler-tiles-$1" --environment staging --project_name loom

popd
