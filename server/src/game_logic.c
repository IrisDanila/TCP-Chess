#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <ctype.h>
#include <unistd.h>

void initialize_game_state(char* game_state) {
    // Initial chess board setup (lowercase for white, uppercase for black)
    const char* initial_setup[8] = {
        "RNBQKBNR",  // Black back row
        "PPPPPPPP",  // Black pawns
        "00000000",  // Empty rows
        "00000000",
        "00000000",
        "00000000",
        "pppppppp",  // White pawns
        "rnbqkbnr"   // White back row
    };
    
    int pos = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            char piece = initial_setup[y][x];
            game_state[pos++] = piece;
            // Determine color: uppercase = black, lowercase = white
            game_state[pos++] = (piece >= 'a' && piece <= 'z') ? 'W' : 
                                (piece == '0' ? 'E' : 'B');
        }
    }
    
    // Add turn information at the end (W for White's turn)
    game_state[pos++] = 'W';
    game_state[pos] = '\0';
}

void update_game_state(char* game_state, const char* move) 
{
    // Parse move string
    int from_x, from_y, to_x, to_y;
    sscanf(move, "%d,%d,%d,%d", &from_x, &from_y, &to_x, &to_y);
    
    // Calculate positions in the game_state string
    int from_pos = (from_y * 8 + from_x) * 2;
    int to_pos = (to_y * 8 + to_x) * 2;
    
    // Move the piece
    game_state[to_pos] = game_state[from_pos];        // Piece type
    game_state[to_pos + 1] = game_state[from_pos + 1]; // Piece color
    
    // Clear the original position
    game_state[from_pos] = '0';     // Empty square
    game_state[from_pos + 1] = 'E'; // Empty color (can be any character)
    
    // Toggle turn
    int turn_pos = 8 * 8 * 2; // Position of turn indicator
    game_state[turn_pos] = (game_state[turn_pos] == 'W') ? 'B' : 'W';
}

void handle_game_session(int player1, int player2) {
    char game_state[256] = {0};
    int current_player = player1;
    int waiting_player = player2;

    initialize_game_state(game_state);
    printf("Initial game state: %s\n", game_state);
    
    // Send initial game state to both players with their color assignment
    char player1_init[260], player2_init[260];
    snprintf(player1_init, sizeof(player1_init), "%sW", game_state); // W for White
    snprintf(player2_init, sizeof(player2_init), "%sB", game_state); // B for Black
    
    send(player1, player1_init, strlen(player1_init), 0);
    send(player2, player2_init, strlen(player2_init), 0);

    while (1) {
        char move[32] = {0};
        int received = recv(current_player, move, sizeof(move), 0);
        // the moves that you recieve are validd moves because they are checked in the client side, in the ui.c file 
        if (received > 0) { 
            printf("Received move: %s from player %d\n", 
            move, current_player == player1 ? 1 : 2);
        
            update_game_state(game_state, move);
            printf("Updated game state: %s\n", game_state);
            
            // Send updated game state to both players
            send(player1, game_state, strlen(game_state), 0);
            send(player2, game_state, strlen(game_state), 0);
            
            // Switch turns
            int temp = current_player;
            current_player = waiting_player;
            waiting_player = temp;
        }
    }
}





