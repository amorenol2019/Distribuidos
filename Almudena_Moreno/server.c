/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 3. Server

ESCUCHAR EN TODAS LAS INTERFACES
ESPERANDO LOS MENSAJES DE LOS CLIENTES
1. RECIBE UN MENSAJE escritor. 
    aumenta contador: counter++
    Si existe, coge ese y aumenta, 
    si no, lo crea
    'server_output.txt'

    [SECONDS.MICRO][ESCRITOR #N] modifica contador con valor X

2. RECIBE MENSAJE DE LECTOR+

    [SECONDS.MICRO][LECTOR #N] lee contador con valor X

Esto es parte de la región crítica
sleep random (75-150 ms)


TIEMPO DE ESPERA: desde que intenta entrar en la región crítica, hasta que entra
*/

#include "proxy.h"
#include <signal.h>
#include <getopt.h>

#define RED "\x1b[31m"
#define RESET_COLOR    "\x1b[0m"
#define USAGE  "Incorrect program call.\nUsage: ./server --port PORT --priority writer/reader (optional) --ratio NUM_RATIO"


int main (int argc, char *argv[]) {
    setbuf(stdout, NULL);

    int opt = 0, long_index = 0;
    char *priority;
    int port, num_ratio;
    //signal(SIGINT, ctrlHandler);   //Close with CTRL + C

    if(argc != 5 && argc != 7) {
        error(RED USAGE RESET_COLOR);
    }

    static struct option long_options[] =
    {
        {"port", required_argument, 0, 'o'},
        {"priority", required_argument, 0, 'i'},
        {"ratio", optional_argument, 0, 'r'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv,"o:i:r:", 
                    long_options, &long_index )) != -1) {
        switch (opt) {
            case 'o' : port = atoi(optarg);
                break;
            case 'i' : priority = optarg;
                break;
            case 'r' : num_ratio = atoi(optarg); 
                break;
            default:
                error(RED USAGE RESET_COLOR); 
        }
    }

    if ((strcmp(priority, "writer") != 0) && (strcmp(priority, "reader") != 0)) {
        error(RED USAGE RESET_COLOR);
    }

    set_client(port);

    FILE* file;
    file = fopen("server_output.txt", "w");

    connect_server();
    signal(SIGINT, ctrlHandlerServer);   //Close with CTRL + C

    while(1) {
        recv_client();
    }

    return 0; 
}