#ifndef BALLOONS_H
#define BALLOONS_H
#include <stdbool.h>
#include "raylib.h"

#define MAX_BALLOONS 25

typedef enum{
    BALLOON_RED,
    BALLOON_BLUE,
    BALLOON_GREEN,
    BALLOON_YELLOW,
    BALLOON_PINK,
    BOSS
} TiposBalao; //tipos de balao

//informacoes dos baloes, vida, hitbox, velocidade, tempo etc
typedef struct{
    float x, y;        
    float w, h;
    float baseX;
    float amp;
    float speed, offset;
    float tempoVida;
    int hp, maxHp;
    bool ativo;

    TiposBalao tipo;

    struct Balao *prox;
} Balao;

//area do jogo
void IniciarBalao(Rectangle areaJogo, float linhaChegada);

//cria um balao na posicao x,y
void SpawnBalao(float x, float y, TiposBalao tipo);

int AttBalao(float deltaTempo, int pontuacao);

void DrawBalao(void);

void ReiniciarBaloes(void); //zera o timer e listas
void FinalizarBalao(void);  //limpa textura

Balao *BalaoEmRetangulo(Rectangle r);
int DanoBalao(Balao *b, int dano);    //tira vida
Balao *BalaoMaisBaixo(void);          //balao mais proximo do jogador
int ContarBaloes(void);               //quantidade de baloes vivos

#endif
