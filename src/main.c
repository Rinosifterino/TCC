#include "raylib.h"

int main(void)
{
    // Inicializa a janela
    InitWindow(800, 450, "Motor 3D - TCC");
    SetTargetFPS(60);

    // Loop principal
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Raylib compilado com sucesso!", 190, 200, 20, DARKGRAY);
        EndDrawing();
    }

    // Fecha a janela
    CloseWindow();
    return 0;
}