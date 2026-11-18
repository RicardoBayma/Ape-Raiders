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
#define DARDOS_MAX 20 //numero maximo de dardos na tela para não sobrecarregar
#define DARDOS_VEL 15.0f //ajuste da velocidade do tiro
#define DARDOS_RECARGA 0.5f //intervalo do tiro automatico

#define EXPLOSOES_MAX 20 //maximo de explosoes na tela ao mesmo tempo
#define EXPLOSAO_QUADROS 10 //quantidade de quadros da explosao na sheet
#define EXPLOSAO_COLUNAS 3 //quadros por linha na sheet
#define EXPLOSAO_TAM 200 //largura e altura de cada quadro da sheet (pixels)
#define EXPLOSAO_DURACAO 0.08f //segundos que cada quadro fica na tela

#define ROSTO_X 88
#define ROSTO_Y 280
#define ROSTO_W 204
#define ROSTO_H 208

//telas, fiz em enum
typedef enum {
    TELA_MENU,
    TELA_JOGO,
    TELA_DERROTA,
    TELA_VITORIA,
    TELA_PAUSE
} Tela;

typedef struct {
    float x, y;
    bool atv;
} Dardo;

typedef struct {
    float x, y; //centro da explosao
    float tam; //tamanho desenhado na tela
    int quadro; //quadro atual da animacao
    float tempo; //tempo no quadro atual
    bool atv;
} Explosao;

int main (void){
    InitWindow(LARGURA, ALTURA, "Ape Raiders"); //cria a janela inicial
    SetTargetFPS(60);
    SetWindowMinSize(1376, 768);

    Texture2D sheet = LoadTexture("macaco_sheet.png");
    Texture2D inicial = LoadTexture("inicial.png");
    Texture2D fundo = LoadTexture("fundo.png");
    Texture2D derrota = LoadTexture("derrota.png");
    Texture2D rostos = LoadTexture("macaco_rostos.png");

    Texture2D coracao = LoadTexture("coracao.png");
    SetTextureFilter(coracao, TEXTURE_FILTER_TRILINEAR);

    Texture2D textDardo = LoadTexture("dardo.png");
    Dardo dardos[DARDOS_MAX] = {0};
    float recarga = 0;//tempo para poder atirar novamente

    Texture2D textExplosao = LoadTexture("explode.png");
    SetTextureFilter(textExplosao, TEXTURE_FILTER_BILINEAR); //deixa a explosao mais sutil no fim
    Explosao explosoes[EXPLOSOES_MAX] = {0};

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
            for (int i = 0; i < DARDOS_MAX; i++) dardos[i].atv = false; //limpa os dardos
            for (int e = 0; e < EXPLOSOES_MAX; e++) explosoes[e].atv = false; //limpa as explosoes
            recarga = 0;
            tela = TELA_JOGO;
        }
    }

    else if (tela == TELA_JOGO) {
        //movimentação do macaco
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {if (x > AREA_X) x -= velocidade;}
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {if (x < AREA_X + AREA_LARGURA - TAM_QUADRO) x+= velocidade;}

        if (IsKeyPressed(KEY_M)) tela = TELA_MENU;
        //tiro automatico
        recarga-=GetFrameTime();
        if(recarga <=0){
            for(int i=0;i<DARDOS_MAX;i++){
                if(!dardos[i].atv){
                    dardos[i].x = x + (TAM_QUADRO - textDardo.width) / 2.0f;
                    dardos[i].y = y - textDardo.height / 2.0f;
                    dardos[i].atv = true;
                    recarga = DARDOS_RECARGA;
                    break;
                }
            }
        }

        //move os dardos, checa se acertou algum balao
        for (int i = 0; i < DARDOS_MAX; i++) {
            if (!dardos[i].atv) continue;
            dardos[i].y -= DARDOS_VEL;

            if (dardos[i].y + textDardo.height < 0) { //dardo saiu por cima da tela
                dardos[i].atv = false;
                continue;
            }

            //define a hitbox do dardo para ser somente a ponta do png
            Rectangle hitDardo = { dardos[i].x + textDardo.width / 2.0f - 4, dardos[i].y, 8, textDardo.height / 2.0f };
            Balao *atingido = BalaoEmRetangulo(hitDardo);
            if (atingido != NULL) {
                int pontos = DanoBalao(atingido, 1);
                pontuacao += pontos;
                dardos[i].atv = false;

                //so ganha pontos quando o balao estoura, cria a explosao no centro dele
                if (pontos > 0) {
                    for (int e = 0; e < EXPLOSOES_MAX; e++) {
                        if (!explosoes[e].atv) {
                            explosoes[e].x = atingido->x + atingido->w / 2;
                            explosoes[e].y = atingido->y + atingido->h / 2;
                            explosoes[e].tam = atingido->w * 1.5f; //ajuste da escala em relacao ao balao
                            explosoes[e].quadro = 0;
                            explosoes[e].tempo = 0;
                            explosoes[e].atv = true;
                            break;
                        }
                    }
                }
            }
        }

        //percorre a spreadsheet da explosao
        for (int e = 0; e < EXPLOSOES_MAX; e++) {
            if (!explosoes[e].atv) continue;
            explosoes[e].tempo += GetFrameTime();
            if (explosoes[e].tempo >= EXPLOSAO_DURACAO) {
                explosoes[e].tempo -= EXPLOSAO_DURACAO;
                explosoes[e].quadro++;
                if (explosoes[e].quadro >= EXPLOSAO_QUADROS) explosoes[e].atv = false;
            }
        }

        if (IsKeyPressed(KEY_H)) {
            vida--;
        }
        
        if (IsKeyPressed(KEY_P)) tela = TELA_PAUSE;

        //pressionar P para pausar o jogo e vice-versa
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
            DrawTexture(inicial, 0, 0, WHITE);

            if ((int)(GetTime() * 2) % 2 == 0) {
                const char *msg = "PRESSIONE ENTER PARA JOGAR";
                DrawText(msg, (LARGURA - MeasureText(msg, 24)) / 2, 700, 24, WHITE);
            }
            DrawText("ESC PARA SAIR", 10, ALTURA - 30, 20, WHITE);
        }

        else if (tela == TELA_JOGO || tela == TELA_PAUSE) {
            DrawTexture(fundo, 0, 0, WHITE);
            DrawBalao();

            for (int i = 0; i < DARDOS_MAX; i++) {
                if (dardos[i].atv) DrawTexture(textDardo, (int)dardos[i].x, (int)dardos[i].y, WHITE);
            }

            //desenho das explosoes
            for (int e = 0; e < EXPLOSOES_MAX; e++) {
                if (!explosoes[e].atv) continue;
                int q = explosoes[e].quadro;
                Rectangle recorte = { (q % EXPLOSAO_COLUNAS) * EXPLOSAO_TAM, (q / EXPLOSAO_COLUNAS) * EXPLOSAO_TAM, EXPLOSAO_TAM, EXPLOSAO_TAM };
                float t = explosoes[e].tam;
                Rectangle destino = { explosoes[e].x - t / 2, explosoes[e].y - t / 2, t, t };
                DrawTexturePro(textExplosao, recorte, destino, (Vector2){0, 0}, 0.0f, WHITE);
            }

            Rectangle recEsq = {0, 0, AREA_X, ALTURA};
            Rectangle recDir = {AREA_X + AREA_LARGURA, 0, LARGURA - (AREA_X + AREA_LARGURA), ALTURA};
            DrawTextureRec(fundo, recEsq, (Vector2){recEsq.x, recEsq.y}, WHITE);
            DrawTextureRec(fundo, recDir, (Vector2){recDir.x, recDir.y}, WHITE);
            DrawTextureRec(sheet, origem, (Vector2){ x, y }, WHITE);

            // quantas vidas já foram perdidas: 0 = normal, 1 = machucado, 2 = desmaiado
            int perdidas = VIDAS_MAX - vida;
            if (perdidas < 0) perdidas = 0;
            if (perdidas > 2) perdidas = 2;   // só existem 3 rostos

            Rectangle recRosto = { perdidas * ROSTO_W, 0, ROSTO_W, ROSTO_H };   // recorta o rosto certo
            DrawTextureRec(rostos, recRosto, (Vector2){ ROSTO_X, ROSTO_Y }, WHITE);

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
            //apertar M manda para o menu.

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
    UnloadTexture(inicial);
    UnloadTexture(fundo);
    UnloadTexture(derrota);
    UnloadTexture(coracao);
    UnloadTexture(textDardo);
    UnloadTexture(textExplosao);
    UnloadTexture(rostos);
    FinalizarBalao();
    CloseWindow();
    return 0;
}