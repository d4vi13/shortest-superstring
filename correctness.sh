#!/bin/bash

files=("dna" "5k" "2.5k" "500" "10k")
threads=(16 1)

mkdir answer
touch correct

echo "" > correct

for file in "${files[@]}"; do
  if [ -f $file ]; then
    echo "Calculating answer for $file"
    ./shsup tests/$file 16 
  fi

  echo "Checking if string contains all strings..."
  tail -n +2 tests/$file | while read line; do
    echo "$line..."
    grep -v $line answer/$file > correct 
    if [ -s correct ]; then
      echo "string $line isnt on the answer"
      exit 1
    fi
  done 

done 

rm correct
