#!/bin/bash

# Touch scores file
if [ -e ./scores.txt ]; then
	rm ./scores.txt
fi

touch scores.txt

# Build
if [ -e ./a.out ]; then
	rm ./a.out
fi
g++ ../main.cpp
echo "Compile done"

# Generate Inputs
cargo run --release --bin gen seeds.txt

# test
for FILE in  ./in/*; do
	fileName=`basename ${FILE}` && cargo run --release --bin tester ./a.out < ./in/${fileName} > ./out/${fileName}
done

# Calc scores
sum=0
while read LINE
do
	SUM=$(($SUM + $LINE))
done <./scores.txt 
	
echo $SUM

# Remove unused file
rm ./scores.txt a.out
