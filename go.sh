#!/bin/sh

ulimit -c unlimited

i = 0
while true; do
	rm -f core
	./test/mutex_test
	if [ -f "./core" ]; then
		echo "hahaha"
		break
	fi
	i=$(($i+1)) 
done
echo "create core file"
echo $i
