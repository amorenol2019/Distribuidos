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

/*////////////////////////////---------------------AUXILIAR FUNCTIONS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void error(char *message);
void open_fd(char mode[2]);
void close_fd();
void write_fd();
void read_fd();
void sem_create();

void sleep_random();
/*////////////////////////////---------------------SERVER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_server (unsigned int port, char *s_priority, int n_ratio);
void communicate_server(unsigned int port, char *s_priority, int n_ratio);

void recv_client();
void *communicate_client(void *arg);

int close_server();

void close_each_thread(int connfd_socket);

void ctrlHandlerServer(int num);

void errorServer(char *message, int create);

/*////////////////////////////---------------------CLIENT---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_ip_port (char* ip, unsigned int port);
int create_socket();
void *connect_client();
void read_or_write(char* ip, int port, int threads, char* mode);

void notify_ready_shutdown();

void recv_shutdown_now(char id_client[2]);
void *msg_shutdown(void *idc);

int close_client();

void ctrlHandler(int num);
