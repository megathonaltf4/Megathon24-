#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 256

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int client_sockets[2] = {0, 0};
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *client_sock) {
    int sock = *(int *)client_sock;
    char buffer[BUFFER_SIZE];
    int n, other_sock;

    pthread_mutex_lock(&clients_mutex);
    if (client_sockets[0] == 0) {
        client_sockets[0] = sock;
        other_sock = client_sockets[1];
    } else {
        client_sockets[1] = sock;
        other_sock = client_sockets[0];
    }
    pthread_mutex_unlock(&clients_mutex);

    printf("Client connected: Socket %d\n", sock);

    while ((n = read(sock, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[n] = '\0';
        printf("Received message from socket %d: %s\n", sock, buffer);

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Client on socket %d has exited\n", sock);
            break;
        }

        // Updated Section: Dynamically check and update the other client socket
        pthread_mutex_lock(&clients_mutex);
        other_sock = (sock == client_sockets[0]) ? client_sockets[1] : client_sockets[0];
        pthread_mutex_unlock(&clients_mutex);

        if (other_sock != 0) {
            if (write(other_sock, buffer, strlen(buffer)) < 0) {
                perror("ERROR writing to socket");
            }
            printf("Relayed message to socket %d\n", other_sock);
        } else {
            printf("No other client to relay the message to.\n");
        }

        bzero(buffer, BUFFER_SIZE);
    }

    pthread_mutex_lock(&clients_mutex);
    if (client_sockets[0] == sock) {
        client_sockets[0] = 0;
    } else {
        client_sockets[1] = 0;
    }
    pthread_mutex_unlock(&clients_mutex);

    printf("Client disconnected: Socket %d\n", sock);
    close(sock);
    pthread_exit(NULL);
}

int main() {
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t thread_id;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    printf("Server listening on port %d\n", PORT);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");

        printf("Accepted connection from client\n");

        if (pthread_create(&thread_id, NULL, handle_client, (void *)&newsockfd) < 0) {
            perror("ERROR creating thread");
            return 1;
        }
        pthread_detach(thread_id);
    }

    close(sockfd);
    return 0;
}
