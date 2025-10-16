#!/bin/bash

files=("dna" "10k" "5k" "2.5k" "500")
threads=(16 1)

mkdir answer
touch tmp coherence

echo "" > tmp
echo "" > coherence

for file in "${files[@]}"; do
  echo "Checking $file coherence..."
  ./shsup tests/$file 16 > answer/$file
  ./shsup tests/$file 1 > tmp

  diff answer/$file tmp > coherence

  if [ -s coherence ]; then 
    echo "Vish, something is wrong"
    exit 1
  fi

  echo "" > coherence
done 

echo "Nice, everthing coherent"

rm tmp coherence
