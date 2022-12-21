#!/bin/sh

for i in `seq 0 500`; do
    (./subscriber --ip 127.0.0.1 --port 8080 --topic A |tee -a paralelo_500.txt )&
    """
    (./subscriber --ip 127.0.0.1 --port 8080 --topic A |tee -a secuencial_50.txt )&
    (./subscriber --ip 127.0.0.1 --port 8080 --topic A |tee -a secuencial_500.txt )&
    (./subscriber --ip 127.0.0.1 --port 8080 --topic A |tee -a secuencial_900.txt )&
    (./subscriber --ip 127.0.0.1 --port 8080 --topic A |tee -a paralelo_50.txt )&
    (./subscriber --ip 127.0.0.1 --port 8080 --topic A |tee -a paralelo_900.txt )&
    (./subscriber --ip 127.0.0.1 --port 8080 --topic A |tee -a justo_50.txt )&
    (./subscriber --ip 127.0.0.1 --port 8080 --topic A |tee -a justo_500.txt )&
    (./subscriber --ip 127.0.0.1 --port 8080 --topic A |tee -a justo_900.txt )&
    """
done

#cat subs_salida.txt | awk -F "Latencia: " 'length($0) > 173 {print $NF}' subs_salida.txt | tee subs_salida_2.txt
