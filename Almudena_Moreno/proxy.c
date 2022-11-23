/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 3. Proxy


Semáforos, variables condición, mutex
Un mutex no vale para sincronización, sirve para excluir uno a uno
VARIALES CONDICION: Un escritor no puede liberar solamente a un lector
sI NO HAY ESCRITORIES, LOS LECTORES NO SE TIENEN QUE BLOQUEAR
*/

#include "proxy.h"

#define backlog 2000
#define MAX_THREADS 250
#define MAX_CLIENTS 1000
#define L_SIZE 1024

int sockfd = 0, counter = 0, roll, num_clients = 0;
int sock_cli[MAX_CLIENTS];
int connfd[MAX_CLIENTS];

char fline[L_SIZE];
char fline[L_SIZE];

FILE* file;

pthread_t c_server[MAX_THREADS];

sem_t sem_max_threads;
sem_t mutex_server;

struct sockaddr_in sock;
struct sockaddr_in sock_serv;

struct request msg_recv;
struct request msg_client;
struct response msg_resp;
struct response msg_serv;


void error(char *message) {
    printf("%s\n", message);
    exit(EXIT_FAILURE);
}

void open_fd(char mode[2]) {
    if ((file = fopen("server_output.txt", mode)) == NULL) {
        error("Error opening file");
    }
}

void close_fd() {
    if (fclose(file) == EOF) {
        error("Error closing file");
    }
}

void write_fd() {
    if (fprintf(file, "%d\n", msg_serv.counter) < 0) {
        error("Error writing file");
    }
}

void read_fd() {
    if (fgets(fline, L_SIZE, file) == NULL) {
        msg_serv.counter = 0;
    } else {
        msg_serv.counter = atoi(fline);
    }
}

/*////////////////////////////---------------------SERVER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_client (unsigned int port) {
    const int enable = 1;

    srand(time(NULL));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Socket creation failed...");
    }
    
    bzero(&sock_serv, sizeof(sock_serv));
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_serv.sin_port = htons(port);
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        close_server();
        error("setsockopt(SO_REUSEADDR) failed");
    }

    if((bind(sockfd, (struct sockaddr*)&sock_serv, sizeof(sock_serv))) == -1) {
        error("Socket bind failed...");
    } 

    if((listen(sockfd, backlog)) == -1) {
        close_server();
        error("Listen failed...");
    }

    open_fd("a+");
    read_fd();
}

void recv_client() {
    /*Server is waiting for clients*/
    connfd[num_clients] = accept(sockfd,(struct sockaddr *)NULL, NULL);
    if (connfd[num_clients] < 0) {
        close(sockfd);
        close(connfd[num_clients]);
        error("Failed server accept...");
    }

    pthread_create(&c_server[num_clients], NULL, &communicate_client, (void *) &connfd[num_clients]);
    //pthread_join(c_server[num_clients], NULL);
    num_clients++;
}

void *communicate_client(void *arg) {
    int connfd_ = *(int *)arg;
    struct timeval wait_time_init;
    struct timeval wait_time_end;
    
    if ((recv(connfd_, &msg_recv, sizeof(msg_recv), 0)) < 0) {
        error("Recv from the client failed...\n");
    }
    if (gettimeofday(&wait_time_init, 0) == -1) {
        error("Error getting stamp of time");
    }

    sem_wait(&sem_max_threads);

    //ESPERANDO PARA ENTRAR A LA REGIÓN CRÍTICA


    sem_wait(&mutex_server);
    if (gettimeofday(&wait_time_end, 0) == -1) {
        error("Error getting stamp of time");
    }
    //Comienza la región crítica
    if (msg_recv.action == WRITE) {
        msg_serv.counter++;
        close_fd();
        open_fd("w");
        write_fd();
        printf("[%ld.%ld]", wait_time_init.tv_sec, wait_time_init.tv_usec);
        printf("[ESCRITOR #%d] modifica contador con valor %d\n", msg_recv.id, msg_serv.counter);
    } else if (msg_recv.action == READ) {
        printf("[%ld.%ld]", wait_time_init.tv_sec, wait_time_init.tv_usec);
        printf("[LECTOR #%d] lee contador con valor %d\n", msg_recv.id, msg_serv.counter);
    } else {
        error("Not action allowed");
    }


    int sleep_number = rand () % 76 + 75;
    usleep(sleep_number);
    int diff = ((wait_time_end.tv_sec - wait_time_init.tv_sec)*1000000 + wait_time_end.tv_usec) - wait_time_init.tv_usec;

    msg_serv.action = msg_recv.action;
    msg_serv.waiting_time = diff * 1000;

    if (send(connfd_, &msg_serv, sizeof(msg_serv), 0) < 0) {
        error("Send to server failed...\n");
    }
    //printf("SEND response COUNTER %d, WAITING TIME, %ld, ACTION, %d\n", msg_serv.counter, msg_serv.waiting_time, msg_serv.action);
    sem_post(&mutex_server);
    sem_post(&sem_max_threads);
    //Fin de región critica

    return 0;
}

void sem_create() {
    sem_init(&sem_max_threads, 0, MAX_THREADS);
    sem_init(&mutex_server, 0, 1);

}

int close_server() {
    if(close(sockfd) == -1) {
        error("Close failed");
    }
    
    return 0;
}

void ctrlHandlerServer(int num) {
    close_server();
    close_fd();
    printf("\n");
    exit(EXIT_SUCCESS);
}

/*/////////////////////////////---------------------CLIENTS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

/*Cuando este leyendo, variable que sea true lectores, y cuando cambie a escritores que cambie para
saber cuando tengo que volver a leer del fichero*/

void set_ip_port (char* ip, unsigned int port) {
    bzero(&sock, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = inet_addr(ip);
    sock.sin_port = htons(port);
}

int create_socket(int id) {
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

void read_or_write(char* ip, int port, int threads, char* mode) {
    pthread_t clients[threads];

    if (strcmp(mode, "reader") == 0) {
        roll = 1;
    } else if (strcmp(mode, "writer") == 0) {
        roll = 0;
    }

    set_ip_port(ip, port);

    int id_thread[threads];
    for (int i = 0; i < threads; i++) {
        id_thread[i] = i;
    }
    for (int i = 0; i < threads; i++) {
        pthread_create(&clients[id_thread[i]], NULL, &connect_client, (void *) &id_thread[i]);
    }

    for(int i = 0; i < threads; i++) {
        if(pthread_join(clients[i], NULL) == -1) {
            printf("Error pthread_join...\n");
            exit(EXIT_FAILURE);
        }
    }
}

void *connect_client(void *arg) {
    int t_id = *(int *)arg;
    msg_client.action = roll;
    msg_client.id = t_id;

    sock_cli[t_id] = create_socket(t_id);
    if((connect(sock_cli[t_id], (struct sockaddr*)&sock, sizeof(sock))) == -1) {
        error("Connection with the server failed...");
    }

    msg_client.action = roll;
    msg_client.id = t_id;

    //printf("[CLIENT #%d] Send %d\n", msg_client.id, msg_client.action);
    if (send(sock_cli[t_id], &msg_client, sizeof(msg_client), 0) == -1) {
        printf("Send to server failed...\n");
    }

    if ((recv(sock_cli[t_id], (void *) &msg_resp, sizeof(msg_resp), 0)) == -1) {
        error("Receive message failed...");
    } else {
        if (msg_resp.action == WRITE) {
            printf("[Cliente #%d] Escritor, contador = %d, tiempo = %ld ns. \n", t_id, msg_resp.counter, msg_resp.waiting_time);
        } else if (msg_resp.action == READ) {
            printf("[Cliente #%d] Lector, contador = %d, tiempo = %ld ns. \n", t_id, msg_resp.counter, msg_resp.waiting_time);
        } else {
            printf("Response action %d\n", msg_resp.action);
        }
    }

    return 0;
}

int close_client(int id){
    if(close(sock_cli[id]) == -1) {
        error("Close failed");
    }

    return 0;
}


void ctrlHandler(int num) {
    close_client(counter);
    printf("\n");
    exit(EXIT_SUCCESS);
}
