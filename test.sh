#! /bin/bash

total=10000000	#total threads number
threads=10	#concurrent threads number
alloctor=$1

mkdir ./out
rm -f ./out/${alloctor}.out
for i in {1..23}; do
	size=$((2**$i))
	/usr/bin/time -f "$i\t%e\t%M" $alloctor $size $threads $total \
		2>>./out/${alloctor}.out
done
