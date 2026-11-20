#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include "balloons.h"
#include "estouro.h" //som de estouro dos baloes
#include "boss.h" //chefao
#include "explosao.h" //animacao da explosao
#include "upgrades.h" //upgrades
#include "ranking.h" //Top 5 do ranking salvo aqui

#define NUM_QUADROS 22 //quantidade de frames do macaco animacao
#define TAM_QUADRO 128 //area do sprote
#define LARGURA 1376 //largura do mapa
#define ALTURA 768 //atura do mapa
#define AREA_X 400
#define AREA_LARGURA 588
#define VIDAS_MAX 3
#define DARDOS_MAX 20 //numero maximo de dardos na tela pra n sobrecarregar
#define DARDOS_VEL 15.0f //velocidade dardo
#define DARDOS_RECARGA 0.5f //intervalo do tiro
#define DARDOS_ESPACO 22 //distancia entre dardos juntos
#define VEL_MACACO_BASE 5.0f //velocidade inicial do macaco
#define VEL_MACACO_NIVEL 0.8f //velocidade adicionada pro macaco por upgrade
#define DARDOS_VEL_NIVEL 2.5f //velocidade adicionada pro dardo por upgrade
#define ROSTO_W 204 //cada rosto do macaco_rostos.png
#define ROSTO_H 208
#define VOLUME_MUSICA 0.5f //volume da musica

//telas, fiz em enum
typedef enum { TELA_MENU, TELA_JOGO, TELA_DERROTA, TELA_PAUSE } Tela;

typedef struct {
    float x, y;
    bool atv;
} Dardo;

static const char *avisoTexto = "";
static float avisoTempo = 0;
static void Avisar(const char *texto) {
    avisoTexto = texto;
    avisoTempo = 2.0f;
}

static void TextoMeio(const char *t, int y, int tam, Color cor) {
    DrawText(t, (LARGURA - MeasureText(t, tam)) / 2, y, tam, cor);
}

int main (void){
    InitWindow(LARGURA, ALTURA, "Ape Raiders"); //cria a janela inicial
    SetTargetFPS(60);
    SetWindowMinSize(LARGURA, ALTURA);
    IniciarEstouro();

    //toca a musica de fundo e deixa em loop, se der a gente bota mais musicas
    Music soundtrack = LoadMusicStream("assets/soundtrack.wav");
    soundtrack.looping = true;
    SetMusicVolume(soundtrack, VOLUME_MUSICA);
    PlayMusicStream(soundtrack);

    Texture2D sheet = LoadTexture("assets/macaco_sheet.png");
    Texture2D inicial = LoadTexture("assets/inicial.png");
    Texture2D fundo = LoadTexture("assets/fundo.png");
    Texture2D derrota = LoadTexture("assets/derrota.png");
    Texture2D rostos = LoadTexture("assets/macaco_rostos.png");
    Texture2D coracao = LoadTexture("assets/coracao.png");
    Texture2D textDardo = LoadTexture("assets/dardo.png");
    SetTextureFilter(coracao, TEXTURE_FILTER_TRILINEAR);
    IniciarExplosoes();

    Tela tela = TELA_MENU;
    Dardo dardos[DARDOS_MAX] = {0};
    float recarga = 0;
    float x = 0, y = ALTURA - TAM_QUADRO - 10;
    int quadro = 0;
    float tempo = 0;
    int vida = 0, pontuacao = 0, baloesEstourados = 0, highScore = 0;

    //limites do spawn dos baloes pra eles n sairem da tela do jogo
    IniciarBalao((Rectangle){ AREA_X, 0, AREA_LARGURA, ALTURA }, y);
    IniciarBoss((Rectangle){ AREA_X, 0, AREA_LARGURA, ALTURA }, y);

    FILE *ficheiro = fopen("highscore.txt", "r");
    if (ficheiro != NULL) {
        fscanf(ficheiro, "%d", &highScore);
        fclose(ficheiro);
    }
    CarregarRanking(); //le o Top 5 salvo nas partidas anteriores

while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    UpdateMusicStream(soundtrack);

    //animacao do macaco 12 fps
    if (tela != TELA_PAUSE && (tempo += dt) >= 1.0f / 12.0f) {
        tempo -= 1.0f / 12.0f;
        quadro = (quadro + 1) % NUM_QUADROS;
    }

    if (tela == TELA_MENU) {
        if (IsKeyPressed(KEY_ENTER)) { //comeca uma partida nova do zero
            x = AREA_X + (AREA_LARGURA - TAM_QUADRO) / 2;
            vida = VIDAS_MAX;
            pontuacao = baloesEstourados = 0;
            recarga = avisoTempo = 0;
            for (int i = 0; i < DARDOS_MAX; i++) dardos[i].atv = false;
            ReiniciarBaloes();
            ReiniciarBoss();
            LimparExplosoes();
            ReiniciarUpgrades();
            tela = TELA_JOGO;
        }
    }

    else if (tela == TELA_JOGO) {
        //valores que dependem dos upgrades
        float velocidade = VEL_MACACO_BASE + nivel[UPG_VEL_MOV] * VEL_MACACO_NIVEL;
        float velDardo = DARDOS_VEL + nivel[UPG_VEL_DARDO] * DARDOS_VEL_NIVEL;
        int qtdDardos = 1 + nivel[UPG_DARDO]; //1 é um, 2 = duplo, 3 = triplo...

        //movimentacao do macaco
        if ((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && x > AREA_X) x -= velocidade;
        if ((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && x < AREA_X + AREA_LARGURA - TAM_QUADRO) x += velocidade;
        if (IsKeyPressed(KEY_M)) tela = TELA_MENU;

        //tiro automatico com os dardo centralizados no macaco
        if ((recarga -= dt) <= 0) {
            int criados = 0;
            for (int i = 0; i < DARDOS_MAX && criados < qtdDardos; i++) {
                if (dardos[i].atv) continue;
                float deslocamento = (criados - (qtdDardos - 1) / 2.0f) * DARDOS_ESPACO;
                dardos[i] = (Dardo){ x + (TAM_QUADRO - textDardo.width) / 2.0f + deslocamento, y - textDardo.height / 2.0f, true };
                criados++;
            }
            if (criados > 0) recarga = DARDOS_RECARGA;
        }

        //move os dardos e checa se acertaram o boss ou algum balao
        for (int i = 0; i < DARDOS_MAX; i++) {
            if (!dardos[i].atv) continue;
            dardos[i].y -= velDardo;
            if (dardos[i].y + textDardo.height < 0) { dardos[i].atv = false; continue; }

            //hitbox so na ponta do dardo
            Rectangle hitDardo = { dardos[i].x + textDardo.width / 2.0f - 4, dardos[i].y, 8, textDardo.height / 2.0f };

            if (BossAtivo()) { //com apenas o boss os dardos so acertam ele
                int resultado = AcertarBoss(hitDardo);
                if (resultado != BOSS_ERROU) dardos[i].atv = false;
                if (resultado == BOSS_MORREU) {
                    Rectangle rb = RetanguloBoss();
                    TocarEstouro();
                    CriarExplosao(rb.x + rb.width / 2, rb.y + rb.height / 2, rb.width * 1.5f);
                    pontuacao += BOSS_PONTOS;
                    baloesEstourados++; //o boss tambem e um balao
                    Avisar("BOSS DERROTADO!");
                }
                continue;
            }

            Balao *atingido = BalaoEmRetangulo(hitDardo);
            if (atingido == NULL) continue;
            dardos[i].atv = false;
            int pontos = DanoBalao(atingido, 1);
            if (pontos > 0) { //so ganha pontos quando o balao estoura
                pontuacao += pontos;
                baloesEstourados++;
                TocarEstouro();
                CriarExplosao(atingido->x + atingido->w / 2, atingido->y + atingido->h / 2, atingido->w * 1.5f);
                const char *upgrade = ContarAbate(); //a cada 7 baloes ganha um upgrade
                if (upgrade != NULL) Avisar(upgrade);
            }
        }

        AttExplosoes(dt);
        if (avisoTempo > 0) avisoTempo -= dt;

        if (IsKeyPressed(KEY_P)) {
            tela = TELA_PAUSE;
            PauseMusicStream(soundtrack);
        }

        //o boss ou os baloes que chegam no macaco tiram vida
        if (BossAtivo()) vida -= AttBoss(dt);
        else {
            vida -= AttBalao(dt, pontuacao);
            if (ChecarBoss(pontuacao)) { //chegou em 500, 1000...: troca os baloes pelo boss
                ReiniciarBaloes();
                Avisar("CUIDADO: BOSS!");
            }
        }

        if (vida <= 0) {
            if (pontuacao > highScore) { //salva o novo recorde
                highScore = pontuacao;
                FILE *ficheiroEscrita = fopen("highscore.txt", "w");
                if (ficheiroEscrita != NULL) {
                    fprintf(ficheiroEscrita, "%d", highScore);
                    fclose(ficheiroEscrita);
                }
            }
            tela = TELA_DERROTA;
            AbrirPopupNome(); //o pop-up do nome aparece assim que a derrota abre
        }
    }

    else if (tela == TELA_DERROTA) {
        if (popupAberto) AtualizarPopupNome(pontuacao);
        else if (IsKeyPressed(KEY_ENTER)) tela = TELA_MENU;
    }

    else if (tela == TELA_PAUSE && IsKeyPressed(KEY_P)) {
        tela = TELA_JOGO;
        ResumeMusicStream(soundtrack);
    }

    BeginDrawing();
        ClearBackground(BLACK);

        if (tela == TELA_MENU) {
            DrawTexture(inicial, 0, 0, WHITE);
            if ((int)(GetTime() * 2) % 2 == 0) TextoMeio("PRESSIONE ENTER PARA JOGAR", 700, 24, WHITE);
            DrawText("ESC PARA SAIR", 10, ALTURA - 30, 20, WHITE);
        }

        else if (tela == TELA_JOGO || tela == TELA_PAUSE) {
            DrawTexture(fundo, 0, 0, WHITE);
            if (BossAtivo()) DrawBoss();
            else DrawBalao();
            for (int i = 0; i < DARDOS_MAX; i++) {
                if (dardos[i].atv) DrawTexture(textDardo, (int)dardos[i].x, (int)dardos[i].y, WHITE);
            }
            DrawExplosoes();

            //redesenha as laterais do fundo por cima, para nada aparecer fora da praia
            DrawTextureRec(fundo, (Rectangle){ 0, 0, AREA_X, ALTURA }, (Vector2){ 0, 0 }, WHITE);
            DrawTextureRec(fundo, (Rectangle){ AREA_X + AREA_LARGURA, 0, LARGURA - (AREA_X + AREA_LARGURA), ALTURA }, (Vector2){ AREA_X + AREA_LARGURA, 0 }, WHITE);
            DrawTextureRec(sheet, (Rectangle){ quadro * TAM_QUADRO, 0, TAM_QUADRO, TAM_QUADRO }, (Vector2){ x, y }, WHITE);

            //rosto: 0 = normal, 1 = machucado, 2 = desmaiado, dps disso ele morre e o jogador perde
            int perdidas = VIDAS_MAX - vida;
            if (perdidas > 2) perdidas = 2;
            DrawTextureRec(rostos, (Rectangle){ perdidas * ROSTO_W, 0, ROSTO_W, ROSTO_H }, (Vector2){ 88, 280 }, WHITE);

            //coracoes centralizados no quadro de baixo da esquerda
            int espaco = 12;
            int inicioX = (385 - (VIDAS_MAX * coracao.width + (VIDAS_MAX - 1) * espaco)) / 2;
            for (int i = 0; i < VIDAS_MAX; i++) {
                DrawTexture(coracao, inicioX + i * (coracao.width + espaco), 650 + (85 - coracao.height) / 2, i < vida ? WHITE : Fade(WHITE, 0.2f));
            }

            DrawText(TextFormat("SCORE: %04d", pontuacao), 80, 70, 30, WHITE);
            DrawText(TextFormat("HI-SCORE: %04d", highScore), 80, 100, 30, GOLD);
            DesenharPainelUpgrades();

            //aviso no topo da tela
            if (avisoTempo > 0) {
                float alfa = fminf(1.0f, avisoTempo / 0.5f);
                int ax = AREA_X + (AREA_LARGURA - MeasureText(avisoTexto, 28)) / 2;
                DrawText(avisoTexto, ax + 2, 62, 28, Fade(BLACK, alfa)); //sombra
                DrawText(avisoTexto, ax, 60, 28, Fade(GOLD, alfa));
            }

            if (tela == TELA_PAUSE) {
                DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.6f));
                TextoMeio("APERTE P PARA VOLTAR", 240, 20, GRAY);
            }
        }

        else if (tela == TELA_DERROTA) {
            DrawTexture(derrota, 0, 0, WHITE);
            DesenharRanking(pontuacao, baloesEstourados); //Top 5 e estatisticas da partida
            if (popupAberto) DesenharPopupNome();
            else if ((int)(GetTime() * 2) % 2 == 0) TextoMeio("ENTER PARA VOLTAR AO MENU", 720, 24, WHITE);
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
    UnloadTexture(rostos);
    FinalizarExplosoes();
    FinalizarBalao();
    FinalizarBoss();
    UnloadMusicStream(soundtrack);
    FinalizarEstouro();
    CloseWindow();
    return 0;
}
