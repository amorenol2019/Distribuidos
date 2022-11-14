/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 3. Proxy


Semáforos, variables condición, mutex

*/

#include "proxy.h"

void error(char *message) {
    printf("%s\n", message);
    exit(EXIT_FAILURE);
}

void ctrlHandler(int num) {
    //close_server();
    exit(EXIT_SUCCESS);
}