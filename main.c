#include "raylib.h"

#define NUM_QUADROS 22
#define TAM_QUADRO 128

int main (void){
    InitWindow(800, 600, "Ape Invaders");
    SetTargetFPS(60);

    Texture2D sheet = LoadTexture("macaco_sheet.png");

    float x = 400, y = 480;
    float velocidade = 5;

    int quadro = 0;
    float tempo = 0;
    float duracao = 1.0f / 12.0f;

while (!WindowShouldClose()) {
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        if (x > 0) x -= velocidade;
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        if (x < 800 - TAM_QUADRO) x += velocidade;
    }

    tempo += GetFrameTime();
    if (tempo >= duracao) {
        tempo -= duracao;
        quadro = (quadro + 1) % NUM_QUADROS;
    }

    Rectangle origem = { quadro * TAM_QUADRO, 0, TAM_QUADRO, TAM_QUADRO };

    BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureRec(sheet, origem, (Vector2){ x, y }, WHITE);
    EndDrawing();
}

    UnloadTexture(sheet);
    CloseWindow();
    return 0;
}