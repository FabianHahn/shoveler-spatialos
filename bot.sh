#/bin/bash

echo "Connecting $1 bots with prefix $2 to $3:$4..."

for i in $(eval echo {1..$1}); do
	./ShovelerBotClient "$2$i" $3 $4 >"bot$i.log" &
	sleep 1
done
