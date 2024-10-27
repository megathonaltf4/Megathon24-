#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <SDL2/SDL.h>

#define PORT 8080
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

typedef struct {
    int x;
    int y;
    int color; // 0 for red, 1 for blue
} GameState;

GameState game_states[2];

void *receive_game_state(void *sock) {
    int server_sock = *(int *)sock;
    char buffer[1024];

    while (1) {
        int read_size = read(server_sock, buffer, 1024);
        if (read_size > 0) {
            buffer[read_size] = '\0';
            sscanf(buffer, "%d,%d,%d;%d,%d,%d",
                   &game_states[0].x, &game_states[0].y, &game_states[0].color,
                   &game_states[1].x, &game_states[1].y, &game_states[1].color);
            printf("Game state received: %d, %d, %d; %d, %d, %d\n",
                   game_states[0].x, game_states[0].y, game_states[0].color,
                   game_states[1].x, game_states[1].y, game_states[1].color);
        } else if (read_size == 0) {
            break;
        } else {
            perror("recv failed");
            break;
        }
    }

    pthread_exit(NULL);
}

void render_game_state(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < 2; i++) {
        if (game_states[i].color == 0) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue
        }

        SDL_Rect rect = {game_states[i].x % SCREEN_WIDTH, game_states[i].y % SCREEN_HEIGHT, 50, 50};
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    pthread_t tid;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {  // Replace with server's IP address
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    pthread_create(&tid, NULL, &receive_game_state, (void *)&sock);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Game Client", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return -1;
    }

    int quit = 0;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_KEYDOWN) {
                char input[1024];
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        strcpy(input, "left");
                        break;
                    case SDLK_RIGHT:
                        strcpy(input, "right");
                        break;
                    case SDLK_UP:
                        strcpy(input, "up");
                        break;
                    case SDLK_DOWN:
                        strcpy(input, "down");
                        break;
                    default:
                        continue;
                }
                send(sock, input, strlen(input), 0);
            }
        }

        render_game_state(renderer);
        SDL_Delay(16); // 60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
