#include "networking.h"
#include "game_logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>

#define MAX_GAMES 10
#define MAX_PLAYERS 20

const char* funny_names[] = {
    "Cheesy McMoves", "Pawn Star", "Knight Rider", "Bishop Blitz",
    "Rook Rookie", "Queen Bee", "King Kong", "Checkmate Charlie",
    "Castle Crasher", "En Passant Pete", "Stalemate Steve", "Gambit Gus",
    "Sicilian Sizzler", "French Fry", "Caro-Kann't Stop", "Alekhine's Gun"
};

typedef struct {
    int socket;
    char name[50];
} Player;

typedef struct {
    Player white;
    Player black;
    char game_state[256];
    pthread_t thread;
} Game;

Game games[MAX_GAMES];
Player waiting_players[MAX_PLAYERS];
int num_waiting_players = 0;
pthread_mutex_t players_mutex = PTHREAD_MUTEX_INITIALIZER;

void* handle_game(void* arg) {
    Game* game = (Game*)arg;
    initialize_game_state(game->game_state);

    // Send initial game state to both players
    char white_init[260], black_init[260];
    snprintf(white_init, sizeof(white_init), "%sW", game->game_state);
    snprintf(black_init, sizeof(black_init), "%sB", game->game_state);
    send(game->white.socket, white_init, strlen(white_init), 0);
    send(game->black.socket, black_init, strlen(black_init), 0);

    while (1) {
        char move[32];
        int current_player = (game->game_state[128] == 'W') ? game->white.socket : game->black.socket;
        int received = recv(current_player, move, sizeof(move), 0);
        
        if (received > 0) {
            printf("Received move: %s from %s\n", move, (current_player == game->white.socket) ? game->white.name : game->black.name);
            // the moves that you recieve are validd moves because they are checked in the client side, in the ui.c file 

            update_game_state(game->game_state, move);
            printf("Updated game state: %s\n", game->game_state);
            
            // Send updated game state to both players
            send(game->white.socket, game->game_state, strlen(game->game_state), 0);
            send(game->black.socket, game->game_state, strlen(game->game_state), 0);
        }
    }

    close(game->white.socket);
    close(game->black.socket);
    return NULL;
}

void start_new_game(Player* player1, Player* player2) {
    for (int i = 0; i < MAX_GAMES; i++) {
        if (games[i].white.socket == 0) {
            games[i].white = *player1;
            games[i].black = *player2;
            pthread_create(&games[i].thread, NULL, handle_game, &games[i]);
            printf("Starting new game between %s and %s\n", player1->name, player2->name);
            return;
        }
    }
    printf("Error: No free game slots\n");
}

int main() {
    int server_socket = start_server(8080);
    if (server_socket == -1) {
        printf("Failed to start the server.\n");
        return 1;
    }

    while (1) {
        int client_socket = wait_for_connection(server_socket);
        if (client_socket != -1) {
            Player new_player;
            new_player.socket = client_socket;
            strcpy(new_player.name, funny_names[rand() % (sizeof(funny_names) / sizeof(funny_names[0]))]);
            
            pthread_mutex_lock(&players_mutex);
            if (num_waiting_players > 0) {
                Player* opponent = &waiting_players[--num_waiting_players];
                pthread_mutex_unlock(&players_mutex);
                start_new_game(opponent, &new_player);
            } else {
                waiting_players[num_waiting_players++] = new_player;
                pthread_mutex_unlock(&players_mutex);
                printf("Player %s is waiting for an opponent\n", new_player.name);
            }
        }
    }

    return 0;
}