#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>

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

struct n_sub_pub {
    int pub;
    int sub;
};

/*////////////////////////////---------------------AUXILIAR FUNCTIONS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void error(char *message);
//void open_fd(char mode[2]);
//void close_fd();
//void write_fd();
//void read_fd();

/*////////////////////////////---------------------BROKER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_server (unsigned int port);
void communicate_server(unsigned int port);

void recv_client();
void *communicate_client(void *arg);

int close_server();

//void close_each_thread(int connfd_socket);

void ctrlHandlerBroker(int num);

/*////////////////////////////---------------------PUBLISHER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_ip_port ();
int create_socket();
void read_or_write(char* ip_client, int port_client, char topic[], char* mode);
void *connect_publisher(void *arg);
int close_client();

void ctrlHandler(int num);

