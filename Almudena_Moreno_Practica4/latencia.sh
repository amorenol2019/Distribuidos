#!/bin/sh

for i in `seq 1 500`; do
    cat secuencial_500_$i.txt >> secuencial_500.txt

done

#cat secuencial_50_1.txt | grep 'Latencia: ' | cut -d: -f6 > secuencial_50_grep.txt
#cat secuencial_500.txt | grep 'Latencia: ' | cut -d: -f6 > secuencial_500_grep.txt
#cat secuencial_900.txt | grep 'Latencia: ' | cut -d: -f6 > secuencial_900_grep.txt

#cat paralelo_50.txt | grep 'Latencia: ' | cut -d: -f6 > paralelo_50_grep.txt
#cat paralelo_500.txt | grep 'Latencia: ' | cut -d: -f6 > paralelo_500_grep.txt
#cat paralelo_900.txt | grep 'Latencia: ' | cut -d: -f6 > paralelo_900_grep.txt

#cat justo_50.txt | grep 'Latencia: ' | cut -d: -f6 > justo_50_grep.txt
#cat justo_500.txt | grep 'Latencia: ' | cut -d: -f6 > justo_500_grep.txt
#cat justo_900.txt | grep 'Latencia: ' | cut -d: -f6 > justo_900_grep.txt