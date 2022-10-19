/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. P1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

def argEntry(int argc, char *argv[]) {
    if(argc != 3) {
        error("Incorrect program call.\n Usage: ./P1 IP_DIRECTION PORT");
    }
    char *ip_dir = argv[1];
    int port = atoi(argv[2]);

    return *ip_dir, port;
}

int main (int argc, char *argv[]) {
    //Primero de todo es abrir canales de comunicación
    int port;
    char *ip;
    *ip, port = argEntry(int argc, char *argv[]);

    return 0;
}