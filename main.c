#include "raylib.h"
#include <math.h> //fminf(): devolve o menor numero entre dois numeros decimais

#define NUM_QUADROS 22 //quantidade de quadros da animação do macaco
#define TAM_QUADRO 128 //largura e altura de cada quadro na sprite (pixels)
#define LARGURA 800 //largura do 'mundo'
#define ALTURA 600 //altura do 'mundo'

//telas, fiz em struct
typedef enum {
    TELA_MENU,
    TELA_JOGO
} Tela;

int main (void){
    //configurações da janela
    SetConfigFlags(FLAG_WINDOW_RESIZABLE); //permite que a janela diminua e aumente
    InitWindow(LARGURA, ALTURA, "Ape Invaders"); //cria a janela inicial
    SetTargetFPS(60);
    SetWindowMinSize(400, 300); //tamanho minimo

    RenderTexture2D alvo = LoadRenderTexture(LARGURA,ALTURA);
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_POINT);

    Texture2D sheet = LoadTexture("macaco_sheet.png");
    Texture2D logo = LoadTexture("logo.png");
    GenTextureMipmaps(&logo);
    SetTextureFilter(logo, TEXTURE_FILTER_TRILINEAR);

    Tela tela = TELA_MENU;

    float x = 400, y = 480;
    float velocidade = 5;
    int quadro = 0;
    float tempo = 0;
    float duracao = 1.0f / 12.0f;

while (!WindowShouldClose()) {
    //movimentação do macaco
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        if (x > 0) x -= velocidade;
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        if (x < 800 - TAM_QUADRO) x += velocidade;
    }

    //animação do macaco
    tempo += GetFrameTime();
    if (tempo >= duracao) {
        tempo -= duracao;
        quadro = (quadro + 1) % NUM_QUADROS;
    }

    if (tela == TELA_MENU) {
        if (IsKeyPressed(KEY_ENTER)) {
            x = 400; //macaco começa no meio da tela
            tela = TELA_JOGO;
        }
    }

    else if (tela == TELA_JOGO) {
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {if (x > 0) x -= velocidade;}
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {if (x < 800 - TAM_QUADRO) x+= velocidade;}

        if (IsKeyPressed(KEY_M)) tela = TELA_MENU;
    }

    Rectangle origem = { quadro * TAM_QUADRO, 0, TAM_QUADRO, TAM_QUADRO };

    BeginTextureMode(alvo);
        ClearBackground(BLACK);

        if (tela == TELA_MENU) {
            //escala do logo, para deixar centralizado.
            float escalaLogo = fminf(560.0f / logo.width, 240.0f / logo.height);
            float larguraLogo = logo.width * escalaLogo;

            DrawTextureEx(logo, (Vector2){ (LARGURA - larguraLogo)/2,80}, 0.0f, escalaLogo, WHITE);

            if ((int)(GetTime() * 2) % 2 == 0) {
                const char *msg = "PRESSIONE ENTER PARA JOGAR";
                DrawText(msg, (LARGURA - MeasureText(msg, 24)) / 2, 440, 24, WHITE);
            }
            DrawText("ESC PARA SAIR", 10, ALTURA - 30, 20, GRAY);
        }

        else if (tela == TELA_JOGO) {
            DrawTextureRec(sheet, origem, (Vector2){x,y}, WHITE);
            DrawText("M PARA MENU", 10, 10, 20, GRAY);
        }
    EndTextureMode();
        //escala da textura interna, é para ficar centralizado
    float escala = fminf((float)GetScreenWidth() / LARGURA,
                        (float)GetScreenHeight() / ALTURA);
    float w = LARGURA * escala;
    float h = ALTURA * escala;
    Rectangle destino = { (GetScreenWidth() - w) / 2, (GetScreenHeight() - h) / 2, w, h };

    BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(alvo.texture,
                    (Rectangle){ 0, 0, LARGURA, -ALTURA },
                    destino, (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}
    //limpa a memoria
    UnloadTexture(sheet);
    UnloadTexture(logo);
    UnloadRenderTexture(alvo);
    CloseWindow();
    return 0;
}