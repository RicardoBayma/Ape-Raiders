#ifndef RANKING_H
#define RANKING_H
#include "raylib.h"
#include <stdio.h>
#include <string.h>

//ranking Top 5 e pop-up para digitar o nome

#define RANK_MAX 5     //5 jogadores no ranking
#define NOME_MAX 15    //maximo de letras do nome
#define NOME_TAM 26    //deixa digitar ate 25 letras mas apraece aviso

typedef struct {
    char nome[NOME_TAM];
    int pontos;
} Recorde;

static Recorde ranking[RANK_MAX]; //posicao 0 e 1o lugar
static int rankQtd = 0;

static bool popupAberto = false;
static bool nomeSalvo = false;    //se apertar pular aparece como anonimo
static char nome[NOME_TAM];
static const char *erro = NULL;

static const Rectangle CAIXA = { 413, 355, 370, 54 };
static const Rectangle SALVAR = { 803, 355, 160, 54 };
static const Rectangle PULAR = { 803, 425, 160, 54 };

static void CarregarRanking(void) {
    FILE *f = fopen("ranking.txt", "r");
    if (f == NULL) return;
    while (rankQtd < RANK_MAX && fscanf(f, "%d %25[^\r\n]", &ranking[rankQtd].pontos, ranking[rankQtd].nome) == 2) rankQtd++;
    fclose(f);
}

//coloca o jogador na posicao certa
static void InserirRanking(int pontos) {
    int pos = 0;
    while (pos < rankQtd && ranking[pos].pontos >= pontos) pos++;
    if (pos >= RANK_MAX) return; //n entrou no Top 5

    if (rankQtd < RANK_MAX) rankQtd++;
    for (int i = rankQtd - 1; i > pos; i--) ranking[i] = ranking[i - 1]; //empurra os de baixo
    strcpy(ranking[pos].nome, nome);
    ranking[pos].pontos = pontos;

    FILE *f = fopen("ranking.txt", "w");
    if (f == NULL) return;
    for (int i = 0; i < rankQtd; i++) fprintf(f, "%d %s\n", ranking[i].pontos, ranking[i].nome);
    fclose(f);
}

static void AbrirPopupNome(void) {
    popupAberto = true;
    nomeSalvo = false;
    nome[0] = '\0';
    erro = NULL;
}

//teclado e mouse do pop-up
static void AtualizarPopupNome(int pontos) {
    int len = (int)strlen(nome);
    for (int c = GetCharPressed(); c > 0; c = GetCharPressed()) {
        if (c >= 32 && c <= 126 && len < NOME_TAM - 1) { nome[len++] = (char)c; nome[len] = '\0'; }
    }
    if (IsKeyPressed(KEY_BACKSPACE) && len > 0) nome[--len] = '\0';

    bool clique = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (clique && CheckCollisionPointRec(GetMousePosition(), PULAR)) popupAberto = false;

    if (IsKeyPressed(KEY_ENTER) || (clique && CheckCollisionPointRec(GetMousePosition(), SALVAR))) {
        if (len == 0) erro = "Digite um nome";
        else if (len > NOME_MAX) erro = "Nome muito longo";
        else {
            popupAberto = false;
            nomeSalvo = true;
            InserirRanking(pontos);
        }
    }
}

//texto centralizado com sombra
static void TextoCentro(const char *t, int x, int largura, int y, int tam, Color cor) {
    while (MeasureText(t, tam) > largura) tam--;
    int tx = x + (largura - MeasureText(t, tam)) / 2;
    DrawText(t, tx + 2, y + 2, tam, BLACK);
    DrawText(t, tx, y, tam, cor);
}

static void DesenharBotao(Rectangle r, const char *texto) {
    DrawRectangleRec(r, (Color){ 196, 132, 62, 255 });
    DrawRectangleLinesEx(r, 3, (Color){ 60, 34, 16, 255 });
    TextoCentro(texto, r.x, r.width, r.y + 13, 28, WHITE);
}

//placa de madeira pendurada por cordas
static void DesenharPopupNome(void) {
    DrawRectangle(0, 0, 1376, 768, Fade(BLACK, 0.55f));
    DrawLineEx((Vector2){ 488, 110 }, (Vector2){ 458, 268 }, 6, (Color){ 196, 152, 92, 255 });
    DrawLineEx((Vector2){ 888, 110 }, (Vector2){ 918, 268 }, 6, (Color){ 196, 152, 92, 255 });
    DrawRectangle(368, 250, 640, 300, (Color){ 164, 104, 55, 255 });
    DrawRectangleLinesEx((Rectangle){ 368, 250, 640, 300 }, 5, (Color){ 64, 36, 16, 255 });

    TextoCentro("Nome para salvar:", 368, 640, 290, 36, GOLD);
    DrawRectangleRec(CAIXA, (Color){ 62, 34, 14, 255 });
    DrawRectangleLinesEx(CAIXA, 3, erro ? RED : BEIGE);
    DrawText(TextFormat("%s_", nome), 425, 368, 26, WHITE);
    if (erro) DrawText(erro, 413, 425, 26, RED);
    DesenharBotao(SALVAR, "Salvar");
    DesenharBotao(PULAR, "Pular");
}

//tabela do Top 5
static void DesenharRanking(int score, int baloes) {
    static const int linhaY[RANK_MAX] = { 376, 417, 453, 491, 528 };
    TextoCentro("RANKING", 387, 390, 278, 32, GOLD);
    TextoCentro("NOME", 402, 190, 338, 22, GOLD);
    TextoCentro("SCORE", 620, 140, 338, 22, GOLD);
    for (int i = 0; i < rankQtd; i++) {
        TextoCentro(TextFormat("%d. %s", i + 1, ranking[i].nome), 402, 190, linhaY[i], 20, WHITE);
        TextoCentro(TextFormat("%04d", ranking[i].pontos), 620, 140, linhaY[i], 22, WHITE);
    }

    const char *rotulos[3] = { "SCORE:", "BALÕES ESTOURADOS:", "JOGADOR:" };
    const char *valores[3] = { TextFormat("%04d", score), TextFormat("%d", baloes), nomeSalvo ? nome : "Anônimo" };
    TextoCentro("ESTATÍSTICAS", 819, 394, 278, 32, GOLD);
    for (int i = 0; i < 3; i++) {
        DrawText(rotulos[i], 841, 362 + i * 73, 24, GOLD);
        DrawText(valores[i], 1191 - MeasureText(valores[i], 24), 362 + i * 73, 24, WHITE);
    }
}

#endif
