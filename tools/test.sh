#!/bin/bash

start_time=`date +%s`

# Touch scores file
if [ -e ./scores.txt ]; then
	rm ./scores.txt
fi

# Build
if [ -e ./a.out ]; then
	rm ./a.out
fi
g++ ../main.cpp
echo "Compile done"

# Generate Inputs
cargo run --release --bin gen seeds.txt

# Test
function run_tester() {
	fileName=$1
	cargo run --release --quiet --bin tester ./a.out < ./in/${fileName} > ./out/${fileName} 2>>./scores.txt
}
export -f run_tester

# Run tests
ls ./in/ | parallel --no-notice --jobs 2 run_tester {}
# for FILE in  ./in/*; do
# 	fileName=`basename ${FILE}` && cargo run --release --quiet --bin tester ./a.out < ./in/${fileName} > ./out/${fileName} 2>>./scores.txt
# done

# Calc scores
SUM=0
while read LINE
do
	SUM=$(($SUM + $LINE))
done < ./scores.txt

echo ${SUM}

# Remove unused file
rm ./scores.txt a.out

end_time=`date +%s`

run_time=$(($end_time - $start_time))
echo "Took ${run_time}[s]"
