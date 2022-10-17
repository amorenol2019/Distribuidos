/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 1. Ejercicio 3. Cliente Multi Hilo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX 1024
#define FAIL 1

int client_socket = 0;

void error(char *msg) {
    printf("%s", msg);
    close(client_socket);
    exit(FAIL);
}

void ctrlHandler(int num) {
    close(client_socket);
    exit(0);
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    //Numero de argumentos incluyendo el nombre del programa
    if(argc != 4) {
        printf("Incorrect program call.\n Usage: ./client IP_DIRECTION PORT");
        exit(1);
    }
    int ID_CLIENT = atoi(argv[1]);
    char *ip_dir = argv[2];
    int port = atoi(argv[3]);

    int connfd = 0, r = 0;
    struct sockaddr_in sock_serv;
    bzero(&sock_serv, sizeof(sock_serv));  //Erase data
    socklen_t len;
    char buff[MAX];
    bzero(buff, MAX);
    char message[MAX];
    bzero(message, MAX);
    fd_set readmask;
    struct timeval timeout;

    signal(SIGINT, ctrlHandler);   //Close with CTRL + C

    /*Create a socket and test*/
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        error("Socket creation failed...\n");
    } else {
        printf("Socket successfully created...\n");
    }

    /*Create IP direction and port*/
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = inet_addr(ip_dir); //"212.128.254.23");   //Any interface
    sock_serv.sin_port = htons(port); 

    /*Server is waiting for clients*/
    len = sizeof(sock_serv);
    if (connect(client_socket, (struct sockaddr *)&sock_serv, len) == -1) {
        error("Server connection failed...\n");
    } else { 
        printf("connected to the server...\n");
    }

    /*Send message to server*/
    printf("> ");
    snprintf(message, MAX, "Hello server! From client: %d\n" , ID_CLIENT);
    printf("%s", message);
    send(client_socket, message, strlen(message), 0);

    //*Select: monitors file descriptos until readyy to I/O operations*/
    FD_ZERO(&readmask); // Reset la mascara
    FD_SET(connfd, &readmask); // Asignamos el nuevo descriptor
    FD_SET(STDIN_FILENO, &readmask); // Entrada
    //NULL Timeout - undefined waiting time
    timeout.tv_sec = 3; timeout.tv_usec = 500000; // Timeout de 3 seg.
    
    if (select(connfd + 1, &readmask, NULL, NULL, &timeout) == -1) {
        exit(EXIT_FAILURE);
    }


    /*Receive data from a socket*/
    r = recv(client_socket, (void*) buff, sizeof(buff), 0);
    if (r > 0) {
        printf("+++ ");
        if (fputs(buff, stdout) == EOF) {
            fprintf(stderr, "[ERROR]: fputs() failed...");
        }
    } else if (r == -1) {
        error("Receive data from server failed...\n");
    }

    ctrlHandler(0);

    return 0;
}