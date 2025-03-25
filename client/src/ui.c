#include "raylib.h"
#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <raylib.h>
#include <sys/select.h>
#include <ctype.h>

#define BOARD_SIZE 8

typedef struct {
    int x, y;
} Position;

typedef struct {
    char type;   // 'P' for pawn, 'R' for rook, etc.
    Color color; // WHITE or BLACK
    Texture2D texture;
    Position position;
} Piece;

Piece board[BOARD_SIZE][BOARD_SIZE]; // 8x8 chessboard
int turn = 0;








#define MAX_POSSIBLE_MOVES 27  // Maximum possible moves for any piece

typedef struct {
    int x, y;
} Square;

typedef struct {
    Square squares[MAX_POSSIBLE_MOVES];
    int count;
} PossibleMoves;

Color highlightColor = (Color){76, 175, 80, 100};  // Light green with alpha

PossibleMoves get_possible_moves(int x, int y) {
    PossibleMoves moves = {0};
    if (x < 0 || x >= 8 || y < 0 || y >= 8 || board[y][x].type == 0) {
        return moves;
    }

    char piece = tolower(board[y][x].type);
    Color piece_color = board[y][x].color;

    switch (piece) {
        case 'p': {
            int direction = (piece_color.r == WHITE.r) ? -1 : 1;
            
            // Forward move
            if (y + direction >= 0 && y + direction < 8 && 
                board[y + direction][x].type == 0) {
                moves.squares[moves.count++] = (Square){x, y + direction};
                
                // Initial two-square move
                if (((piece_color.r == WHITE.r && y == 6) || 
                    (piece_color.r == BLACK.r && y == 1)) &&
                    board[y + 2 * direction][x].type == 0) {
                    moves.squares[moves.count++] = (Square){x, y + 2 * direction};
                }
            }
            
            // Captures
            for (int dx = -1; dx <= 1; dx += 2) {
                if (x + dx >= 0 && x + dx < 8 && y + direction >= 0 && y + direction < 8) {
                    if (board[y + direction][x + dx].type != 0 && 
                        board[y + direction][x + dx].color.r != piece_color.r) {
                        moves.squares[moves.count++] = (Square){x + dx, y + direction};
                    }
                }
            }
            break;
        }
        case 'r': {
            // Rook moves
            const int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
            for (int d = 0; d < 4; d++) {
                int nx = x, ny = y;
                while (1) {
                    nx += directions[d][0];
                    ny += directions[d][1];
                    if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8) break;
                    if (board[ny][nx].type != 0) {
                        if (board[ny][nx].color.r != piece_color.r) {
                            moves.squares[moves.count++] = (Square){nx, ny};
                        }
                        break;
                    }
                    moves.squares[moves.count++] = (Square){nx, ny};
                }
            }
            break;
        }
        case 'n': {
            // Knight moves
            const int knight_moves[8][2] = {
                {-2, -1}, {-2, 1}, {2, -1}, {2, 1},
                {-1, -2}, {1, -2}, {-1, 2}, {1, 2}
            };
            for (int i = 0; i < 8; i++) {
                int nx = x + knight_moves[i][0];
                int ny = y + knight_moves[i][1];
                if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8 &&
                    (board[ny][nx].type == 0 || 
                     board[ny][nx].color.r != piece_color.r)) {
                    moves.squares[moves.count++] = (Square){nx, ny};
                }
            }
            break;
        }
        case 'b': {
            // Bishop moves
            const int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
            for (int d = 0; d < 4; d++) {
                int nx = x, ny = y;
                while (1) {
                    nx += directions[d][0];
                    ny += directions[d][1];
                    if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8) break;
                    if (board[ny][nx].type != 0) {
                        if (board[ny][nx].color.r != piece_color.r) {
                            moves.squares[moves.count++] = (Square){nx, ny};
                        }
                        break;
                    }
                    moves.squares[moves.count++] = (Square){nx, ny};
                }
            }
            break;
        }
        case 'q': {
            // Queen moves (combination of rook and bishop)
            const int directions[8][2] = {
                {0, 1}, {1, 0}, {0, -1}, {-1, 0},
                {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
            };
            for (int d = 0; d < 8; d++) {
                int nx = x, ny = y;
                while (1) {
                    nx += directions[d][0];
                    ny += directions[d][1];
                    if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8) break;
                    if (board[ny][nx].type != 0) {
                        if (board[ny][nx].color.r != piece_color.r) {
                            moves.squares[moves.count++] = (Square){nx, ny};
                        }
                        break;
                    }
                    moves.squares[moves.count++] = (Square){nx, ny};
                }
            }
            break;
        }
        case 'k': {
            // King moves
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8 &&
                        (board[ny][nx].type == 0 || 
                         board[ny][nx].color.r != piece_color.r)) {
                        moves.squares[moves.count++] = (Square){nx, ny};
                    }
                }
            }
            break;
        }
    }
    return moves;
}

Sound moveSound;
Sound captureSound;
Sound castleSound;

// New function to check if a king is in check
bool is_king_in_check(Color king_color) {
    // Find the king's position
    int king_x = -1, king_y = -1;
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if ((board[y][x].type == 'K' || board[y][x].type=='k') && ColorToInt(board[y][x].color) == ColorToInt(king_color)) {
                king_x = x;
                king_y = y;
                break;
            }
        }
        if (king_x != -1) break;
    }

    // Check if any opponent's piece can attack the king
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (board[y][x].type != 0 && ColorToInt(board[y][x].color) != ColorToInt(king_color)) {
                PossibleMoves moves = get_possible_moves(x, y);
                for (int i = 0; i < moves.count; i++) {
                    if (moves.squares[i].x == king_x && moves.squares[i].y == king_y) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


bool is_checkmate(Color king_color) {
    // First, check if the king is in check
    if (!is_king_in_check(king_color)) {
        return false;
    }

    // Try all possible moves for all pieces of the current player
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (board[y][x].type != 0 && ColorToInt(board[y][x].color) == ColorToInt(king_color)) {
                PossibleMoves moves = get_possible_moves(x, y);
                for (int i = 0; i < moves.count; i++) {
                    // Store the original state
                    Piece original_from = board[y][x];
                    Piece original_to = board[moves.squares[i].y][moves.squares[i].x];

                    // Make the move
                    board[moves.squares[i].y][moves.squares[i].x] = board[y][x];
                    board[y][x].type = 0;

                    // Check if the king is still in check after this move
                    bool still_in_check = is_king_in_check(king_color);

                    // Undo the move
                    board[y][x] = original_from;
                    board[moves.squares[i].y][moves.squares[i].x] = original_to;

                    // If any move gets the king out of check, it's not checkmate
                    if (!still_in_check) {
                        return false;
                    }
                }
            }
        }
    }

    // If we've tried all moves and none get the king out of check, it's checkmate
    return true;
}


// Modified move function to handle check and game over conditions
bool move_piece(int from_x, int from_y, int to_x, int to_y) {
    // Store the original state
    Piece original_from = board[from_y][from_x];
    Piece original_to = board[to_y][to_x];

    // Make the move
    board[to_y][to_x] = board[from_y][from_x];
    board[from_y][from_x].type = 0;

    // Check if the move puts the current player's king in check
    Color current_color = (turn == 0) ? WHITE : BLACK;
    if (is_king_in_check(current_color)) {
        // Undo the move
        board[from_y][from_x] = original_from;
        board[to_y][to_x] = original_to;
        return false;
    }

    // Play appropriate sound
    if (original_to.type != 0) {
        PlaySound(captureSound);
    } else {
        PlaySound(moveSound);
    }

    // Check if opponent is in check
    Color opponent_color = (turn == 0) ? BLACK : WHITE;
    if (is_king_in_check(opponent_color)) {
        printf("%s checked %s\n", (turn == 0) ? "White" : "Black", (turn == 0) ? "Black" : "White");
        // TODO: Implement logic to restrict opponent's moves to only those that get out of check
    }

    turn = 1 - turn; // Switch turns
    return true;
}

// Function to deserialize the game_state string back into the board state
void deserialize_board(const char* game_state) {
    printf("Deserializing game state: %s\n", game_state);
    
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            int pos = (y * 8 + x) * 2;
            char piece_type = game_state[pos];
            char piece_color = game_state[pos + 1];
            
            // Clear the current square
            board[y][x].type = 0;
            board[y][x].color = WHITE;
            
            // If it's not an empty square, set the piece
            if (piece_type != '0') {
                board[y][x].type = piece_type;
                board[y][x].color = (piece_color == 'W') ? WHITE : BLACK;
                
                // Load appropriate texture
                char texture_path[64];
                snprintf(texture_path, sizeof(texture_path), "assets/pieces/%c%c.png",
                         piece_color == 'W' ? 'w' : 'b',
                         tolower(piece_type));
                board[y][x].texture = LoadTexture(texture_path);
            }
        }
    }
    
    // Update turn
    turn = (game_state[128] == 'W') ? 0 : 1;
    printf("Turn updated to: %d\n", turn);
}


// Load all pieces into the initial board positions
void load_pieces() {
    // First, clear the board
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            board[y][x].type = 0;
            board[y][x].color = WHITE;
            board[y][x].texture.id = 0;
        }
    }

    // Load textures once and store them
    Texture2D textures[12];  // 6 piece types * 2 colors
    const char* pieces = "PRNBQK";
    const char* colors = "wb";
    int texture_index = 0;

    for (int c = 0; c < 2; c++) {
        for (int p = 0; p < 6; p++) {
            char path[64];
            snprintf(path, sizeof(path), "assets/pieces/%c%c.png", 
                     colors[c], tolower(pieces[p]));
            textures[texture_index++] = LoadTexture(path);
        }
    }

    // Initialize the board with pieces
    // Pawns
    for (int i = 0; i < BOARD_SIZE; i++) {
        board[1][i].type = 'P';
        board[1][i].color = BLACK;
        board[1][i].texture = textures[0];  // black pawn texture

        board[6][i].type = 'P';
        board[6][i].color = WHITE;
        board[6][i].texture = textures[6];  // white pawn texture
    }

    // Other pieces
    const char* setup = "RNBQKBNR";
    for (int i = 0; i < BOARD_SIZE; i++) {
        char piece = setup[i];
        int texture_offset = strchr(pieces, piece) - pieces;
        
        // Black pieces
        board[0][i].type = piece;
        board[0][i].color = BLACK;
        board[0][i].texture = textures[texture_offset];

        // White pieces
        board[7][i].type = piece;
        board[7][i].color = WHITE;
        board[7][i].texture = textures[texture_offset + 6];
    }
}

// Drawing the board with pieces
void draw_board() {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            // Draw board square
            Color squareColor = ((x + y) % 2 == 0) ? RAYWHITE : GRAY;
            DrawRectangle(x * 100, y * 100, 100, 100, squareColor);

            // Draw piece if present
            if (board[y][x].type != 0 && board[y][x].texture.id != 0) {
                Vector2 position = {x * 100.0f, y * 100.0f};
                DrawTextureV(board[y][x].texture, position, WHITE);
            }
        }
    }
}

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
bool game_over = false;
const char* winner = NULL;
float restart_timer = 0.0f;

void run_ui(int socket) {
    load_pieces();
    int selected_x = -1, selected_y = -1;
    int player_color = -1;  // -1 = unassigned, 0 = white, 1 = black


    // Load sounds
    moveSound = LoadSound("assets/sounds/move.wav");
    captureSound = LoadSound("assets/sounds/capture.wav");
    castleSound = LoadSound("assets/sounds/castle.wav");
    // Receive initial game state and player color
    char initial_state[260];  // Increased size to accommodate color character
    if (recv(socket, initial_state, sizeof(initial_state), 0) > 0) {
        printf("Received initial state: %s\n", initial_state);
        
        // The last character of initial_state indicates this player's color
        char my_color = initial_state[strlen(initial_state) - 1];
        player_color = (my_color == 'W') ? 0 : 1;
        
        // Remove the color indicator before deserializing
        initial_state[strlen(initial_state) - 1] = '\0';
        deserialize_board(initial_state);
        
        printf("Assigned player color: %s\n", player_color == 0 ? "White" : "Black");
    }

    PossibleMoves possible_moves = {0};

    while (!WindowShouldClose()) {
        // Network update logic
        fd_set readfds;
        struct timeval tv = {0, 0};
        FD_ZERO(&readfds);
        FD_SET(socket, &readfds);
        
        if (select(socket + 1, &readfds, NULL, NULL, &tv) > 0) {
            char update[256];
            int received = recv(socket, update, sizeof(update), 0);
            if (received > 0) {
                update[received] = '\0';
                printf("Received update: %s\n", update);
                deserialize_board(update);
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if(!game_over) {

        draw_board();

        // Check if it's this player's turn
        bool is_my_turn = (turn == player_color);
        
        if (is_my_turn && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int x = GetMouseX() / 100;
            int y = GetMouseY() / 100;

            if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                if (selected_x == -1) {
                    // Selecting a piece - make sure it's the player's color
                    if (board[y][x].type != 0 && 
                        ((player_color == 0 && board[y][x].color.r == WHITE.r) ||
                         (player_color == 1 && board[y][x].color.r == BLACK.r))) {
                        selected_x = x;
                        selected_y = y;
                        possible_moves = get_possible_moves(selected_x, selected_y);
                        printf("Selected piece %c at %d,%d\n", board[y][x].type, x, y);
                    }
                } else {
                    // Attempting to move
                    bool is_valid_move = false;
                    for (int i = 0; i < possible_moves.count; i++) {
                        if (possible_moves.squares[i].x == x && 
                            possible_moves.squares[i].y == y) {
                            is_valid_move = true;
                            break;
                        }
                    }

                    if (is_valid_move) {
                        if (move_piece(selected_x, selected_y, x, y)) {
                            char move[32];
                            snprintf(move, sizeof(move), "%d,%d,%d,%d", 
                                    selected_x, selected_y, x, y);
                            printf("Sending valid move: %s\n", move);
                            
                            if (send(socket, move, strlen(move), 0) < 0) {
                                printf("Failed to send move\n");
                            } else {
                                printf("Move sent successfully\n");
                            }
                        } else {
                            printf("Invalid move: would put own king in check\n");
                        }
                    }
                    selected_x = selected_y = -1;
                    possible_moves.count = 0;
                }
            }
        }

        // Draw possible moves
        for (int i = 0; i < possible_moves.count; i++) {
            DrawRectangle(possible_moves.squares[i].x * 100, 
                         possible_moves.squares[i].y * 100,
                         100, 100, (Color){0, 255, 0, 100});  // Highlight color for possible moves
        }

        // Draw selection highlight
        if (selected_x != -1 && selected_y != -1) {
            DrawRectangle(selected_x * 100, selected_y * 100, 
                         100, 100, (Color){255, 255, 0, 100});
        }

        // Draw turn indicator and player color
        const char* turnText = is_my_turn ? "Your Turn" : "Opponent's Turn";
        const char* colorText = player_color == 0 ? "You are White" : "You are Black";
        DrawText(turnText, 10, SCREEN_HEIGHT - 50, 20, BLACK);
        DrawText(colorText, 10, SCREEN_HEIGHT - 25, 20, BLACK);

        // Unload sounds
        UnloadSound(moveSound);
        UnloadSound(captureSound);
        UnloadSound(castleSound);

        // Check for checkmate after each move
            if (is_checkmate(WHITE)) {
                game_over = true;
                winner = "Black";
                restart_timer = 5.0f;
            } else if (is_checkmate(BLACK)) {
                game_over = true;
                winner = "White";
                restart_timer = 5.0f;
            }
        } else {
            // Draw game over screen
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 200});
            char winnerText[50];
            snprintf(winnerText, sizeof(winnerText), "%s won!", winner);
            DrawText(winnerText, SCREEN_WIDTH/2 - MeasureText(winnerText, 40)/2, SCREEN_HEIGHT/2 - 50, 40, WHITE);

        }

        EndDrawing();
    }
     // Unload sounds
    UnloadSound(moveSound);
    UnloadSound(captureSound);
    UnloadSound(castleSound);
}


void print_board_state() {
    printf("Current board state:\n");
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            printf("%c ", board[y][x].type ? board[y][x].type : '.');
        }
        printf("\n");
    }
    printf("Current turn: %d\n", turn);
}
