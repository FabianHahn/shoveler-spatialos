#/bin/bash

echo "Connecting $1 bots to $2:$3..."

for i in $(eval echo {1..$1}); do
	./ShovelerBotClient "bot$i" $2 $3 >"bot$i.log" &
	sleep 1
done
