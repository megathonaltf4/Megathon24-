#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 256

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void *receive_messages(void *sockfd) {
    int sock = *(int *)sockfd;
    char buffer[BUFFER_SIZE];
    int n;

    while ((n = read(sock, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[n] = '\0';
        printf("Received: %s\n", buffer);
        bzero(buffer, BUFFER_SIZE);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    pthread_t thread_id;
    char buffer[BUFFER_SIZE];

    if (argc < 2) {
       fprintf(stderr, "usage %s hostname\n", argv[0]);
       exit(0);
    }
    portno = PORT;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    printf("Connected to server\n");

    if (pthread_create(&thread_id, NULL, receive_messages, (void *)&sockfd) < 0) {
        perror("ERROR creating thread");
        return 1;
    }

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE - 1, stdin);
        
        if (strncmp(buffer, "exit", 4) == 0) {
            break; // Exit the loop
        }
        
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) error("ERROR writing to socket");
    }

    close(sockfd);
    printf("Disconnected from server\n");
    return 0;
}
