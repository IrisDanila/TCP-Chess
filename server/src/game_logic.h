#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

void handle_game_session(int player1, int player2);
void initialize_game_state(char* game_state);
void update_game_state(char* game_state, const char* move);

#endif // GAME_LOGIC_H