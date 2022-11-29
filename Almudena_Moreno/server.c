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
#define USAGE  "Incorrect program call.\nUsage: ./server --port PORT --priority writer/reader (optional) --ratio NUM_RATIO\n"


int main (int argc, char *argv[]) {
    setbuf(stdout, NULL);

    int opt = 0, long_index = 0;
    char *priority;
    int port, num_ratio = -1;

    if(argc != 5 && argc != 7) {
        printf(RED USAGE RESET_COLOR);
        exit(EXIT_FAILURE);
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
        const char *tmp_optarg = optarg;
        switch (opt) {
            case 'o' : port = atoi(optarg);
                break;
            case 'i' : priority = optarg;
                break;
            case 'r' : 
                if(!optarg && NULL != argv[optind] && '-' != argv[optind][0]) {
                    tmp_optarg = argv[optind++];
                }
                if (tmp_optarg) {
                    num_ratio = atoi(tmp_optarg);
                    if (num_ratio == 0) {
                        printf(RED "RATIO COULD NOT BE 0.\n" RESET_COLOR);
                        exit(EXIT_FAILURE);
                    }
                }
                break;
            default:
                printf(RED USAGE RESET_COLOR);
                exit(EXIT_FAILURE);
        }
    }

    if ((strcmp(priority, "writer") != 0) && (strcmp(priority, "reader") != 0)) {
        error(RED USAGE RESET_COLOR);
    }

    communicate_server(port, priority, num_ratio);

    signal(SIGINT, ctrlHandlerServer);   //Close with CTRL + C
    sem_create();

    recv_client();

    return 0; 
}