#include "balloons.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define BALAO_TAM 200.0f   //lado do quadrado em que o sprite 1000x1000 e desenhado
#define BOSS_TAM  280.0f
#define NUM_TEXTURAS 6     //0 até 4 pra vidas dos balões, boss ainda falta ser definido!!!

//dados de cada tipo de sprite
typedef struct {
    const char *arquivo;
    float tam;                      //tamanho de desenho
    float hbX, hbY, hbW, hbH;
    Color corReserva;               //cor usada se a textura nao carregar
} InfoSprite;

static const InfoSprite SPRITES[NUM_TEXTURAS] = {
    {"Baloes/vermelho.png", BALAO_TAM, 0.353f, 0.263f, 0.335f, 0.435f, RED},
    {"Baloes/azul.png",     BALAO_TAM, 0.353f, 0.263f, 0.335f, 0.435f, BLUE},
    {"Baloes/verde.png",    BALAO_TAM, 0.353f, 0.263f, 0.335f, 0.435f, GREEN},
    {"Baloes/amarelo.png",  BALAO_TAM, 0.353f, 0.263f, 0.335f, 0.435f, YELLOW},
    {"Baloes/rosa.png",     BALAO_TAM, 0.353f, 0.263f, 0.335f, 0.435f, PINK},
    {"Baloes/boss.png",     BOSS_TAM,  0.143f, 0.098f, 0.714f, 0.800f, PURPLE}
};

//velocidade base (px/s) por hp atual (1..5): quanto mais "camadas", mais rapido
static const float VEL_POR_HP[6] = {0, 60, 75, 90, 105, 120};
#define VEL_BOSS 40.0f
#define HP_BOSS 30

static Balao *lista = NULL;
static int total = 0;
static Texture2D tex[NUM_TEXTURAS];
static Rectangle area;          //limites laterais
static float linhaFim;         //y em que o balao chega no jogador
static float timerSpawn = 0;   //contagem para o proximo balao


static int NivelDificuldade(int pontuacao) {
    int n = pontuacao / 100;           //a cada 100 pontos sobe 1 nivel
    return n > 15 ? 15 : n;
}

static int Maximo(int a, int b) { return a > b ? a : b; }
static int Minimo(int a, int b) { return a < b ? a : b; }

//sorteia o tipo por "roleta" e baloes fortes so entram em niveis mais altos
static TiposBalao SortearTipo(int nivel) {
    int peso[5];
    peso[0] = Maximo(10, 100 - 12 * nivel);                      //vermelho
    peso[1] = nivel >= 1 ? Minimo(60, 15 + 8 * nivel) : 0;       //azul
    peso[2] = nivel >= 3 ? Minimo(50, 8 * (nivel - 2)) : 0;      //verde
    peso[3] = nivel >= 5 ? Minimo(40, 7 * (nivel - 4)) : 0;      //amarelo
    peso[4] = nivel >= 7 ? Minimo(30, 6 * (nivel - 6)) : 0;      //rosa

    int soma = 0;
    for (int i = 0; i < 5; i++) soma += peso[i];

    int sorteio = GetRandomValue(1, soma);
    for (int i = 0; i < 5; i++) {
        if (sorteio <= peso[i]) return (TiposBalao)i;
        sorteio -= peso[i];
    }
    return BALLOON_RED;
}
