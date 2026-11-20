#ifndef UPGRADES_H
#define UPGRADES_H
#include "raylib.h"
#include <stddef.h> //NULL

#define ABATES_POR_UPGRADE 7 //baloes estourados para ganhar um upgrade

//interior do segundo quadrado do painel direito
#define UPG_X 1100
#define UPG_Y 500
#define UPG_W 225

//tipos de upgrade (UPG_TOTAL = quantidade de tipos, serve de tamanho para os vetores)
typedef enum {
    UPG_DARDO,
    UPG_VEL_DARDO,
    UPG_VEL_MOV,
    UPG_TOTAL
} Upgrade;

static const int NIVEL_MAX[UPG_TOTAL] = { 4, 5, 5 }; //dardo: ate quintuplo (4 extras); velocidades: ate nivel 5
static const char *NOME_UPG[UPG_TOTAL] = { "DARDOS", "VEL. TIRO", "VEL. MOVER" };

static int nivel[UPG_TOTAL]; //nivel atual de cada upgrade (acumulativo)
static int abates = 0;       //baloes estourados desde o ultimo upgrade

static void ReiniciarUpgrades(void) {
    for (int u = 0; u < UPG_TOTAL; u++) nivel[u] = 0;
    abates = 0;
}

//conta um balao estourado; a cada 7 sorteia um upgrade que ainda nao chegou no maximo
//devolve o texto do aviso, ou NULL se nao ganhou nada agora
static const char *ContarAbate(void) {
    if (++abates < ABATES_POR_UPGRADE) return NULL;
    abates = 0;

    int disponiveis[UPG_TOTAL], n = 0;
    for (int i = 0; i < UPG_TOTAL; i++) {
        if (nivel[i] < NIVEL_MAX[i]) disponiveis[n++] = i;
    }
    if (n == 0) return NULL;

    int tipo = disponiveis[GetRandomValue(0, n - 1)];
    nivel[tipo]++;
    if (tipo == UPG_VEL_DARDO) return "DARDO MAIS RAPIDO!";
    if (tipo == UPG_VEL_MOV) return "MACACO MAIS RAPIDO!";
    static const char *DARDOS[] = { "", "DARDO DUPLO!", "DARDO TRIPLO!", "DARDO QUADRUPLO!", "DARDO QUINTUPLO!" };
    return DARDOS[nivel[tipo]];
}

//desenha o painel de upgrades dentro do segundo quadrado
static void DesenharPainelUpgrades(void) {
    int x0 = UPG_X + 8, larg = UPG_W - 16, y0 = UPG_Y + 6;

    DrawText("UPGRADES", UPG_X + (UPG_W - MeasureText("UPGRADES", 14)) / 2, y0, 14, GOLD);
    y0 += 22;

    for (int i = 0; i < UPG_TOTAL; i++) {
        //no dardo mostra a quantidade atual; nos outros, so o nome
        DrawText(i == UPG_DARDO ? TextFormat("DARDOS x%d", 1 + nivel[i]) : NOME_UPG[i], x0, y0, 14, WHITE);

        //um quadradinho por nivel: dourado = ja ganhou, apagado = falta ganhar
        for (int k = 0; k < NIVEL_MAX[i]; k++) {
            DrawRectangle(x0 + k * 12, y0 + 17, 8, 8, k < nivel[i] ? GOLD : Fade(WHITE, 0.2f));
        }
        y0 += 32;
    }

    //barra de progresso ate o proximo upgrade
    DrawRectangle(x0, y0 + 4, larg, 10, Fade(BLACK, 0.5f));
    DrawRectangle(x0, y0 + 4, larg * abates / ABATES_POR_UPGRADE, 10, SKYBLUE);
    DrawRectangleLines(x0, y0 + 4, larg, 10, WHITE);
}

#endif
