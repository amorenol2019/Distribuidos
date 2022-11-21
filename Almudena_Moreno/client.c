/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 3. Client
Cuando acabe un thread que entre el siguiente, no esperar que acaben todos

1. MENSAJE INDICANDO QUE QUIERE ESCRIBIR/LEER
2. RECIBE COMO RESPUESTA MENSAJE CON OPERACION REALIZADA,
    CONTADOR, Y TIEMPO DE ESPERA EN EL SERVIDOR

Cada thread lanzado y ejecutado:
    (N ENTRE 0 Y NUM_THREADS)
    [Cliente #N] Lector/Escritor, contador=X, tiempo=Y ns.
*/

#include "proxy.h"
#include <signal.h>
#include <getopt.h>

#define RED "\x1b[31m"
#define RESET_COLOR    "\x1b[0m"
#define USAGE  "Incorrect program call.\nUsage: ./client --ip IP --port PORT --mode writer/reader --threads NUM_THREADS"

int main (int argc, char *argv[]) {
    setbuf(stdout, NULL);

    int opt = 0, long_index = 0;
    char *ip, *mode;
    int port, num_threads;

    if(argc != 9) {
        error(RED USAGE RESET_COLOR);
    }
    static struct option long_options[] =
    {
        {"ip", required_argument, NULL, 'i'},
        {"mode", required_argument, NULL, 'm'},
        {"port", required_argument, NULL, 'p'},
        {"threads", required_argument, NULL, 'n'},
        {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv,"i:m:p:n:", 
                    long_options, &long_index )) != -1) {
        switch (opt) {
             case 'i' : ip = optarg;
                 break;
             case 'm' : mode = optarg;
                 break;
             case 'p' : port = atoi(optarg); 
                 break;
             case 'n' : num_threads = atoi(optarg);
                 break;
             default: error(RED USAGE RESET_COLOR); 
        }
    }

    if ((strcmp(mode, "writer") != 0) && (strcmp(mode, "reader") != 0)) {
        error(RED USAGE RESET_COLOR);
    }

    signal(SIGINT, ctrlHandler);   //Close with CTRL + C

    read_or_write(ip, port, num_threads, mode);
    sleep(1);
    return 0;   
}