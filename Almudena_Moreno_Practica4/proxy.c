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
char* broker_mode;
struct n_topic topics[MAX_LIMIT_TOPICS];

pthread_mutex_t mutex_limit;
pthread_cond_t  limit_max;
pthread_barrier_t barrier;

//CLIENT VARIABLES
int sock_cli, port_c;
char *ip_c, *mode_c;

//PUBLISHER
struct message msg_register;
struct response msg_resp;
FILE* file;

/*////////////////////////////---------------------AUXILIAR FUNCTIONS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

void error(char *message) {
    printf("%s\n", message);
    exit(EXIT_FAILURE);
}

struct timespec take_time(char* option){
    struct timespec wait_time_init;

    if (timespec_get(&wait_time_init, TIME_UTC) == -1) {
        error("Error getting stamp of time");
    }
    if (strcmp(option, "print") == 0) {
        printf("[%ld.%ld]", wait_time_init.tv_sec, wait_time_init.tv_nsec);
    }

    return wait_time_init;
}
/*////////////////////////////---------------------BROKER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_server (unsigned int port, char* mode) {
    srand(time(NULL));

    broker_mode = mode;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Socket creation failed...");
    }

    bzero(&sock_serv, sizeof(sock_serv));
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_serv.sin_port = htons(port);
}

void communicate_server(unsigned int port, char* mode) {
    const int enable = 1;

    set_server(port, mode);

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

void *communicate_client(void *arg) {
    int connfd_ = *(int *) arg, unregistered = 0;
    struct message msg_recv;

    if ((recv(connfd_, (void *) &msg_recv, sizeof(msg_recv), 0)) == -1) {
        error("Recv from the client failed...\n");
    }
    
    take_time("print");
    printf(" Nuevo cliente ");
    if (msg_recv.action == REGISTER_PUBLISHER) {
        send_message(counter_pub++, msg_recv, MAX_LIMIT_PUB, connfd_, "Publicador");
        while (!unregistered) {
            struct message public_topic;
            if ((recv(connfd_, (void *) &public_topic, sizeof(public_topic), 0)) == -1) {
                error("Recv from the client failed...\n");   //Que no salga esto si no se hace el unregistered
            } else {
                if (public_topic.action == PUBLISH_DATA) {
                    receive_data(public_topic);
                    sending_data(public_topic);
                } else if (public_topic.action == UNREGISTER_PUBLISHER) {
                    unregistered = 1;
                    unregister(public_topic, "Publicador");
                }
            }
        }
    } else if (msg_recv.action == REGISTER_SUBSCRIBER) {
        send_message(++counter_sub, msg_recv, MAX_LIMIT_SUB, connfd_, "Suscriptor");        
        while (!unregistered) {
            struct message public_topic;
            
            if ((recv(connfd_, (void *) &public_topic, sizeof(public_topic), 0)) == -1) {
                error("Recv from the client failed...\n");
            } else if (public_topic.action == UNREGISTER_SUBSCRIBER) {
                unregistered = 1;
                unregister(public_topic, "Suscriptor");
            }
        }
        
    }
    delete_topic();
    close(connfd_);
        

    return 0;
}

int exists(char topic[100], char* type, int fd, int counter_id) {
    int return_value = 1;
    printf("Resumen: \n");
    for (int i = 0; i < counter_topics; i++) {
        if (strcmp(topic, topics[i].name) == 0) {
            return_value = 0;
            if (strcmp(type, "Publicador") == 0) {
                topics[i].pub++;
            } else if (strcmp(type, "Suscriptor") == 0) {
                struct Node *t_node = (struct Node *) malloc(sizeof(struct Node));
                t_node->id = counter_id;
                t_node->fd = fd;
                t_node->next = NULL;  

                if (topics[i].sub == 0) {  //If there are no sub, does not exist first and last
                    topics[i].first = t_node;
                    topics[i].last = t_node;
                } else {
                    topics[i].last->next = t_node;
                    topics[i].last = t_node;
                }
                topics[i].sub++;
            }

        }
        printf("%s: %d Suscriptores - %d Publicadores\n", topics[i].name, topics[i].sub, topics[i].pub);
    }

    return return_value;
}

int free_topic() {
    for (int i = 0; i < counter_topics; i++) {
        if (strcmp(topics[i].name, "NULL") == 0) {
            return i;
        }
    }

    return counter_topics;
}

void send_message(int counter, struct message msg_recv, int limit, int connfd_send, char* type) {
    struct response msg_response;

    printf("(%d) %s conectado : %s\n", counter, type, msg_recv.topic);

    //Topic does not exist and there are enough free n topics
    if (exists(msg_recv.topic, type, connfd_send, counter) == 1 && counter_topics < MAX_LIMIT_TOPICS) {
        struct n_topic new_topic;
        sprintf(new_topic.name, "%s", msg_recv.topic);
        if (strcmp(type, "Publicador") == 0) {
            new_topic.pub = 1;
            new_topic.sub = 0;
        } else if (strcmp(type, "Suscriptor") == 0) {
            new_topic.pub = 0;
            new_topic.sub = 1;

            struct Node *t_node = (struct Node *) malloc(sizeof(struct Node));
            t_node->id = counter;
            t_node->fd = connfd_send;
            t_node->next = NULL;

            new_topic.first = t_node;
            new_topic.last = t_node;
        }

        topics[free_topic()] = new_topic;

        printf("%s: %d Suscriptores - %d Publicadores\n", msg_recv.topic, topics[counter_topics].sub, topics[counter_topics].pub);
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

/*
void print_list_node() {
    for (int i = 0; i < counter_topics; i++) {
        printf("LIST OF NODES: \nTOPIC %s n_sub %d n_pub %d \n", topics[i].name, topics[i].sub, topics[i].pub);
        struct Node *node;
        node = topics[i].first;
        printf("    FIRST: \n  ID %d, FD, %d, next id,", node->id, node->fd);
        for (int j = 0; j < topics[i].sub - 1; j++) {
            printf("NODES \n");
            if (topics[i].sub == 1) {
                printf("    ID %d, FD, %d", node->id, node->fd);
            } else {
                printf("   ID %d, FD, %d, next id, %d", node->id, node->fd, node->next->id);
            }
            node = node->next;
            printf("\n");
        }
        printf("    LAST: \n  ID %d, FD, %d, next id,", topics[i].last->id, topics[i].last->fd);
    }
}*/

char* switch_status(enum status response_status) {
    if (response_status == OK) {
        return "OK";
    } else if (response_status == LIMIT) {
        return "LIMIT";
    } else {
        return "ERROR";
    }
}

void receive_data(struct message receive) {
    char data[100];
    struct timespec time;
    take_time("print");
    
    strcpy(data, receive.data.data);
    time = receive.data.time_generated_data;
    printf(" Recibido mensaje para publicar en topic: %s - mensaje: %s - Generó: %ld.%ld\n", receive.topic, data, time.tv_sec, time.tv_nsec);
}

void sending_data(struct message publish) {
    if (strcmp(broker_mode, "secuencial") == 0)  {
        sending_data_secuencial(publish);
    } else if (strcmp(broker_mode, "paralelo") == 0) {
        sending_data_parallel(publish);
    } else if (strcmp(broker_mode, "justo") == 0) {
        sending_data_parallel(publish);
    }
}

void sending_data_secuencial(struct message publish) {
    struct publish msg_topic;
    struct n_topic topic_pub;
    
    for (int i = 0; i < counter_topics; i++) {
        if (strcmp(publish.topic, topics[i].name) == 0) {
            topic_pub = topics[i];
            break;
        }
    }

    if (topic_pub.sub == 0) {
        printf("There are no subscribers in this topic.\n");
    } else if (topic_pub.sub == 1){
        struct Node *node;
        int connfd_topic;
        node = topic_pub.first;
        connfd_topic = node->fd;
        msg_topic = publish.data;
        take_time("print");
        printf(" Enviando mensaje en topic %s a %d suscriptores.\n", topic_pub.name, topic_pub.sub);
        if (send(connfd_topic, &msg_topic, sizeof(msg_topic), 0) == -1) {
            error("Send to server failed...\n");
        }
    } else {
        take_time("print");
        printf(" Enviando mensaje en topic %s a %d suscriptores.\n", topic_pub.name, topic_pub.sub);
        struct Node *node;
        node = topic_pub.first;
        for (int i = 0; i < topic_pub.sub; i++) {
            int connfd_topic = node->fd;
            msg_topic = publish.data;
            if (send(connfd_topic, &msg_topic, sizeof(msg_topic), 0) == -1) {
                error("Send to server failed...\n");
            }
            node = node->next;
        }
    }

}

void sending_data_parallel(struct message publish) {
    struct publish msg_topic;
    struct n_topic topic_pub;
    
    for (int i = 0; i < counter_topics; i++) {
        if (strcmp(publish.topic, topics[i].name) == 0) {
            topic_pub = topics[i];
            break;
        }
    }

    if (topic_pub.sub == 0) {
        printf("There are no subscribers in this topic.\n");
    } else if (topic_pub.sub == 1){
        take_time("print");
        printf(" Enviando mensaje en topic %s a %d suscriptores.\n", topic_pub.name, topic_pub.sub);
        pthread_t t_broker;
        struct argThread parallel;
        parallel.node = topic_pub.first;
        parallel.msg_publish = publish.data;
        pthread_create(&t_broker, NULL, &thread_one_sus, &parallel);

    } else {
        take_time("print");
        printf(" Enviando mensaje en topic %s a %d suscriptores.\n", topic_pub.name, topic_pub.sub);
        struct Node *node;
        node = topic_pub.first;
        for (int i = 0; i < topic_pub.sub; i++) {
            pthread_t t_broker;
            struct argThread parallel;
            parallel.node = node;
            parallel.msg_publish = publish.data;
            node = node->next;
            pthread_create(&t_broker, NULL, &thread_one_sus, &parallel);
        }
    }
}

void *thread_one_sus(void *arg) {
    struct argThread parallel = *(struct argThread *)arg;
    struct Node *node;
    int connfd_topic;
    struct publish msg_topic;
    node = parallel.node;
    connfd_topic = node->fd;
    msg_topic = parallel.msg_publish;
    
    if (send(connfd_topic, &msg_topic, sizeof(msg_topic), 0) == -1) {
        error("Send to server failed...\n");
    }

    return 0;
}

void print_list_topics(char* topic, char* type) {
    printf("Resumen: \n");
    for (int i = 0; i < counter_topics; i++) {
        if (strcmp(topic, topics[i].name) == 0) {
            if (strcmp(type, "Publicador") == 0) {
                topics[i].pub--;
            } else if (strcmp(type, "Suscriptor") == 0) {
                topics[i].sub--;
            }
        }
        printf("%s: %d Suscriptores - %d Publicadores\n", topics[i].name, topics[i].sub, topics[i].pub);
    }
}

void unregister(struct message receive, char* type) {
    if (strcmp(type, "Suscriptor") == 0) {
        //Eliminar nodo
        int deleted = 0;
        for (int i = 0; i < counter_topics; i++) {
            if (strcmp(receive.topic, topics[i].name) == 0) {
                struct Node* current_node = topics[i].first;
                if (current_node->id == receive.id) {
                    topics[i].first = current_node->next;
                    topics[i].sub--;
                    deleted = 1;
                    free(current_node);
                } else {
                    struct Node* node_to_delete = current_node->next;
                    for (int j = 0; j < topics[i].sub - 1; j++) {
                        if(node_to_delete->id == receive.id) {
                            //eliminar nodo
                            topics[i].sub--;
                            deleted = 1;
                            if (j == topics[i].sub - 1) {
                                current_node->next = NULL;
                                topics[i].last = current_node;
                            } else {
                                current_node->next = node_to_delete->next;
                            }
                            free(node_to_delete);
                            break;
                        } else {
                            current_node = node_to_delete;
                            node_to_delete = node_to_delete->next;
                        }
                    }
                }

                if (deleted == 0) {
                    error("This subscriber does not exist");
                }
            }
        }
    }

    take_time("print");
    printf(" Eliminado cliente (%d) %s : %s\n", receive.id, type, receive.topic);
    print_list_topics(receive.topic, type);
}

void delete_topic() {
    for (int i = 0; i < counter_topics; i++) {
        if ((topics[i].sub + topics[i].pub) == 0) {
            sprintf(topics[i].name, "%s", "NULL");
            counter_topics--;
        }
    }
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

void ctrlHandlerBroker() {
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
    ip_c = ip_client;
    port_c = port_client;
    topic_client = topic;
    set_ip_port();
    mode_c = mode;
    
    strcpy(msg_register.topic, topic);
    if (strcmp(mode, "publisher") == 0) {
        msg_register.action = REGISTER_PUBLISHER;
        pthread_create(&client, NULL, &connect_publisher, NULL);
    } else if (strcmp(mode, "subscriber") == 0) {
        msg_register.action = REGISTER_SUBSCRIBER;
        pthread_create(&client, NULL, &connect_subscriber, NULL);
    }
    
    if(pthread_join(client, NULL) == -1) {
        printf("Error pthread_join...\n");
        exit(EXIT_FAILURE);
    }
}

void ctrlHandlerClient() {
    if (strcmp(mode_c, "publisher") == 0) {
        pub_unregister();
    } else if (strcmp(mode_c, "subscriber") == 0) {
        sub_unregister();
    }
    
    close_client();
    printf("\n");
    exit(EXIT_SUCCESS);
}

int close_client(){
    close(sock_cli);
    
    return 0;
}

/*/////////////////////////////---------------------PUBLISHER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void open_fd(char mode[2]) {
    if ((file = fopen("/proc/loadavg", mode)) == NULL) {
        error("Error opening file");
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

void close_fd() {
    if (fclose(file) == EOF) {
        error("Error closing file");
    }
}

void *connect_publisher(void *arg) {
    char* message;
    sock_cli = create_socket();

    if((connect(sock_cli, (struct sockaddr*)&sock, sizeof(sock))) == -1) {
        error("Connection with the server failed...");
    }

    take_time("print");
    printf(" Publisher conectado con el broker correctamente.\n");

    if (send(sock_cli, &msg_register, sizeof(msg_register), 0) == -1) {
        printf("Send to server failed...\n");
    }

    if (recv(sock_cli, (void *) &msg_resp, sizeof(msg_resp), 0) == -1) {
        error("Error receiving message from broker");
    } else {
        take_time("print");
        if (msg_resp.response_status == OK) {
            id_client = msg_resp.id;
            printf(" Registrado correctamente con ID: %d para topic %s\n", msg_resp.id, msg_register.topic);
        } else { //if (msg_resp.response_status == LIMIT || msg_resp.response_status == ERROR)
            sprintf(message, " Error al hacer el registro: error=%s", switch_status(msg_resp.response_status));
            error(message);
        }
    }
    
    while(1) {
        send_publisher();
        sleep(3);
    }

    close_client();

    return 0;
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
    
    take_time("print");
    printf(" Publicado mensaje topic: %s - mensaje: %s - Generó: %ld.%ld\n", msg_publisher.topic, data_published.data, publish_time.tv_sec, publish_time.tv_nsec);
}

void pub_unregister() {
    struct message msg_publisher;

    msg_publisher.action = UNREGISTER_PUBLISHER;
    strcpy(msg_publisher.topic, topic_client);
    msg_publisher.id = id_client;

    if (send(sock_cli, &msg_publisher, sizeof(msg_publisher), 0) == -1) {
        printf("Send to server failed...\n");
    }

    take_time("print");
    printf(" De-Registrado (%d) correctamente del broker.\n", id_client);
}

/*/////////////////////////////---------------------SUBSCRIBER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

void receive_topic() {
    struct publish msg_publisher;
    struct timespec wait_current;
    int latency;

    if (recv(sock_cli, (void *) &msg_publisher, sizeof(msg_publisher), 0) == -1) {
        printf("Send to server failed...\n");
    }
    wait_current = take_time("print");
    long init_time =  msg_publisher.time_generated_data.tv_sec * 1e9 +  msg_publisher.time_generated_data.tv_nsec;
    long end_time = wait_current.tv_sec * 1e9 + wait_current.tv_nsec;

    latency = end_time - init_time;
    printf(" Recibido mensaje topic: %s - mensaje: %s - Generó: %ld.%ld - Recibido: %ld.%ld - Latencia: %d.\n", topic_client, msg_publisher.data, msg_publisher.time_generated_data.tv_sec, msg_publisher.time_generated_data.tv_nsec, wait_current.tv_sec, wait_current.tv_nsec, latency);
}   

void sub_unregister() {
    struct message msg_subscriber;

    msg_subscriber.action = UNREGISTER_SUBSCRIBER;
    strcpy(msg_subscriber.topic, topic_client);
    msg_subscriber.id = id_client;

    if (send(sock_cli, &msg_subscriber, sizeof(msg_subscriber), 0) == -1) {
        printf("Send to server failed...\n");
    }

    take_time("print");
    printf("De-Registrado (%d) correctamente del broker\n", id_client);
}

void *connect_subscriber(void *arg) {
   
    sock_cli = create_socket();

    if((connect(sock_cli, (struct sockaddr*)&sock, sizeof(sock))) == -1) {
        error("Connection with the server failed...");
    }
    
    take_time("print");
    printf(" Subscriber conectado con el broker correctamente. (%s:%d)\n", ip_c, port_c);

    if (send(sock_cli, &msg_register, sizeof(msg_register), 0) == -1) {
        printf("Send to server failed...\n");
    }

    if ((recv(sock_cli, (void *) &msg_resp, sizeof(msg_resp), 0)) == -1) {
        error("Error receiving message from broker");
    } else {
        take_time("print");
        if (msg_resp.response_status == OK) {
            id_client = msg_resp.id;
            printf(" Registrado correctamente con ID: %d para topic %s\n", msg_resp.id, msg_register.topic);
        } else if (msg_resp.response_status == LIMIT) {
            char* message;
            sprintf(message, " Error al hacer el registro: %s", switch_status(msg_resp.response_status));
            error(message);
        }
    }

    while(1) {
        receive_topic();
    }
    
    sub_unregister();
    
    close_client();

    return 0;
}
