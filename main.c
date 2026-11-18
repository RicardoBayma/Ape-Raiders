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

//upgrades (ajuste aqui o balanceamento do jogo)
#define VEL_MACACO_BASE 5.0f //velocidade inicial do macaco
#define VEL_MACACO_NIVEL 0.8f //quanto cada nivel do upgrade soma na velocidade do macaco
#define DARDOS_VEL_NIVEL 2.5f //quanto cada nivel do upgrade soma na velocidade do dardo
#define DARDOS_ESPACO 22 //distancia (pixels) entre dardos disparados juntos
#define ABATES_POR_UPGRADE 10 //baloes estourados para ganhar um upgrade

//interior do segundo quadrado (moldura de baixo do painel direito)
#define UPG_X 1100
#define UPG_Y 500
#define UPG_W 225
#define UPG_H 240

//telas, fiz em enum
typedef enum {
    TELA_MENU,
    TELA_JOGO,
    TELA_DERROTA,
    TELA_VITORIA,
    TELA_PAUSE
} Tela;

//tipos de upgrade (UPG_TOTAL = quantidade de tipos, serve de tamanho para os vetores)
typedef enum {
    UPG_DARDO,
    UPG_VEL_DARDO,
    UPG_VEL_MOV,
    UPG_TOTAL
} Upgrade;

static const int NIVEL_MAX[UPG_TOTAL] = { 2, 5, 5 }; //dardo: ate triplo (2 extras); velocidades: ate nivel 5
static const char *NOME_UPG[UPG_TOTAL] = { "DARDOS", "VEL. TIRO", "VEL. MOVER" };

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

//sorteia um upgrade entre os que ainda nao chegaram no nivel maximo
//devolve -1 se todos ja estao no maximo
static int SortearUpgrade(const int nivel[UPG_TOTAL]) {
    int disponiveis[UPG_TOTAL];
    int n = 0;
    for (int i = 0; i < UPG_TOTAL; i++) {
        if (nivel[i] < NIVEL_MAX[i]) disponiveis[n++] = i;
    }
    if (n == 0) return -1;
    return disponiveis[GetRandomValue(0, n - 1)];
}

//texto do aviso que aparece na tela quando ganha o upgrade
static const char *TextoUpgrade(int tipo, int nivelNovo) {
    if (tipo == UPG_DARDO) return (nivelNovo == 1) ? "DARDO DUPLO!" : "DARDO TRIPLO!";
    if (tipo == UPG_VEL_DARDO) return "DARDO MAIS RAPIDO!";
    return "MACACO MAIS RAPIDO!";
}

//desenha o painel de upgrades dentro do segundo quadrado
static void DesenharPainelUpgrades(const int nivel[UPG_TOTAL], int abates) {
    int x0 = UPG_X + 8;       //margem interna
    int larg = UPG_W - 16;    //largura util
    int y0 = UPG_Y + 6;

    const char *titulo = "UPGRADES";
    DrawText(titulo, UPG_X + (UPG_W - MeasureText(titulo, 14)) / 2, y0, 14, GOLD);
    y0 += 22;

    for (int i = 0; i < UPG_TOTAL; i++) {
        //no dardo mostra a quantidade atual (x1, x2, x3); nos outros, so o nome
        const char *rotulo = (i == UPG_DARDO) ? TextFormat("DARDOS x%d", 1 + nivel[i]) : NOME_UPG[i];
        DrawText(rotulo, x0, y0, 14, WHITE);

        //um quadradinho por nivel: dourado = ja ganhou, apagado = falta ganhar
        for (int k = 0; k < NIVEL_MAX[i]; k++) {
            Color cor = (k < nivel[i]) ? GOLD : Fade(WHITE, 0.2f);
            DrawRectangle(x0 + k * 12, y0 + 17, 8, 8, cor);
        }
        y0 += 32;
    }

    //barra de progresso ate o proximo upgrade
    DrawRectangle(x0, y0 + 4, larg, 10, Fade(BLACK, 0.5f));
    DrawRectangle(x0, y0 + 4, larg * abates / ABATES_POR_UPGRADE, 10, SKYBLUE);
    DrawRectangleLines(x0, y0 + 4, larg, 10, WHITE);
}

int main (void){
    InitWindow(LARGURA, ALTURA, "Ape Raiders"); //cria a janela inicial
    SetTargetFPS(60);
    SetWindowMinSize(1376, 768);

    Texture2D sheet = LoadTexture("assets/macaco_sheet.png");
    Texture2D inicial = LoadTexture("assets/inicial.png");
    Texture2D fundo = LoadTexture("assets/fundo.png");
    Texture2D derrota = LoadTexture("assets/derrota.png");
    Texture2D rostos = LoadTexture("assets/macaco_rostos.png"); //3 rostos de 204x208 lado a lado
    Texture2D coracao = LoadTexture("assets/coracao.png");
    SetTextureFilter(coracao, TEXTURE_FILTER_TRILINEAR);
    Texture2D textDardo = LoadTexture("assets/dardo.png");
    Dardo dardos[DARDOS_MAX] = {0};
    float recarga = 0;//tempo para poder atirar novamente
    Texture2D textExplosao = LoadTexture("assets/explode.png");
    SetTextureFilter(textExplosao, TEXTURE_FILTER_BILINEAR); //deixa a explosao mais sutil no fim
    Explosao explosoes[EXPLOSOES_MAX] = {0};

    Tela tela = TELA_MENU;

    float x = AREA_X + (AREA_LARGURA - TAM_QUADRO) / 2;
    float y = ALTURA - TAM_QUADRO - 10;
    int quadro = 0;
    float tempo = 0;
    float duracao = 1.0f / 12.0f;
    int vida = VIDAS_MAX;
    int nivel[UPG_TOTAL] = {0}; //nivel atual de cada upgrade (acumulativo)
    int abates = 0; //baloes estourados desde o ultimo upgrade
    float avisoTempo = 0; //tempo que o aviso de upgrade ainda fica na tela
    const char *avisoTexto = "";

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
    if (tela == TELA_MENU) {
        if (IsKeyPressed(KEY_ENTER)) {
            x = AREA_X + (AREA_LARGURA - TAM_QUADRO) / 2; 
            vida = VIDAS_MAX;
            pontuacao = 0;
            ReiniciarBaloes(); //limpa os baloes da partida passada
            for (int i = 0; i < DARDOS_MAX; i++) dardos[i].atv = false; //limpa os dardos
            for (int e = 0; e < EXPLOSOES_MAX; e++) explosoes[e].atv = false; //limpa as explosoes
            for (int u = 0; u < UPG_TOTAL; u++) nivel[u] = 0; //zera os upgrades da partida passada
            abates = 0;
            avisoTempo = 0;
            recarga = 0;
            tela = TELA_JOGO;
        }
    }

    else if (tela == TELA_JOGO) {
        //valores que dependem dos upgrades (recalculados todo frame)
        float velocidade = VEL_MACACO_BASE + nivel[UPG_VEL_MOV] * VEL_MACACO_NIVEL;
        float velDardo = DARDOS_VEL + nivel[UPG_VEL_DARDO] * DARDOS_VEL_NIVEL;
        int qtdDardos = 1 + nivel[UPG_DARDO]; //1 = simples, 2 = duplo, 3 = triplo

        //movimentação do macaco
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {if (x > AREA_X) x -= velocidade;}
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {if (x < AREA_X + AREA_LARGURA - TAM_QUADRO) x+= velocidade;}

        if (IsKeyPressed(KEY_M)) tela = TELA_MENU;
        //tiro automatico
        recarga-=GetFrameTime();
        if(recarga <=0){
            int criados = 0;
            for(int i=0;i<DARDOS_MAX && criados < qtdDardos;i++){
                if(!dardos[i].atv){
                    //espalha os dardos lado a lado, centralizados no macaco
                    float deslocamento = (criados - (qtdDardos - 1) / 2.0f) * DARDOS_ESPACO;
                    dardos[i].x = x + (TAM_QUADRO - textDardo.width) / 2.0f + deslocamento;
                    dardos[i].y = y - textDardo.height / 2.0f;
                    dardos[i].atv = true;
                    criados++;
                }
            }
            if (criados > 0) recarga = DARDOS_RECARGA;
        }

        //move os dardos, checa se acertou algum balao
        for (int i = 0; i < DARDOS_MAX; i++) {
            if (!dardos[i].atv) continue;
            dardos[i].y -= velDardo;

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

                    //conta o balao estourado; ao completar, sorteia um upgrade
                    abates++;
                    if (abates >= ABATES_POR_UPGRADE) {
                        abates = 0;
                        int sorteado = SortearUpgrade(nivel);
                        if (sorteado >= 0) {
                            nivel[sorteado]++;
                            avisoTexto = TextoUpgrade(sorteado, nivel[sorteado]);
                            avisoTempo = 2.0f;
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

        //o aviso de upgrade some depois de 2 segundos
        if (avisoTempo > 0) avisoTempo -= GetFrameTime();

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

        //teste de upgrade (tirar futuramente): U sorteia um upgrade na hora
        if (IsKeyPressed(KEY_U)) {
            int sorteado = SortearUpgrade(nivel);
            if (sorteado >= 0) {
                nivel[sorteado]++;
                avisoTexto = TextoUpgrade(sorteado, nivel[sorteado]);
                avisoTempo = 2.0f;
            }
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

            //segundo quadrado: lista de upgrades e progresso
            DesenharPainelUpgrades(nivel, abates);

            //aviso do upgrade ganho, no alto da praia (some suavemente no ultimo meio segundo)
            if (avisoTempo > 0) {
                float alfa = fminf(1.0f, avisoTempo / 0.5f);
                int larguraAviso = MeasureText(avisoTexto, 28);
                int ax = AREA_X + (AREA_LARGURA - larguraAviso) / 2;
                DrawText(avisoTexto, ax + 2, 62, 28, Fade(BLACK, alfa)); //sombra
                DrawText(avisoTexto, ax, 60, 28, Fade(GOLD, alfa));
            }

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