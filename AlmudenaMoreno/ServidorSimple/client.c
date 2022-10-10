/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 1. Ejercicio 1. Cliente Simple
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX 256
#define PORT 8080

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    int tcp_socket = 0, r = 0;

    char buff[MAX];
    explicit_bzero(buff, MAX);  //Erase data if necessary 

    /*Create IP direction and port*/
    struct sockaddr_in servaddr;
    explicit_bzero(&servaddr, sizeof(servaddr));  //Erase data if necessary 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   //Any interface
    servaddr.sin_port = htons(PORT); 

    /*Create a socket and test*/
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1) {
        printf("Socket creation failed...\n");
        exit(1);
    } else {
        printf("Socket successfully created...\n");
    }

    /*Server is waiting for clients*/
    socklen_t len = sizeof(servaddr);
    if (connect(tcp_socket, (struct sockaddr *)&servaddr, len) == -1) {
        printf("Server connection failed...\n");
        exit(1);
    } else { 
        printf("connected to the server...\n");
    }

    /*Send message to server*/
    printf("> ");
    char message[MAX];
    fgets(message, MAX, stdin);
    send(tcp_socket, message, strlen(message), 0);

    /*Receive data from a socket*/
    r = recv(tcp_socket, (void*) buff, sizeof(buff), 0);
    if (r > 0) {
        printf("+++ ");
        if (fputs(buff, stdout) == EOF) {
            fprintf(stderr, "[ERROR]: fputs() failed...");
        }
    }

    /*Socket closed*/
    if (close(tcp_socket) < 0) {
        printf("Client not correctly closed...\n");
        exit(1);
    }

    return 0;
}