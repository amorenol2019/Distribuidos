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

struct thread_clients {
    enum operations action;   //WRITE O READ
    unsigned int num_threads;          //ID Cliente
};

void error(char *message);
void ctrlHandler(int num);
void ctrlHandlerServer(int num);

/*////////////////////////////---------------------SERVER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_client (unsigned int port);

int connect_server();

void recv_client();
void *msg_ready();

int notify_shutdown_now(char name[2]);

int close_server();

void ctrlHandlerServer(int num);

/*////////////////////////////---------------------CLIENT---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_ip_port (char* ip, unsigned int port);
void create_socket();
void *connect_client();
void read_or_write(char* ip, int port, int threads, char* mode);

void notify_ready_shutdown();

void recv_shutdown_now(char id_client[2]);
void *msg_shutdown(void *idc);

int close_client();

void ctrlHandler(int num);
