#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>

#define RED "\x1b[31m"
#define RESET_COLOR    "\x1b[0m"

enum operations {
    REGISTER_PUBLISHER = 0,
    UNREGISTER_PUBLISHER,
    REGISTER_SUBSCRIBER,
    UNREGISTER_SUBSCRIBER,
    PUBLISH_DATA
};
struct publish {
    struct timespec time_generated_data;
    char data[100];
};
struct message {
    enum operations action;
    char topic[100];
    // Solo utilizado en mensajes de UNREGISTER
    int id;
    // Solo utilizado en mensajes PUBLISH_DATA
    struct publish data;
};

enum status {
    ERROR = 0,
    LIMIT,
    OK
};

struct response {
    enum status response_status;
    int id;
};

struct n_topic {
    char name[100];
    int pub;
    int sub;
    struct Node* first;
    struct Node* last;
};

struct Node {
    int id;
    int fd;
    struct Node* next;
};

/*////////////////////////////---------------------AUXILIAR FUNCTIONS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void error(char *message);
void take_time();

/*////////////////////////////---------------------BROKER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_server (unsigned int port);
void communicate_server(unsigned int port);

void recv_client();
void *communicate_client(void *arg);
int exists(char topic[100], char* type, int fd);
void send_message(int counter, struct message msg_recv, int limit, int connfd_send, char* type);
void print_list_topics(char* topic, char* type);
void unregister_publisher(struct message receive, char* type);
void receive_data(struct message receive);
void sending_data(int connfd_topic, struct message publish);

int close_server();

void ctrlHandlerBroker(int num);

/*////////////////////////////---------------------CLIENTS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_ip_port ();
int create_socket();
void read_or_write(char* ip_client, int port_client, char topic[], char* mode);
int close_client();

//void ctrlHandler(int num);

/*////////////////////////////---------------------PUBLISHER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void open_fd(char mode[2]);
char* read_fd();
void close_fd();

void *connect_publisher(void *arg);
void send_publisher();
void pub_unregister();

/*////////////////////////////---------------------SUBSCRIBER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void *connect_subscriber(void *arg);
void receive_topic();
void sub_unregister();