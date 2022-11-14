#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>

enum operations {
    WRITE = 0,   //Quiere escribir
    READ         //Quiere leer
};

struct request {
    enum operations action;   //WRITE O READ
    unsigned int id;          //ID Cliente
};

struct response {
    enum operations action;
    unsigned int counter;     //Valor del contador
    long waiting_time;        //Tiempo que ha esperado para llevar a cabo su 'action'
};

void error(char *message);
void ctrlHandler(int num);

