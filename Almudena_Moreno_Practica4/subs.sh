#!/bin/sh

for i in `seq 1 500`; do
    (./subscriber --ip 127.0.0.1 --port 8080 --topic A | grep 'Latencia: ' | cut -d: -f6 | sed 's/.$//g' | tee -a secuencial_500_$i.txt )&
    sleep 0.001

    #(./subscriber --ip 127.0.0.1 --port 8080 --topic A | grep 'Latencia: ' | cut -d: -f6 | sed 's/.$//g' | tee -a secuencial_50_$i.txt )&
    #(./subscriber --ip 127.0.0.1 --port 8080 --topic A | grep 'Latencia: ' | cut -d: -f6 | sed 's/.$//g' | tee -a secuencial_900_$i.txt )&
    #(./subscriber --ip 127.0.0.1 --port 8080 --topic A | grep 'Latencia: ' | cut -d: -f6 | sed 's/.$//g' | tee -a paralelo_50_$i.txt )&
    #(./subscriber --ip 127.0.0.1 --port 8080 --topic A | grep 'Latencia: ' | cut -d: -f6 | sed 's/.$//g' |tee -a paralelo_500_$i.txt )&
    #(./subscriber --ip 127.0.0.1 --port 8080 --topic A | grep 'Latencia: ' | cut -d: -f6 | sed 's/.$//g' |tee -a paralelo_900_$i.txt )&
    #(./subscriber --ip 127.0.0.1 --port 8080 --topic A | grep 'Latencia: ' | cut -d: -f6 | sed 's/.$//g' |tee -a justo_50_$i.txt )&
    #(./subscriber --ip 127.0.0.1 --port 8080 --topic A | grep 'Latencia: ' | cut -d: -f6 | sed 's/.$//g' |tee -a justo_500_$i.txt )&
    #(./subscriber --ip 127.0.0.1 --port 8080 --topic A | grep 'Latencia: ' | cut -d: -f6 | sed 's/.$//g' |tee -a justo_900_$i.txt )&
    
done

#cat subs_salida.txt | awk -F "Latencia: " 'length($0) > 173 {print $NF}' subs_salida.txt | tee subs_salida_2.txt
