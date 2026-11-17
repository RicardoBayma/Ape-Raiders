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
