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
    {"assets/vermelho.png", BALAO_TAM, 0.353f, 0.263f, 0.335f, 0.435f, RED},
    {"assets/azul.png",     BALAO_TAM, 0.353f, 0.263f, 0.335f, 0.435f, BLUE},
    {"assets/verde.png",    BALAO_TAM, 0.353f, 0.263f, 0.335f, 0.435f, GREEN},
    {"assets/amarelo.png",  BALAO_TAM, 0.353f, 0.263f, 0.335f, 0.435f, YELLOW},
    {"assets/rosa.png",     BALAO_TAM, 0.353f, 0.263f, 0.335f, 0.435f, PINK},
    {"assets/boss.png",     BOSS_TAM,  0.143f, 0.098f, 0.714f, 0.800f, PURPLE}
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

static int IndiceSprite(const Balao *b) {
    if (b->tipo == BOSS) return 5;
    int idx = b->hp - 1;               //balao muda de cor ao levar dano
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    return idx;
}

static float VelocidadeBase(const Balao *b) {
    return b->tipo == BOSS ? VEL_BOSS : VEL_POR_HP[b->hp];
}


void IniciarBalao(Rectangle areaJogo, float linhaChegada) {
    area = areaJogo;
    linhaFim = linhaChegada;
    SetRandomSeed((unsigned int)time(NULL));

    for (int i = 0; i < NUM_TEXTURAS; i++) {
        tex[i] = LoadTexture(SPRITES[i].arquivo);
        if (tex[i].id != 0) {
            GenTextureMipmaps(&tex[i]);
            SetTextureFilter(tex[i], TEXTURE_FILTER_TRILINEAR);
        }
    }
    lista = NULL;
    total = 0;
    timerSpawn = 1.0f;      //pequena pausa antes do 1o balao
}

void SpawnBalao(float x, float y, TiposBalao tipo) {
    if (total >= MAX_BALLOONS) return;

    Balao *b = (Balao *)malloc(sizeof(Balao));
    if (b == NULL) return;

    int idx = (tipo == BOSS) ? 5 : (int)tipo;
    const InfoSprite *s = &SPRITES[idx];

    b->tipo = tipo;
    b->maxHp = (tipo == BOSS) ? HP_BOSS : (int)tipo + 1;
    b->hp = b->maxHp;
    b->w = s->hbW * s->tam;
    b->h = s->hbH * s->tam;
    b->amp = (float)GetRandomValue(10, 30);
    b->offset = (float)GetRandomValue(0, 628) / 100.0f;
    b->tempoVida = 0;
    b->ativo = true;
    b->speed = VelocidadeBase(b);

    //garante que o balao incluindo o balanco fique dentro da praia
    float minX = area.x + b->amp;
    float maxX = area.x + area.width - b->w - b->amp;
    if (x < minX) x = minX;
    if (x > maxX) x = maxX;
    b->baseX = x;
    b->x = x;
    b->y = y;

    b->prox = lista;
    lista = b;
    total++;
}

int AttBalao(float dt, int pontuacao) {
    int nivel = NivelDificuldade(pontuacao);
    float multVel = 1.0f + 0.08f * nivel;                     //+8% de velocidade por nivel, podemos mudar isso depois se quisermos
    float intervalo = 1.6f - 0.09f * nivel;                   //spawn cada vez mais rapido
    if (intervalo < 0.45f) intervalo = 0.45f;

    //spawn nasce logo acima da tela, em x aleatorio dentro da praia
    timerSpawn -= dt;
    if (timerSpawn <= 0) {
        timerSpawn += intervalo;
        TiposBalao tipo = SortearTipo(nivel);
        const InfoSprite *s = &SPRITES[(int)tipo];
        float larg = s->hbW * s->tam;
        float altura = s->hbH * s->tam;
        float x = (float)GetRandomValue((int)area.x, (int)(area.x + area.width - larg));
        SpawnBalao(x, -altura, tipo);
    }

    int chegaram = 0;
    Balao **pp = &lista;
    while (*pp != NULL) {
        Balao *b = *pp;
        b->tempoVida += dt;
        b->y += b->speed * multVel * dt;
        b->x = b->baseX + sinf(b->tempoVida * 2.0f + b->offset) * b->amp;


        if (b->x < area.x) b->x = area.x;
        if (b->x + b->w > area.x + area.width) b->x = area.x + area.width - b->w;

        bool remover = false;
        if (!b->ativo) {
            remover = true;                                   //estourado
        } else if (b->y + b->h >= linhaFim) {
            chegaram++;                                           //chegou no jogador
            remover = true;
        }

        if (remover) {
            *pp = b->prox;
            free(b);
            total--;
        } else {
            pp = &b->prox;
        }
    }
    return chegaram;
}

void DrawBalao(void) {
    for (Balao *b = lista; b != NULL; b = b->prox) {
        if (!b->ativo) continue;                              //estourado nesse frame nao desenha
        int idx = IndiceSprite(b);
        const InfoSprite *s = &SPRITES[idx];

        if (tex[idx].id == 0) {                               //se o sprite n carregar ele desenha uma elipse
            DrawEllipse((int)(b->x + b->w / 2), (int)(b->y + b->h / 2),
                        b->w / 2, b->h / 2, s->corReserva);
            continue;
        }
        Rectangle origem = {0, 0, (float)tex[idx].width, (float)tex[idx].height};
        Rectangle destino = {b->x - s->hbX * s->tam, b->y - s->hbY * s->tam, s->tam, s->tam};
        DrawTexturePro(tex[idx], origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
    }
}

void ReiniciarBaloes(void) {
    while (lista != NULL) {
        Balao *prox = lista->prox;
        free(lista);
        lista = prox;
    }
    total = 0;
    timerSpawn = 1.0f;
}

void FinalizarBalao(void) {
    ReiniciarBaloes();
    for (int i = 0; i < NUM_TEXTURAS; i++) UnloadTexture(tex[i]);
}


Balao *BalaoEmRetangulo(Rectangle r) {
    for (Balao *b = lista; b != NULL; b = b->prox) {
        if (!b->ativo) continue;
        Rectangle hit = {b->x, b->y, b->w, b->h};
        if (CheckCollisionRecs(r, hit)) return b;
    }
    return NULL;
}

int DanoBalao(Balao *b, int dano) {
    if (b == NULL || !b->ativo) return 0;
    b->hp -= dano;
    if (b->hp <= 0) {
        b->ativo = false;                                     //sera removido no proximo AttBalao
        return b->maxHp * 10;                                 //10 pontos no score por "camada" do balao
    }
    b->speed = VelocidadeBase(b);                             //perdeu camada: muda de cor e fica mais lento
    return 0;
}

Balao *BalaoMaisBaixo(void) {
    Balao *mais = NULL;
    for (Balao *b = lista; b != NULL; b = b->prox) {
        if (b->ativo && (mais == NULL || b->y > mais->y)) mais = b;
    }
    return mais;
}

int ContarBaloes(void) { return total; }
