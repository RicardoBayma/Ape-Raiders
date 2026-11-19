#ifndef ESTOURO_H
#define ESTOURO_H
#include "raylib.h"
#include <stdbool.h>
#include <stddef.h> //NULL

#define ESTOURO_QTD 8 //quantos podem tocar ao mesmo tempo
#define ESTOURO_VOLUME 0.2f //volume

static Sound somEstouro; //som original
static Sound qtdEstouro[ESTOURO_QTD]; //copias do som
static int proximoCanal = 0;
static bool somCarregado = false; //fica false se nao tiver placa de som ou o arquivo nao abrir

//liga o audio e carrega o som
static void IniciarEstouro(void) {
    InitAudioDevice();
    somEstouro = LoadSound("assets/estouro.wav");
    somCarregado = IsAudioDeviceReady() && somEstouro.stream.buffer != NULL;
    if (!somCarregado) return; //sem som o jogo continua normal, so fica mudo

    for (int i = 0; i < ESTOURO_QTD; i++) {
        qtdEstouro[i] = LoadSoundAlias(somEstouro);
        SetSoundVolume(qtdEstouro[i], ESTOURO_VOLUME);
    }
}

//toca o estouro
static void TocarEstouro(void) {
    if (!somCarregado) return;
    PlaySound(qtdEstouro[proximoCanal]);
    proximoCanal = (proximoCanal + 1) % ESTOURO_QTD;
}

//limpa o som e desliga o audio
static void FinalizarEstouro(void) {
    if (somCarregado) {
        for (int i = 0; i < ESTOURO_QTD; i++) UnloadSoundAlias(qtdEstouro[i]);
    }
    UnloadSound(somEstouro);
    CloseAudioDevice();
}

#endif