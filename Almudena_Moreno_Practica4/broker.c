/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 4. Broker

Broker aceptará un máximo de 10 topics distintos, 100 publicadores y 900 suscriptores
MODOS: secuencial, paralelo, justo
*/

#include "proxy.h"
#include <getopt.h>

#define RED "\x1b[31m"
#define RESET_COLOR    "\x1b[0m"
#define MAX_ARGUMENTS 5
#define USAGE  "Incorrect program call.\nUsage: ./broker --port $BROKER_PORT --mode $MODE {secuencial, paralelo, justo}\n"


int main (int argc, char *argv[]) {
    setbuf(stdout, NULL);
    int opt = 0, long_index = 0;
    char *mode;
    int port;

    if(argc != MAX_ARGUMENTS) {
        printf(RED USAGE RESET_COLOR);
        exit(EXIT_FAILURE);
    }

    static struct option long_options[] =
    {
        {"port", required_argument, 0, 'p'},
        {"mode", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv,"p:m:", 
                    long_options, &long_index )) != -1) {
        switch (opt) {
            case 'p' : port = atoi(optarg);
                break;
            case 'm' : mode = optarg;
                break;
            default:
                printf(RED USAGE RESET_COLOR);
                exit(EXIT_FAILURE);
        }
    }

    if ((strcmp(mode, "secuencial") != 0) && (strcmp(mode, "paralelo") != 0) && (strcmp(mode, "justo") != 0)) {
        error(RED USAGE RESET_COLOR);
    }

    communicate_server(port);
    return 0;
}