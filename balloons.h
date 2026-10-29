#ifndef BALLOONS_H
#define BALLOONS_H
#include <stdbool.h>

#define MAX_BALLOONS 25 //definicao inicial de qtd max, editavel baseado nas necessidades

typedef enum{
    BALLOON_RED,
    BALLOON_BLUE,
    BALLOON_GREEN,
    BALLOON_YELLOW,
    BALLOON_PINK,
    BOSS
} TiposBalao; //definicao dos tipos de inimigos, balao e boss

typedef struct{
    float x, y; //coordenadas do balao
    float speed, offset; //velocidade de movimentacao vertical (speed) e horizontal (offset) que faz eles mexerem alternadamente
    int hp, maxHp; //vida atual do balao(hp) e quantos hits para chegar ao total da vida dele(maxHp)
    bool ativo;//se o balao existe ou nao

    TiposBalao tipo;//identificar qual dos tipos de balao o atual é
} Balao;

void IniciarBalao(void);
void SpawnBalao(float x, float y, TiposBalao tipo);
void AttBalao(float deltaTempo);
void DrawBalao(void);

#endif