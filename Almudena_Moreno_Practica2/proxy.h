#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>

enum operations {
    READY_TO_SHUTDOWN = 0,
    SHUTDOWN_NOW,
    SHUTDOWN_ACK
};

struct message {
    char origin[20];
    enum operations action;
    unsigned int clock_lamport;
};

void error(char *message);
void set_name (char name[2]);
void set_ip_port (char* ip, unsigned int port);
int get_clock_lamport();
unsigned int update_lamport(struct message clock);
int p1_or_p3(char name[2]);

/*////////////////////////////---------------------SERVER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
int connect_server();

void recv_client();
void *msg_ready();

int notify_shutdown_now(char name[2]);

void recv_ack(char name[2]);
void *msg_ack(void *idc);

int close_server();

void ctrlHandlerServer(int num);

/*////////////////////////////---------------------CLIENT---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
int connect_client();

void notify_ready_shutdown();

void recv_shutdown_now(char id_client[2]);
void *msg_shutdown(void *idc);

void notify_shutdown_ack();

int close_client();

void ctrlHandler(int num);
