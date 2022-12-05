/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 4. Proxy
Comunicación sincrona, cuando te llega unregistro lo comprueba y ...
¿Que sentido tiene tener dos threads?


NUNCA MUEREN, SOLO CUANDO HAGO CTRL+C
*/

#include "proxy.h"

#define BACKLOG 2000
#define MAX_CLIENTS 1000
#define MAX_LIMIT_PUB 100
#define MAX_LIMIT_SUB 900

//OTHERS
int sockfd = 0;
pthread_t client;

struct sockaddr_in sock;
struct sockaddr_in sock_serv;

//SERVER VARIABLES
int connfd, counter_pub = 0, counter_sub = 0;

//CLIENT VARIABLES
int sock_cli, port_c;
char* ip_c;

//PUBLISHER
struct message msg_register;
struct response msg_resp;
/*////////////////////////////---------------------AUXILIAR FUNCTIONS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

void error(char *message) {
    printf("%s\n", message);
    exit(EXIT_FAILURE);
}

/*////////////////////////////---------------------BROKER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_server (unsigned int port) {
    srand(time(NULL));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Socket creation failed...");
    }

    bzero(&sock_serv, sizeof(sock_serv));
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_serv.sin_port = htons(port);
}

void communicate_server(unsigned int port) {
    const int enable = 1;

    set_server(port);

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        close_server();
        error("setsockopt(SO_REUSEADDR) failed");
    }

    if((bind(sockfd, (struct sockaddr*)&sock_serv, sizeof(sock_serv))) == -1) {
        error("Socket bind failed...");
    } 

    if((listen(sockfd, BACKLOG)) == -1) {
        close_server();
        error("Listen failed...");
    }
}

void recv_client() {
    /*Server is waiting for clients*/
    while(1) {
        pthread_t c_server;
        connfd = accept(sockfd,(struct sockaddr *)NULL, NULL);
        if (connfd < 0) {
            close(sockfd);
            close(connfd);
            error("Failed server accept...");
        } else {
            pthread_create(&c_server, NULL, &communicate_client, &connfd);
        }
    }
}

void send_message(int counter, struct message msg_recv, int limit, int connfd_send, char* type) {
    struct response msg_response;

    printf("(%d) %s conectado : %s\n", counter, type, msg_recv.topic);
    if (counter == limit - 1) {
        msg_response.response_status = LIMIT;
    } else {
        msg_response.response_status = OK;
    }   
    msg_response.id = counter;
    if (send(connfd_send, &msg_response, sizeof(msg_response), 0) < 0) {
        error("Send to server failed...\n");
    }
}

void *communicate_client(void *arg) {
    int connfd_ = *(int *) arg;
    char* mode;
    struct message msg_recv;
    struct timeval wait_time_init;

    if (gettimeofday(&wait_time_init, 0) == -1) {
        error("Error getting stamp of time");
    }
    if ((recv(connfd_, (void *) &msg_recv, sizeof(msg_recv), 0)) == -1) {
        error("Recv from the client failed...\n");
    }
    printf("[%ld.%ld] Nuevo cliente ", wait_time_init.tv_sec, wait_time_init.tv_usec);
    if (msg_recv.action == REGISTER_PUBLISHER) {
        mode = "Publicador";
        send_message(counter_pub++, msg_recv, MAX_LIMIT_PUB, connfd_, "Publicador");
        
    } else if (msg_recv.action == REGISTER_SUBSCRIBER) {
        mode = "Suscriptor";
        send_message(++counter_sub, msg_recv, MAX_LIMIT_SUB, connfd_, "Subscriptor");
    }
        
    
    return 0;
}

/*
Cuando se desconectan
Resumen:
$TOPIC1: M Suscriptores - N Publicadores
$TOPIC2: M Suscriptores - N Publicadores

● Cuando recibe un mensaje de un publicador.
[SECONDS.NANOSECONDS] Recibido mensaje para publicar en topic: $TOPIC -
mensaje: $data - Generó: $time_generated_data
● Cuando envía un mensaje a los suscriptores
[SECONDS.NANOSECONDS] Enviando mensaje en topic $TOPIC a $N suscriptores.
*/

int close_server() {
    if(close(sockfd) == -1) {
        error("Close failed");
    }

    return 0;
}


void ctrlHandlerServer(int num) {
    close_server();
    printf("\n");
    exit(EXIT_SUCCESS);
}

/*/////////////////////////////---------------------CLIENTS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_ip_port () {
    bzero(&sock, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = inet_addr(ip_c);
    sock.sin_port = htons(port_c);
}

int create_socket() {
    const int enable = 1;
    int socket_cli;
    socket_cli = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_cli == -1) {
        error("Socket creation failed...");
    }

    if (setsockopt(socket_cli, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        close_server();
        error("setsockopt(SO_REUSEADDR) failed");
    }

    return socket_cli;
}

void read_or_write(char* ip_client, int port_client, char topic[], char* mode) {
    int p_s;
    ip_c = ip_client;
    port_c = port_client;
    set_ip_port();

    if (strcmp(mode, "publisher") == 0) {
        msg_register.action = REGISTER_PUBLISHER;
        p_s = 0;
    } else if (strcmp(mode, "subscriber") == 0) {
        msg_register.action = REGISTER_SUBSCRIBER;
        p_s = 1;
    }
        
    strcpy(msg_register.topic, topic);
    
    pthread_create(&client, NULL, &connect_publisher, &p_s);
    
    if(pthread_join(client, NULL) == -1) {
        printf("Error pthread_join...\n");
        exit(EXIT_FAILURE);
    }
}

void *connect_publisher(void *arg) {
    int mode_p_s = *(int *) arg;
    char* mode_string;
    char end_line[50];
    struct timeval wait_time_init;
    
    sock_cli = create_socket();

    if((connect(sock_cli, (struct sockaddr*)&sock, sizeof(sock))) == -1) {
        error("Connection with the server failed...");
    }

    if (mode_p_s == 0) {
        mode_string = "Publisher";
        sprintf(end_line, "\n");
    } else if (mode_p_s == 1) {
        mode_string = "Subscriber";
        sprintf(end_line, "(%s:%d)\n", ip_c, port_c);
    }

    if (gettimeofday(&wait_time_init, 0) == -1) {
        error("Error getting stamp of time");
    }

    printf("[%ld.%ld] %s conectado con el broker correctamente. %s", wait_time_init.tv_sec, wait_time_init.tv_usec, mode_string, end_line);

    if (send(sock_cli, &msg_register, sizeof(msg_register), 0) == -1) {
        printf("Send to server failed...\n");
    }

    if ((recv(sock_cli, (void *) &msg_resp, sizeof(msg_resp), 0)) == -1) {
        error("Receive message failed...");
    }

    close_client();

    return 0;
}

int close_client(){
    close(sock_cli);
    
    return 0;
}


/*/////////////////////////////---------------------PUBLISHER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/


/*/////////////////////////////---------------------SUBSCRIBER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
