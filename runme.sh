#!/bin/bash
make build

count=10
pids=()
for (( i=0; i<$count; i++ ))
do
	./file_lock test &
	pids+=($!)
done

sleep 5m

for pid in ${pids[@]}
do
	kill -SIGINT $pid
done


cat statistic >> statistics.txt
rm statistic
