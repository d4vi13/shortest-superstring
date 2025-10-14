#!/bin/bash

files=("tests/dna" "tests/10k" "tests/5k" "tests/2.5k" "tests/500")
threads=(16 8 4 2 1)

touch result
echo "tam,total,sequencial,paralelo" > result

for file in "${files[@]}"; do
  for n in "${threads[@]}"; do 
    for i in {1..20}; do
      echo "Calculando $file com $n pela $i vez" 
      ./shsup $file $n >> result
    done
  done
done 
