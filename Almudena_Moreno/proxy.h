#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

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
void close_fd();
void open_fd(char mode[2]);
void write_fd();
void read_fd();
void sem_create();

void close_thread();
/*////////////////////////////---------------------SERVER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_server (unsigned int port, char *s_priority);

int connect_server();

void recv_client();
void *communicate_client(void *arg);

int notify_shutdown_now(char name[2]);

int close_server();

void ctrlHandlerServer(int num);

/*////////////////////////////---------------------CLIENT---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_ip_port (char* ip, unsigned int port);
int create_socket(int id);
void *connect_client();
void read_or_write(char* ip, int port, int threads, char* mode);

void notify_ready_shutdown();

void recv_shutdown_now(char id_client[2]);
void *msg_shutdown(void *idc);

int close_client(int id);

void ctrlHandler(int num);
