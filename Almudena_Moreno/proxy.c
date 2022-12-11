/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 3. Proxy
*/

#include "proxy.h"

#define backlog 2000
#define MAX_THREADS 250
#define MAX_CLIENTS 1000
#define L_SIZE 1024

//SERVER VARIABLES
int n_readers = 0, ratio, n_writers = 0, serv_counter, priority, roll, r_wait = 0, w_wait = 0;
int ratio_reader = 0, ratio_writer = 0;

//CLIENT VARIABLES
int sock_cli[MAX_CLIENTS];
int total_threads = 0;

//OTHERS
int sockfd = 0;

char fline[L_SIZE];
FILE* file;

sem_t sem_max_threads;

pthread_mutex_t counter_mutex, mutex_prior, reader;
pthread_cond_t  reader_cond, writer_cond;

struct sockaddr_in sock;
struct sockaddr_in sock_serv;

/*////////////////////////////---------------------AUXILIAR FUNCTIONS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

void error(char *message) {
    printf("%s\n", message);
    close_fd();
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
    if (fprintf(file, "%d\n", serv_counter) < 0) {
        error("Error writing file");
    }
}

void read_fd() {
    if (fgets(fline, L_SIZE, file) == NULL) {
        serv_counter = 0;
    } else {
        serv_counter = atoi(fline);
    }
}

void sem_create() {
    sem_init(&sem_max_threads, 0, MAX_THREADS);
}

void sleep_random() {
    int sleep_number = rand () % 76 + 75;
    usleep(sleep_number * 1000);
}
/*////////////////////////////---------------------SERVER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void decide_priority(char *s_priority) {
    if (strcmp(s_priority, "reader") == 0) {
        priority = 0;
    } else {
        priority = 1;
    }
}

void set_server (unsigned int port, char *s_priority, int n_ratio) {
    srand(time(NULL));

    decide_priority(s_priority);
    ratio = n_ratio;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Socket creation failed...");
    }

    bzero(&sock_serv, sizeof(sock_serv));
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_serv.sin_port = htons(port);
}

void communicate_server(unsigned int port, char *s_priority, int n_ratio) {
    const int enable = 1;

    set_server(port, s_priority, n_ratio);

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
    while(1) {
        sem_wait(&sem_max_threads);
        pthread_t c_server;
        pthread_create(&c_server, NULL, &communicate_client, NULL);
    }
}

void *communicate_client(void *arg) {
    int connfd_;
    struct timeval wait_time_init;
    struct timeval wait_time_end;
    struct request msg_recv;
    struct response msg_serv;

    connfd_ = accept(sockfd,(struct sockaddr *)NULL, NULL);
    if (connfd_ < 0) {
        close(sockfd);
        close(connfd_);
        error("Failed server accept...");
    }
        
    if ((recv(connfd_, (void *) &msg_recv, sizeof(msg_recv), 0)) == -1) {
        error("Recv from the client failed...\n");
    }

    if (gettimeofday(&wait_time_init, 0) == -1) {
        error("Error getting stamp of time");
    }
    
    //WAITING TO ENTER TO CRITIC ZONE
    if (msg_recv.action == WRITE) {
        pthread_mutex_lock(&counter_mutex);
        w_wait++;  //Num of writers waiting
        pthread_mutex_unlock(&counter_mutex);

        pthread_mutex_lock(&mutex_prior);   //Will wait until there are no more readers or optional ratio
        if (priority == 0) {  //Priority readers
            if (ratio != -1) {   //OPTIONAL ARGUMENT
                while ((ratio_reader == 0) && (r_wait != 0 || n_readers != 0)){       //Will wait until there are no more readers
                    pthread_cond_wait(&writer_cond, &mutex_prior);
                } 
            } else {    //NOT OPTIONAL ARGUMENT RATIO
                while (r_wait != 0) {
                    pthread_cond_wait(&writer_cond, &mutex_prior);
                } 
            }
        }  else if (priority == 1) {  //Priority writers
            while (ratio != -1 && ratio_writer == 1 && r_wait != 0) {         //Will wait until there are no more readers
                pthread_cond_wait(&writer_cond, &mutex_prior);
            }
        }

        //Increase counter and edit file    
        if (gettimeofday(&wait_time_end, 0) == -1) {
            error("Error getting stamp of time");
        }
        
        printf("[%ld.%ld][ESCRITOR #%d] modifica contador con valor %d\n", wait_time_init.tv_sec, wait_time_init.tv_usec, msg_recv.id, ++serv_counter);
        msg_serv.counter = serv_counter;
        close_fd();
        open_fd("w");
        write_fd();
        sleep_random();

        w_wait--;
        n_writers++;

        if (ratio != -1 && (n_writers % ratio) == 0 && priority == 1 && r_wait != 0) {
            ratio_writer = 1;
            pthread_cond_signal(&reader_cond);
        }
        if ((ratio != -1 && ratio_reader == 1 && priority == 0) || (w_wait == 0)) {
            ratio_reader = 0;
            n_writers = 0;
            pthread_cond_broadcast(&reader_cond);
        } 

        pthread_mutex_unlock(&mutex_prior);

    } else if (msg_recv.action == READ) {
        //Counter readers waiting
        pthread_mutex_lock(&reader);
        r_wait++;  //Num of readers waiting
        pthread_mutex_unlock(&reader);

        if (priority == 1) {
            if (ratio == -1) {
                //printf("--------------W WAITING %d PRIOR %d RATIO %d\n", w_wait, priority, ratio);
                while (w_wait != 0){   //Will wait until there are no more writers
                    pthread_mutex_lock(&reader);
                    pthread_cond_wait(&reader_cond, &reader);
                    pthread_mutex_unlock(&reader);
                } 
            } else {
                while (ratio_writer == 0 && (w_wait != 0 || n_writers != 0)){    //OPTINAL ARGUMENT RATIO
                    pthread_mutex_lock(&reader);
                    pthread_cond_wait(&reader_cond, &reader);
                    pthread_mutex_unlock(&reader);
                }  
            }
        } else if (priority == 0){
            while (ratio != -1 && ratio_reader == 1 && w_wait != 0) {   //Will wait until there are no more writers
                pthread_mutex_lock(&reader);
                pthread_cond_wait(&reader_cond, &reader);
                pthread_mutex_unlock(&reader);
            }
        }

        //Increase counter
        if (gettimeofday(&wait_time_end, 0) == -1) {
            error("Error getting stamp of time");
        }

        printf("[%ld.%ld][LECTOR #%d] lee contador con valor %d\n", wait_time_init.tv_sec, wait_time_init.tv_usec, msg_recv.id, serv_counter);
        msg_serv.counter = serv_counter;
        sleep_random();

        pthread_mutex_lock(&reader);
        r_wait--;
        n_readers++;
        //printf("READERS S------------------------------------------------------------- %d    %d    %d\n", n_readers, (n_readers % ratio), r_wait);
        pthread_mutex_unlock(&reader);

        pthread_mutex_lock(&counter_mutex);
        if (ratio != -1 && (n_readers % ratio) == 0 && priority == 0 && w_wait != 0) {
            ratio_reader = 1;
            pthread_cond_signal(&writer_cond);
        }
        if ((ratio != -1 && ratio_writer == 1 && priority == 1) || (r_wait == 0)) {
            ratio_writer = 0;
            n_readers = 0;
            pthread_cond_broadcast(&writer_cond);
        }
        pthread_mutex_unlock(&counter_mutex);
        
    } else {
        error("Not action allowed");
    }

    msg_serv.action = msg_recv.action;
    long init_time = wait_time_init.tv_sec * 1e9 + wait_time_init.tv_usec * 1e3;
    long end_time = wait_time_end.tv_sec * 1e9 + wait_time_end.tv_usec * 1e3;
    msg_serv.waiting_time = (end_time - init_time);

    if (send(connfd_, &msg_serv, sizeof(msg_serv), 0) < 0) {
        error("Send to server failed...\n");
    }

    sem_post(&sem_max_threads);
    close_each_thread(connfd_);

    //Fin de región critica

    return 0;
}

void close_each_thread(int connfd_socket) {
    if (close(connfd_socket) == -1) {
        error("ERROR Closing socket client");
    }
    pthread_exit(0);
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

    pthread_mutex_destroy(&mutex_prior);
    pthread_mutex_destroy(&counter_mutex);
    pthread_mutex_destroy(&reader);

    pthread_cond_destroy(&reader_cond);
    pthread_cond_destroy(&writer_cond);
    //sem_destroy(&sem_max_threads);
    

    printf("\n");
    exit(EXIT_SUCCESS);
}

/*/////////////////////////////---------------------CLIENTS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_ip_port (char* ip, unsigned int port) {
    bzero(&sock, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = inet_addr(ip);
    sock.sin_port = htons(port);
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
    struct request msg_client;
    struct response msg_resp;

    int t_id = *(int *)arg;
    msg_client.action = roll;
    msg_client.id = t_id;

    sock_cli[t_id] = create_socket();
    if((connect(sock_cli[t_id], (struct sockaddr*)&sock, sizeof(sock))) == -1) {
        error("Connection with the server failed...");
    }

    msg_client.action = roll;
    msg_client.id = t_id;

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
    close_client(t_id);

    return 0;
}

int close_client(int id){
    close(sock_cli[id]);
    
    return 0;
}

void ctrlHandler(int num) {
    close_client(serv_counter);
    printf("\n");
    exit(EXIT_SUCCESS);
}
