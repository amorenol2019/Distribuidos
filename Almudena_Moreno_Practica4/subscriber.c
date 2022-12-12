/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 4. Subscriber
*/

#include "proxy.h"
#include <getopt.h>

#define RED "\x1b[31m"
#define RESET_COLOR    "\x1b[0m"
#define MAX_ARGUMENTS 7
#define USAGE  "Incorrect program call.\nUsage: ./subscriber --ip $BROKER_IP --port $BROKER_PORT --topic $TOPIC\n"


int main (int argc, char *argv[]) {
    setbuf(stdout, NULL);
    int opt = 0, long_index = 0;
    char *topic, *ip;
    int port;

    if(argc != MAX_ARGUMENTS) {
        printf(RED USAGE RESET_COLOR);
        exit(EXIT_FAILURE);
    }

    static struct option long_options[] =
    {
        {"ip", required_argument, 0, 'i'},
        {"port", required_argument, 0, 'p'},
        {"topic", required_argument, 0, 't'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv,"i:p:t:", 
                    long_options, &long_index )) != -1) {
        switch (opt) {
            case 'i' : ip = optarg;
                break;
            case 'p' : port = atoi(optarg);
                break;
            case 't' : topic = optarg;
                break;
            default:
                printf(RED USAGE RESET_COLOR);
                exit(EXIT_FAILURE);
        }
    }
    
    signal(SIGINT, ctrlHandlerClient);   //Close with CTRL + C

    read_or_write(ip, port, topic, "subscriber");

    return 0;
}