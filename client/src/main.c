#include "raylib.h"
#include "network.h"
#include "ui.h"
#include <stdio.h>
#include <unistd.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

int main() {
    printf("Starting chess client...\n");
    
    int socket = connect_to_server("127.0.0.1", 8080);
    if (socket == -1) {
        printf("Failed to connect to server\n");
        return 1;
    }
    printf("Connected to server successfully\n");

    printf("Initializing window...\n");
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess Game");
    if (!IsWindowReady()) {
        printf("Failed to initialize window\n");
        close(socket);
        return 1;
    }
    printf("Window initialized successfully\n");

    SetTargetFPS(60);
    printf("Starting game loop...\n");
    
    run_ui(socket);
    
    printf("Game loop ended, cleaning up...\n");
    CloseWindow();
    close(socket);
    
    return 0;
}