#ifndef BOSS_H
#define BOSS_H

#include "raylib.h"

#define BOSS_INTERVALO 500     //a cada 500 de score, o boss aparece
#define BOSS_VIDA_PRIMEIRO 10  //vida do primeiro boss
#define BOSS_TAM 200.0f        //tamanho do boss na tela
#define BOSS_VEL_X 3.0f        //velocidade
#define BOSS_VEL_Y 15.0f       //velocidade de descida (pixels por segundo)
#define BOSS_Y_INICIAL 40.0f   //altura onde o boss aparece
#define BOSS_PONTOS 100        //pontos ganhos ao derrotar o boss
#define BOSS_PISCAR 0.1f       //tempo que o boss fica vermelho ao levar dano

//resultado de AcertarBoss
#define BOSS_ERROU 0
#define BOSS_ACERTOU 1
#define BOSS_MORREU 2

typedef struct {
    float x, y;
    float dir;
    int vida;
    int vidaMax;
    int numero;    //qual boss é
    float piscar;
    bool atv;
} Boss;

static Boss boss;
static Texture2D texBoss;
static Rectangle areaBoss;   //area onde o boss pode andar
static float linhaBoss;      //altura do macaco
static int proximoBoss = BOSS_INTERVALO; //pontuacao em que aparece o proximo boss

//vida de cada boss
static int VidaDoBoss(int numero) {
    if (numero == 1) return BOSS_VIDA_PRIMEIRO;
    return 10 + 5 * numero;
}

//carrega a imagem
static void IniciarBoss(Rectangle area, float linhaMacaco) {
    texBoss = LoadTexture("assets/boss.png");
    GenTextureMipmaps(&texBoss);
    SetTextureFilter(texBoss, TEXTURE_FILTER_TRILINEAR);
    areaBoss = area;
    linhaBoss = linhaMacaco;
    boss = (Boss){0};
    proximoBoss = BOSS_INTERVALO;
}

//zera tudo para uma partida nova
static void ReiniciarBoss(void) {
    boss = (Boss){0};
    proximoBoss = BOSS_INTERVALO;
}

static bool BossAtivo(void) {
    return boss.atv;
}

static Rectangle RetanguloBoss(void) {
    return (Rectangle){ boss.x, boss.y, BOSS_TAM, BOSS_TAM };
}

//se atingir o score necessario, ativa o boss
static bool ChecarBoss(int pontuacao) {
    if (boss.atv || pontuacao < proximoBoss) return false;

    boss.numero = proximoBoss / BOSS_INTERVALO;
    boss.vidaMax = VidaDoBoss(boss.numero);
    boss.vida = boss.vidaMax;
    boss.x = areaBoss.x + (areaBoss.width - BOSS_TAM) / 2;
    boss.y = BOSS_Y_INICIAL;
    boss.dir = 1;
    boss.piscar = 0;
    boss.atv = true;

    proximoBoss += BOSS_INTERVALO;
    return true;
}

//move o boss e devolve quantas vidas o macaco perdeu
static int AttBoss(float dt) {
    if (!boss.atv) return 0;

    if (boss.piscar > 0) boss.piscar -= dt;

    //anda de um lado pro outro
    boss.x += boss.dir * BOSS_VEL_X;
    if (boss.x < areaBoss.x) {
        boss.x = areaBoss.x;
        boss.dir = 1;
    }
    if (boss.x > areaBoss.x + areaBoss.width - BOSS_TAM) {
        boss.x = areaBoss.x + areaBoss.width - BOSS_TAM;
        boss.dir = -1;
    }

    //desce devagar. se chegar no macaco, tira uma vida e sobe de novo
    boss.y += BOSS_VEL_Y * dt;
    if (boss.y + BOSS_TAM >= linhaBoss) {
        boss.y = BOSS_Y_INICIAL;
        return 1;
    }
    return 0;
}

//testa se o dardo acertou o boss
static int AcertarBoss(Rectangle hit) {
    if (!boss.atv) return BOSS_ERROU;

    //hitbox
    float margem = BOSS_TAM * 0.15f;
    Rectangle corpo = { boss.x + margem, boss.y + margem, BOSS_TAM - 2 * margem, BOSS_TAM - 2 * margem };
    if (!CheckCollisionRecs(hit, corpo)) return BOSS_ERROU;

    boss.vida--;
    boss.piscar = BOSS_PISCAR;
    if (boss.vida <= 0) {
        boss.atv = false;
        return BOSS_MORREU;
    }
    return BOSS_ACERTOU;
}

//desenha o boss e a barra de vida em cima dele
static void DrawBoss(void) {
    if (!boss.atv) return;

    Color cor = (boss.piscar > 0) ? RED : WHITE; //pisca vermelho ao levar dano
    Rectangle origem = { 0, 0, (float)texBoss.width, (float)texBoss.height };
    DrawTexturePro(texBoss, origem, RetanguloBoss(), (Vector2){0, 0}, 0.0f, cor);

    //barra de vida
    int barraX = (int)boss.x;
    int barraY = (int)boss.y - 16;
    int barraW = (int)BOSS_TAM;
    int preenchido = barraW * boss.vida / boss.vidaMax;
    DrawRectangle(barraX, barraY, barraW, 10, Fade(BLACK, 0.6f));
    DrawRectangle(barraX, barraY, preenchido, 10, RED);
    DrawRectangleLines(barraX, barraY, barraW, 10, WHITE);

    const char *nome = TextFormat("BOSS %d", boss.numero);
    DrawText(nome, barraX + (barraW - MeasureText(nome, 16)) / 2, barraY - 18, 16, WHITE);
}

static void FinalizarBoss(void) {
    UnloadTexture(texBoss);
}

#endif