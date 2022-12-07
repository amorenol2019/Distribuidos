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
#define MAX_LIMIT_TOPICS 10
#define L_SIZE 2000

//OTHERS
int sockfd = 0, id_client;
pthread_t client;
char* topic_client;
struct sockaddr_in sock;
struct sockaddr_in sock_serv;

//SERVER VARIABLES
int connfd, counter_pub = 0, counter_sub = 0, counter_topics = 0;

char topics[MAX_LIMIT_TOPICS][100];
struct n_sub_pub count_topic[MAX_LIMIT_TOPICS];

//CLIENT VARIABLES
int sock_cli, port_c;
char* ip_c;

//PUBLISHER
struct message msg_register;
struct response msg_resp;
FILE* file;

/*////////////////////////////---------------------AUXILIAR FUNCTIONS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

void error(char *message) {
    printf("%s\n", message);
    exit(EXIT_FAILURE);
}

void take_time(){
    struct timeval wait_time_init;

    if (gettimeofday(&wait_time_init, 0) == -1) {
        error("Error getting stamp of time");
    }

    printf("[%ld.%ld]", wait_time_init.tv_sec, wait_time_init.tv_usec);
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

int exists(char topic[100], char* type) {
    int return_value = 1, n_pub, n_sub;
    printf("Resumen: \n");
    for (int i = 0; i < counter_topics; i++) {
        if (strcmp(topic, topics[i]) == 0) {
            return_value = 0;
            if (strcmp(type, "Publicador") == 0) {
                count_topic[i].pub++;
            } else if (strcmp(type, "Suscriptor") == 0) {
                count_topic[i].sub++;
            }
        }
        printf("%s: %d Suscriptores - %d Publicadores\n", topics[i], count_topic[i].sub, count_topic[i].pub);
    }

    return return_value;
}

void send_message(int counter, struct message msg_recv, int limit, int connfd_send, char* type) {
    struct response msg_response;

    printf("(%d) %s conectado : %s\n", counter, type, msg_recv.topic);

    if (exists(msg_recv.topic, type) == 1 && counter_topics < MAX_LIMIT_TOPICS) {
        sprintf(topics[counter_topics], msg_recv.topic);
        struct n_sub_pub new_topic;
        if (strcmp(type, "Publicador") == 0) {
            new_topic.pub = 1;
            new_topic.sub = 0;
        } else if (strcmp(type, "Suscriptor") == 0) {
            new_topic.pub = 0;
            new_topic.sub = 1;
        }
        count_topic[counter_topics] = new_topic;
        printf("%s: %d Suscriptores - %d Publicadores\n", msg_recv.topic, count_topic[counter_topics].sub, count_topic[counter_topics].pub);
        counter_topics++;
    }

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

void print_list_topics(char* topic, char* type) {
    printf("Resumen: \n");
    for (int i = 0; i < counter_topics; i++) {
        if (strcmp(topic, topics[i]) == 0) {
            if (strcmp(type, "Publicador") == 0) {
                count_topic[i].pub--;
            } else if (strcmp(type, "Suscriptor") == 0) {
                count_topic[i].sub--;
            }
        }
        printf("%s: %d Suscriptores - %d Publicadores\n", topics[i], count_topic[i].sub, count_topic[i].pub);
    }
}

void unregister_publisher(struct message receive, char* type) {
    take_time();
    /*
    Resumen:
    $TOPIC1: M Suscriptores - N Publicadores
    $TOPIC2: M Suscriptores - N Publicadores
    */
    printf(" Eliminado cliente (%d) %s : %s\n", receive.id, type, receive.topic);
    print_list_topics(receive.topic, type);
}

void receive_data(struct message receive) {
    char data[100];
    struct timespec time;
    take_time();
    
    strcpy(data, receive.data.data);
    time = receive.data.time_generated_data;
    printf(" Recibido mensaje para publicar en topic: %s -mensaje: %s - Generó: %ld.%ld\n", receive.topic, data, time.tv_sec, time.tv_nsec);
}

void sending_data() {
    take_time();
    printf(" Enviando mensaje en topic $TOPIC a $N suscriptores.");
}


void *communicate_client(void *arg) {
    int connfd_ = *(int *) arg, unregistered = 0;;
    struct message msg_recv;

    
    if ((recv(connfd_, (void *) &msg_recv, sizeof(msg_recv), 0)) == -1) {
        error("Recv from the client failed...\n");
    }
    
    take_time();
    
    if (msg_recv.action == REGISTER_PUBLISHER) {
        printf(" Nuevo cliente ");
        send_message(counter_pub++, msg_recv, MAX_LIMIT_PUB, connfd_, "Publicador");
    } else if (msg_recv.action == REGISTER_SUBSCRIBER) {
        printf(" Nuevo cliente ");
        send_message(++counter_sub, msg_recv, MAX_LIMIT_SUB, connfd_, "Suscriptor");
    } 

    if (msg_recv.action == REGISTER_PUBLISHER) {
        while (!unregistered) {
            struct message public_topic;
            if ((recv(connfd_, (void *) &public_topic, sizeof(public_topic), 0)) == -1) {
                error("Recv from the client failed...\n");
            } 
            if (public_topic.action == PUBLISH_DATA) {
                receive_data(public_topic);
            } else if (public_topic.action == UNREGISTER_PUBLISHER) {
                unregistered = 1;
                unregister_publisher(public_topic, "Publicador");
            }
        }
    }
    
    close(connfd_);
        

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
    topic_client = topic;
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

    take_time();
    printf(" %s conectado con el broker correctamente. %s", mode_string, end_line);

    if (send(sock_cli, &msg_register, sizeof(msg_register), 0) == -1) {
        printf("Send to server failed...\n");
    }

    if ((recv(sock_cli, (void *) &msg_resp, sizeof(msg_resp), 0)) == -1) {
        error("Error receiving message from broker");
    } else {
        take_time();
        if (msg_resp.response_status == OK) {
            id_client = msg_resp.id;
            printf(" Registrado correctamente con ID: %d para topic %s\n", msg_resp.id, msg_register.topic);
        } else if (msg_resp.response_status == LIMIT) {
            //char* message;
            //sprintf(message, " Error al hacer el registro: error=%s", msg_resp.response_status);
            error(" Error al hacer el registro: error=%s");
        }
    }
    send_publisher();
    sleep(3);
    unregister();
    close_client();

    return 0;
}

int close_client(){
    close(sock_cli);
    
    return 0;
}


/*/////////////////////////////---------------------PUBLISHER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
/*informacion a enviar /proc/loadavg*/

void open_fd(char mode[2]) {
    if ((file = fopen("/proc/loadavg", mode)) == NULL) {
        error("Error opening file");
    }
}

void close_fd() {
    if (fclose(file) == EOF) {
        error("Error closing file");
    }
}

char* read_fd() {
    char fline[L_SIZE];
    char* msg;
    if (fgets(fline, L_SIZE, file) == NULL) {
        error("File /proc/loadavg is empty");
    } else {
        msg = fline;
    }

    return msg;
}

void send_publisher() {
    struct message msg_publisher;
    struct publish data_published;
    struct timespec publish_time;
    char* message;

    open_fd("r");
    message = read_fd();
    message[strcspn(message, "\n")] = 0;
    close_fd();
    
    //printf("%s\n", message);
    timespec_get(&publish_time, TIME_UTC);

    msg_publisher.action = PUBLISH_DATA;
    strcpy(msg_publisher.topic, topic_client);
    strcpy(data_published.data, message);
    data_published.time_generated_data = publish_time;
    msg_publisher.data = data_published;

    if (send(sock_cli, &msg_publisher, sizeof(msg_publisher), 0) == -1) {
        printf("Send to server failed...\n");
    }
    
    take_time();
    printf(" Publicado mensaje topic: %s - mensaje: %s - Generó: %ld.%ld\n", msg_publisher.topic, data_published.data, publish_time.tv_sec, publish_time.tv_nsec);
}

void unregister() {
    struct message msg_publisher;

    msg_publisher.action = UNREGISTER_PUBLISHER;
    strcpy(msg_publisher.topic, topic_client);
    msg_publisher.id = id_client;

    if (send(sock_cli, &msg_publisher, sizeof(msg_publisher), 0) == -1) {
        printf("Send to server failed...\n");
    }

    take_time();
    printf(" De-Registrado (%d) correctamente del broker.\n", id_client);
}
/*/////////////////////////////---------------------SUBSCRIBER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
/*printf([SECONDS.NANOSECONDS] Recibido mensaje topic: $topic - mensaje: $data -
Generó: $time_generated_data - Recibido: $time_received_data - Latencia:
$latency.) */