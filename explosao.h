#ifndef EXPLOSAO_H
#define EXPLOSAO_H
#include "raylib.h"

#define EXPLOSOES_MAX 20       //maximo de explosoes na tela ao mesmo tempo
#define EXPLOSAO_QUADROS 10    //quantidade de quadros da explosao na sheet
#define EXPLOSAO_COLUNAS 3     //quadros por linha na sheet
#define EXPLOSAO_LINHAS 4      //linhas de quadros na sheet (600x800 = 3x4 quadros)
#define EXPLOSAO_TAM 200       //largura e altura de cada quadro da sheet (pixels)
#define EXPLOSAO_DURACAO 0.08f //segundos que cada quadro fica na tela

typedef struct {
    float x, y;  //centro da explosao
    float tam;   //tamanho desenhado na tela
    int quadro;  //quadro atual da animacao
    float tempo; //tempo no quadro atual
    bool atv;
} Explosao;

static Explosao explosoes[EXPLOSOES_MAX];
static Texture2D texExplosao;

static Rectangle quadrosExplosao[EXPLOSAO_LINHAS][EXPLOSAO_COLUNAS];

static void IniciarExplosoes(void) {
    texExplosao = LoadTexture("assets/explode.png");
    SetTextureFilter(texExplosao, TEXTURE_FILTER_BILINEAR); //deixa a explosao mais sutil no fim
    for (int l = 0; l < EXPLOSAO_LINHAS; l++) {
        for (int c = 0; c < EXPLOSAO_COLUNAS; c++) {
            quadrosExplosao[l][c] = (Rectangle){ c * EXPLOSAO_TAM, l * EXPLOSAO_TAM, EXPLOSAO_TAM, EXPLOSAO_TAM };
        }
    }
}

static void LimparExplosoes(void) {
    for (int e = 0; e < EXPLOSOES_MAX; e++) explosoes[e].atv = false;
}

static void CriarExplosao(float cx, float cy, float tam) {
    for (int e = 0; e < EXPLOSOES_MAX; e++) {
        if (!explosoes[e].atv) {
            explosoes[e] = (Explosao){ cx, cy, tam, 0, 0, true };
            return;
        }
    }
}

//a explosao some no ultimo frame
static void AttExplosoes(float dt) {
    for (int e = 0; e < EXPLOSOES_MAX; e++) {
        if (!explosoes[e].atv) continue;
        explosoes[e].tempo += dt;
        if (explosoes[e].tempo >= EXPLOSAO_DURACAO) {
            explosoes[e].tempo -= EXPLOSAO_DURACAO;
            if (++explosoes[e].quadro >= EXPLOSAO_QUADROS) explosoes[e].atv = false;
        }
    }
}

static void DrawExplosoes(void) {
    for (int e = 0; e < EXPLOSOES_MAX; e++) {
        if (!explosoes[e].atv) continue;
        int q = explosoes[e].quadro;
        float t = explosoes[e].tam;
        Rectangle destino = { explosoes[e].x - t / 2, explosoes[e].y - t / 2, t, t };
        DrawTexturePro(texExplosao, quadrosExplosao[q / EXPLOSAO_COLUNAS][q % EXPLOSAO_COLUNAS], destino, (Vector2){0, 0}, 0.0f, WHITE);
    }
}

static void FinalizarExplosoes(void) {
    UnloadTexture(texExplosao);
}

#endif
