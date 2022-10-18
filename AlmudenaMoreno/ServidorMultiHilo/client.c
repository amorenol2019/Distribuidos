/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 1. Ejercicio 3. Cliente MultiHilo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX 1024

int client_socket = 0;

void error(char *msg) {
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

void ctrlHandler(int num) {
    close(client_socket);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

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

    if(argc != 4) {
        error("Incorrect program call.\n Usage: ./client ID_CLIENT IP_DIRECTION PORT");
    }
    int ID_CLIENT = atoi(argv[1]);
    char *ip_dir = argv[2];
    int port = atoi(argv[3]);

    signal(SIGINT, ctrlHandler); //Close with CTRL + C

    /*Create a socket and test*/
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        error("Socket creation failed...");
    } else {
        printf("Socket successfully created...\n");
    }

    /*Create IP direction and port*/
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = inet_addr(ip_dir);
    sock_serv.sin_port = htons(port); 

    /*Server is waiting for clients*/
    len = sizeof(sock_serv);
    if (connect(client_socket, (struct sockaddr *)&sock_serv, len) == -1) {
        close(client_socket);
        error("Server connection failed...");
    } else { 
        printf("connected to the server...\n");
    }

    /*Send message to server*/
    snprintf(message, MAX, "Hello server! From client: %d\n" , ID_CLIENT);
    send(client_socket, message, strlen(message), 0);

    //*Select: monitors file descriptos until readyy to I/O operations*/
    FD_ZERO(&readmask); // Reset mask
    FD_SET(connfd, &readmask); // Assign new descriptor
    FD_SET(STDIN_FILENO, &readmask); // Entry
    timeout.tv_sec = 2; timeout.tv_usec = 500000; // Timeout de 2.5 seg.
    
    if (select(connfd + 1, &readmask, NULL, NULL, &timeout) == -1) {
        close(client_socket);
        error("Failed in select...");
    }

    /*Receive data from a socket*/
    r = recv(client_socket, (void*) buff, sizeof(buff), 0);
    if (r > 0) {
        printf("+++ ");
        if (fputs(buff, stdout) == EOF) {
            close(client_socket);
            fprintf(stderr, "[ERROR]: fputs() failed...");
        }
    } else if (r == -1) {
        close(client_socket);
        error("Receive data from server failed...");
    }

    return 0;
}