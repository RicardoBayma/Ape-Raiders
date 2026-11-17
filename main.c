#include "raylib.h"
#include <math.h> //fminf(): devolve o menor numero entre dois numeros decimais
#include <stdio.h> //para poder abrir o arquivo txt para salvar highscore
#include "balloons.h"

#define NUM_QUADROS 22 //quantidade de quadros da animação do macaco
#define TAM_QUADRO 128 //largura e altura de cada quadro na sprite (pixels)
#define LARGURA 1376 //largura do 'mundo'
#define ALTURA 768 //altura do 'mundo'
#define AREA_X 400
#define AREA_LARGURA 588
#define VIDAS_MAX 3

//telas, fiz em enum
typedef enum {
    TELA_MENU,
    TELA_JOGO,
    TELA_DERROTA,
    TELA_VITORIA,
    TELA_PAUSE
} Tela;

int main (void){
    InitWindow(LARGURA, ALTURA, "Ape Raiders"); //cria a janela inicial
    SetTargetFPS(60);
    SetWindowMinSize(1376, 768);

    Texture2D sheet = LoadTexture("macaco_sheet.png");
    Texture2D logo = LoadTexture("logo.png");
    GenTextureMipmaps(&logo);
    SetTextureFilter(logo, TEXTURE_FILTER_TRILINEAR);

    Texture2D fundo = LoadTexture("fundo.png");
    Texture2D derrota = LoadTexture("derrota.png");

    Texture2D coracao = LoadTexture("coracao.png");
    SetTextureFilter(coracao, TEXTURE_FILTER_TRILINEAR);

    Tela tela = TELA_MENU;

    float x = AREA_X + (AREA_LARGURA - TAM_QUADRO) / 2;
    float y = ALTURA - TAM_QUADRO - 10;
    float velocidade = 5;
    int quadro = 0;
    float tempo = 0;
    float duracao = 1.0f / 12.0f;
    int vida = VIDAS_MAX;

    //limites do spawn dos baloes pra eles n sairem da tela do jogo
    IniciarBalao((Rectangle){ AREA_X, 0, AREA_LARGURA, ALTURA }, y);
    
    int pontuacao = 0;
    int highScore = 0;

    FILE *ficheiro = fopen("highscore.txt", "r");

    if (ficheiro != NULL) {
        fscanf(ficheiro, "%d", &highScore);
        fclose(ficheiro);
    }

while (!WindowShouldClose()) {

    //animação do macaco
    tempo += GetFrameTime();
    if (tempo >= duracao) {
        tempo -= duracao;
        quadro = (quadro + 1) % NUM_QUADROS;
    }

    if (tela == TELA_MENU) {
        if (IsKeyPressed(KEY_ENTER)) {
            x = AREA_X + (AREA_LARGURA - TAM_QUADRO) / 2; 
            vida = VIDAS_MAX;
            pontuacao = 0;
            ReiniciarBaloes(); //limpa os baloes da partida passada
            tela = TELA_JOGO;
        }
    }

    else if (tela == TELA_JOGO) {
        //movimentação do macaco
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {if (x > AREA_X) x -= velocidade;}
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {if (x < AREA_X + AREA_LARGURA - TAM_QUADRO) x+= velocidade;}

        if (IsKeyPressed(KEY_M)) tela = TELA_MENU;

        if (IsKeyPressed(KEY_H)) {
            vida--;
        }
        
        if (IsKeyPressed(KEY_P)) tela = TELA_PAUSE;

        //precionar P para pausar o jogo e vice-versa
        //cada balao que chega no macaco tira 1 vida
        vida -= AttBalao(GetFrameTime(), pontuacao);
        
        if (vida <= 0) {
            // ESCRITA NO ARQUIVO: Salva o novo recorde se bateu
            if (pontuacao > highScore) {
                highScore = pontuacao;
                FILE *ficheiroEscrita = fopen("highscore.txt", "w");
                if (ficheiroEscrita != NULL) {
                    fprintf(ficheiroEscrita, "%d", highScore);
                    fclose(ficheiroEscrita);
                }
            }
            tela = TELA_DERROTA;
        }
        //aumentar a pontuação para teste (tirar futuramente)
        if (IsKeyPressed(KEY_K)) {
            pontuacao += 10;
        }
        //teste (tirar futuramente): J tira 1 de vida do balao mais proximo do macaco
        if (IsKeyPressed(KEY_J)) {
            Balao *alvoBalao = BalaoMaisBaixo();
            pontuacao += DanoBalao(alvoBalao, 1);
        }
        //teste de vitoria (tirar futuramente)
        if (IsKeyPressed(KEY_V)) tela = TELA_VITORIA;
    }

    else if (tela == TELA_DERROTA) {
        if (IsKeyPressed(KEY_ENTER)) {
            tela = TELA_MENU;
        }
    }
    
    else if (tela == TELA_VITORIA) {
        if (IsKeyPressed(KEY_ENTER)) {
            tela = TELA_MENU;
        }
    }

    else if (tela == TELA_PAUSE) {
        if (IsKeyPressed(KEY_P)) {
            tela = TELA_JOGO;
        }
    }

    Rectangle origem = { quadro * TAM_QUADRO, 0, TAM_QUADRO, TAM_QUADRO };

    BeginDrawing();
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

        else if (tela == TELA_JOGO || tela == TELA_PAUSE) {
            DrawTexture(fundo, 0, 0, WHITE);
            DrawBalao();

            Rectangle recEsq = {0, 0, AREA_X, ALTURA};
            Rectangle recDir = {AREA_X + AREA_LARGURA, 0, LARGURA - (AREA_X + AREA_LARGURA), ALTURA};
            DrawTextureRec(fundo, recEsq, (Vector2){recEsq.x, recEsq.y}, WHITE);
            DrawTextureRec(fundo, recDir, (Vector2){recDir.x, recDir.y}, WHITE);
            DrawTextureRec(sheet, origem, (Vector2){ x, y }, WHITE);

            int espaco = 12;
            int larguratotal = VIDAS_MAX * coracao.width + (VIDAS_MAX - 1) * espaco;
            int inicioX = (385 - larguratotal) / 2;
            int coracaoY = 650 + (85 - coracao.height) / 2;

            for (int i = 0; i < VIDAS_MAX; i++) {
                Color cor = (i < vida) ? WHITE : Fade(WHITE, 0.2f);
                DrawTexture(coracao, inicioX + i * (coracao.width + espaco), coracaoY, cor);
            }

            DrawText(TextFormat("SCORE: %04d", pontuacao), 80, 70, 30, WHITE);
            DrawText(TextFormat("HI-SCORE: %04d", highScore), 80, 100, 30, GOLD);
            //apertar M ainda manda para o menu.

            if (tela == TELA_PAUSE) {
                DrawRectangle(0,0, LARGURA, ALTURA, Fade(BLACK, 0.6f));
                DrawText("APERTE P PARA VOLTAR", (LARGURA - MeasureText("APERTE P PARA VOLTAR", 20)) / 2, 240, 20, GRAY);
            }
        }

        else if (tela == TELA_DERROTA) {
            DrawTexture(derrota, 0, 0, WHITE);
            DrawText("ENTER PARA VOLTAR AO MENU", (LARGURA - MeasureText("ENTER PARA VOLTAR AO MENU", 20)) / 2, 320, 20, GRAY);
        }

        else if (tela == TELA_VITORIA) {
            const char *msg = "VOCE VENCEU";
            DrawText(msg, (LARGURA - MeasureText(msg,40))/2, 240, 40, GREEN);
            DrawText("ENTER PARA VOLTAR AO MENU", (LARGURA - MeasureText("ENTER PARA VOLTAR AO MENU", 20)) / 2, 320, 20, GRAY);
        }
    EndDrawing();
    }
    
    //limpa a memoria
    UnloadTexture(sheet);
    UnloadTexture(logo);
    UnloadTexture(fundo);
    UnloadTexture(derrota);
    UnloadTexture(coracao);
    FinalizarBalao();
    CloseWindow();
    return 0;
}
