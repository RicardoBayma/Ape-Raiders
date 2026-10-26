#include "raylib.h"
#include <math.h> //fminf(): devolve o menor numero entre dois numeros decimais

#define NUM_QUADROS 22 //quantidade de quadros da animação do macaco
#define TAM_QUADRO 128 //largura e altura de cada quadro na sprite (pixels)
#define LARGURA 1376 //largura do 'mundo'
#define ALTURA 768 //altura do 'mundo'
#define AREA_X 400
#define AREA_LARGURA 588

//telas, fiz em struct
typedef enum {
    TELA_MENU,
    TELA_JOGO,
    TELA_DERROTA,
    TELA_VITORIA
} Tela;

int main (void){
    //configurações da janela
    SetConfigFlags(FLAG_WINDOW_RESIZABLE); //permite que a janela diminua e aumente
    InitWindow(LARGURA, ALTURA, "Ape Raiders"); //cria a janela inicial
    SetTargetFPS(60);
    SetWindowMinSize(688, 384); //tamanho minimo

    RenderTexture2D alvo = LoadRenderTexture(LARGURA,ALTURA);
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_POINT);

    Texture2D sheet = LoadTexture("macaco_sheet.png");
    Texture2D logo = LoadTexture("logo.png");
    GenTextureMipmaps(&logo);
    SetTextureFilter(logo, TEXTURE_FILTER_TRILINEAR);

    Texture2D painelEsq = LoadTexture("painel_esquerdo.png");
    Texture2D praia = LoadTexture("praia.png");
    Texture2D painelDir = LoadTexture("painel_direito.png");

    Tela tela = TELA_MENU;

    float x = AREA_X + (AREA_LARGURA - TAM_QUADRO) / 2;
    float y = ALTURA - TAM_QUADRO - 10;
    float velocidade = 5;
    int quadro = 0;
    float tempo = 0;
    float duracao = 1.0f / 12.0f;

while (!WindowShouldClose()) {

    //animação do macaco
    tempo += GetFrameTime();
    if (tempo >= duracao) {
        tempo -= duracao;
        quadro = (quadro + 1) % NUM_QUADROS;
    }

    if (tela == TELA_MENU) {
        if (IsKeyPressed(KEY_ENTER)) {
            x = AREA_X + (AREA_LARGURA - TAM_QUADRO) / 2; //macaco começa no meio da tela
            tela = TELA_JOGO;
        }
    }

    else if (tela == TELA_JOGO) {
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {if (x > AREA_X) x -= velocidade;}
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {if (x < AREA_X + AREA_LARGURA - TAM_QUADRO) x+= velocidade;}

        if (IsKeyPressed(KEY_M)) tela = TELA_MENU;
        
        if (IsKeyPressed(KEY_V)) tela = TELA_VITORIA;
        if (IsKeyPressed(KEY_G)) tela = TELA_DERROTA;
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
            DrawTexture(painelEsq, 0, 0, WHITE);
            DrawTexture(praia, AREA_X, 0, WHITE);
            DrawTexture(painelDir, AREA_X + AREA_LARGURA, 0, WHITE);
            DrawTextureRec(sheet, origem, (Vector2){x,y}, WHITE);
            DrawText("M PARA MENU", 110, 678, 24, WHITE);
        }

        else if (tela == TELA_DERROTA) {
            const char *msg = "VOCÊ PERDEU";
            DrawText(msg, (LARGURA - MeasureText(msg,40))/2, 240, 40, RED);
            DrawText("ENTER PARA VOLTAR AO MENU", (LARGURA - MeasureText("ENTER PARA VOLTAR AO MENU", 20)) / 2, 320, 20, GRAY);
        }

        else if (tela == TELA_VITORIA) {
            const char *msg = "VOCE VENCEU";
            DrawText(msg, (LARGURA - MeasureText(msg,40))/2, 240, 40, GREEN);
            DrawText("ENTER PARA VOLTAR AO MENU", (LARGURA - MeasureText("ENTER PARA VOLTAR AO MENU", 20)) / 2, 320, 20, GRAY);
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
    UnloadTexture(painelEsq);
    UnloadTexture(praia);
    UnloadTexture(painelDir);
    UnloadRenderTexture(alvo);
    CloseWindow();
    return 0;
}