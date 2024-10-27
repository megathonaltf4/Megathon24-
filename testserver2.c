#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 2

typedef struct {
    int x;
    int y;
    int color; // 0 for red, 1 for blue
} GameState;

typedef struct {
    int sock;
    struct sockaddr_in address;
    int addr_len;
    int player_id;
} client_t;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
GameState game_states[MAX_CLIENTS];

void *handle_client(void *arg) {
    char buffer[1024];
    client_t *cli = (client_t *)arg;

    while (1) {
        int read_size = read(cli->sock, buffer, 1024);
        if (read_size > 0) {
            buffer[read_size] = '\0';
            printf("Received action: %s\n", buffer);

            pthread_mutex_lock(&clients_mutex);
            if (strcmp(buffer, "left") == 0) {
                game_states[cli->player_id].x -= 10;
            } else if (strcmp(buffer, "right") == 0) {
                game_states[cli->player_id].x += 10;
            } else if (strcmp(buffer, "up") == 0) {
                game_states[cli->player_id].y -= 10;
            } else if (strcmp(buffer, "down") == 0) {
                game_states[cli->player_id].y += 10;
            }
            pthread_mutex_unlock(&clients_mutex);

            snprintf(buffer, sizeof(buffer), "%d,%d,%d;%d,%d,%d", 
                     game_states[0].x, game_states[0].y, game_states[0].color,
                     game_states[1].x, game_states[1].y, game_states[1].color);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i]) {
                    send(clients[i]->sock, buffer, strlen(buffer), 0);
                    printf("Game state sent: %s\n", buffer);
                }
            }
        } else if (read_size == 0) {
            break;
        } else {
            perror("recv failed");
            break;
        }
    }

    close(cli->sock);
    free(cli);
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        game_states[i].x = 100 * i;
        game_states[i].y = 100 * i;
        game_states[i].color = i; // 0 for red, 1 for blue
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    int player_id = 0;
    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) >= 0) {
        printf("New client connected\n");

        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->sock = new_socket;
        cli->address = address;
        cli->addr_len = addrlen;
        cli->player_id = player_id++;

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == NULL) {
                clients[i] = cli;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        pthread_t tid;
        pthread_create(&tid, NULL, &handle_client, (void *)cli);
    }

    return 0;
}
