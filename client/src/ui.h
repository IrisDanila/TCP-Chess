#ifndef __UI_H__
#define __UI_H__

#include <raylib.h>

typedef struct Position Position;
typedef struct Piece Piece;
typedef struct Square Square;
typedef struct PossibleMoves PossibleMoves;

void run_ui(int socket);
void deserialize_board(char* game_state);
void run_uii(int socket);

#endif // __UI_H__