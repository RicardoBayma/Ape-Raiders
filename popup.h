#ifndef POPUP_H
#define POPUP_H
#include "raylib.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

//pop-up da tela de derrota e o jogador digita o nome pro ranking

#define NOME_MAX 15                        //maximo de caracteres do nome, por enquanto e 15
#define NOME_DIGITAVEL 30                  //deixa digitar alem do maximo para poder avisar "Nome muito longo"
#define NOME_BYTES (NOME_DIGITAVEL * 2 + 1) //letras com acento ocupam 2 bytes

//resultado de AtualizarPopupNome
#define POPUP_NADA 0
#define POPUP_SALVOU 1
#define POPUP_PULOU 2

//posicao e tamanho da placa
#define POP_W 640
#define POP_H 300
#define POP_X ((1376 - POP_W) / 2)
#define POP_Y 250

//partes de dentro da placa
#define POP_CAIXA_X (POP_X + 45)
#define POP_CAIXA_Y (POP_Y + 105)
#define POP_CAIXA_W 370
#define POP_CAIXA_H 54
#define POP_BOTAO_X (POP_CAIXA_X + POP_CAIXA_W + 20)
#define POP_BOTAO_W 160
#define POP_BOTAO_H 54

static bool popupAberto = false;
static char popupTexto[NOME_BYTES];
static const char *popupErro = NULL; //mensagem de erro

static Rectangle BotaoSalvar(void) { return (Rectangle){ POP_BOTAO_X, POP_CAIXA_Y, POP_BOTAO_W, POP_BOTAO_H }; }
static Rectangle BotaoPular(void)  { return (Rectangle){ POP_BOTAO_X, POP_CAIXA_Y + POP_BOTAO_H + 16, POP_BOTAO_W, POP_BOTAO_H }; }

static int ContarLetras(const char *s) {
    int n = 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (((unsigned char)s[i] & 0xC0) != 0x80) n++;
    }
    return n;
}

//abre o pop-up com a caixa vazia
static void AbrirPopupNome(void) {
    popupAberto = true;
    popupTexto[0] = '\0';
    popupErro = NULL;
}

static bool PopupNomeAberto(void) { return popupAberto; }

//nome digitado sem espacos no comeco e no fim
static const char *NomeDigitado(void) {
    static char limpo[NOME_BYTES];
    const char *ini = popupTexto;
    while (*ini == ' ') ini++;
    strcpy(limpo, ini);
    int fim = (int)strlen(limpo);
    while (fim > 0 && limpo[fim - 1] == ' ') limpo[--fim] = '\0';
    return limpo;
}

//tenta salvar e confere o tamanho do nome antes
static int TentarSalvar(void) {
    int letras = ContarLetras(NomeDigitado());
    if (letras == 0) {
        popupErro = "Digite um nome";
        return POPUP_NADA;
    }
    if (letras > NOME_MAX) {
        popupErro = "Nome muito longo";
        return POPUP_NADA;
    }
    popupAberto = false;
    return POPUP_SALVOU;
}

//le o teclado e o mouse
static int AtualizarPopupNome(void) {
    if (!popupAberto) return POPUP_NADA;

    //letras digitadas
    int c = GetCharPressed();
    while (c > 0) {
        if (c >= 32 && c <= 255 && ContarLetras(popupTexto) < NOME_DIGITAVEL) {
            int tam = 0;
            const char *utf8 = CodepointToUTF8(c, &tam);
            int len = (int)strlen(popupTexto);
            memcpy(popupTexto + len, utf8, tam);
            popupTexto[len + tam] = '\0';
            popupErro = NULL; //mudou o texto: some o aviso
        }
        c = GetCharPressed();
    }

    bool apagar = IsKeyPressedRepeat(KEY_BACKSPACE); //segurando o backspace apaga rapido
    bool enter = false;
    int tecla = GetKeyPressed();
    while (tecla > 0) {
        if (tecla == KEY_BACKSPACE) apagar = true;
        if (tecla == KEY_ENTER || tecla == KEY_KP_ENTER) enter = true;
        tecla = GetKeyPressed();
    }

    //apaga a ultima letra
    if (apagar) {
        int len = (int)strlen(popupTexto);
        while (len > 0 && ((unsigned char)popupTexto[len - 1] & 0xC0) == 0x80) len--; //bytes de continuacao
        if (len > 0) len--;
        popupTexto[len] = '\0';
        popupErro = NULL;
    }

    //enter tambem salva
    if (enter) return TentarSalvar();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, BotaoSalvar())) return TentarSalvar();
        if (CheckCollisionPointRec(m, BotaoPular())) {
            popupAberto = false;
            return POPUP_PULOU;
        }
    }
    return POPUP_NADA;
}

static void TextoSombra(const char *t, int x, int y, int tam, Color cor) {
    DrawText(t, x + 2, y + 2, tam, (Color){ 40, 20, 8, 255 });
    DrawText(t, x, y, tam, cor);
}

//corda, tá bugada, a gente tenta ajeitar se possivel
static void DesenharCorda(Vector2 a, Vector2 b) {
    DrawLineEx(a, b, 9, (Color){ 120, 85, 45, 255 });
    DrawLineEx(a, b, 5, (Color){ 196, 152, 92, 255 });
    float dx = b.x - a.x, dy = b.y - a.y;
    int passos = 12;
    for (int i = 1; i < passos; i++) {
        float t = (float)i / passos;
        Vector2 p = { a.x + dx * t, a.y + dy * t };
        DrawLineEx((Vector2){ p.x - 4, p.y - 2 }, (Vector2){ p.x + 4, p.y + 3 }, 2, (Color){ 120, 85, 45, 255 });
    }
}

//triangulo
static void Triangulo(Vector2 a, Vector2 b, Vector2 c, Color cor) {
    DrawTriangle(a, b, c, cor);
    DrawTriangle(a, c, b, cor);
}

//folha
static void DesenharFolha(Vector2 base, float angulo, float comp, float larg) {
    float dx = cosf(angulo), dy = sinf(angulo);
    float px = -dy, py = dx;
    Color contorno = { 22, 80, 30, 255 };
    Color lado1 = { 70, 165, 60, 255 };
    Color lado2 = { 110, 200, 80, 255 };
    const int partes = 10;

    for (int camada = 0; camada < 2; camada++) {
        float extra = (camada == 0) ? 3.0f : 0.0f;
        for (int k = 0; k < partes; k++) {
            float t0 = (float)k / partes, t1 = (float)(k + 1) / partes;
            float w0 = sinf(t0 * PI) * larg + extra, w1 = sinf(t1 * PI) * larg + extra;
            Vector2 c0 = { base.x + dx * comp * t0, base.y + dy * comp * t0 };
            Vector2 c1 = { base.x + dx * comp * t1, base.y + dy * comp * t1 };
            Vector2 e0 = { c0.x + px * w0, c0.y + py * w0 }, e1 = { c1.x + px * w1, c1.y + py * w1 };
            Vector2 d0 = { c0.x - px * w0, c0.y - py * w0 }, d1 = { c1.x - px * w1, c1.y - py * w1 };
            Color a = (camada == 0) ? contorno : lado1;
            Color b = (camada == 0) ? contorno : lado2;
            Triangulo(c0, e0, e1, a);
            Triangulo(c0, e1, c1, a);
            Triangulo(c0, d0, d1, b);
            Triangulo(c0, d1, c1, b);
        }
    }
    DrawLineEx(base, (Vector2){ base.x + dx * comp * 0.9f, base.y + dy * comp * 0.9f }, 2, contorno);
    DrawLineEx((Vector2){ base.x - dx * 10, base.y - dy * 10 }, base, 3, contorno);
}

//botao de madeira mais clara (acende com o mouse em cima)
static void DesenharBotao(Rectangle r, const char *texto) {
    bool emCima = CheckCollisionPointRec(GetMousePosition(), r);
    Color base = emCima ? (Color){ 222, 160, 82, 255 } : (Color){ 196, 132, 62, 255 };
    DrawRectangleRec((Rectangle){ r.x + 4, r.y + 4, r.width, r.height }, (Color){ 50, 28, 12, 160 }); //sombra
    DrawRectangleRec(r, base);
    DrawRectangle((int)r.x, (int)r.y, (int)r.width, 6, Fade(WHITE, 0.25f));                           //brilho de cima
    DrawRectangle((int)r.x, (int)(r.y + r.height - 6), (int)r.width, 6, Fade(BLACK, 0.2f));
    DrawRectangleLinesEx(r, 3, (Color){ 60, 34, 16, 255 });
    int tam = 28;
    int w = MeasureText(texto, tam);
    TextoSombra(texto, (int)(r.x + (r.width - w) / 2), (int)(r.y + (r.height - tam) / 2), tam, emCima ? GOLD : WHITE);
}

static void DesenharPopupNome(void) {
    if (!popupAberto) return;

    DrawRectangle(0, 0, 1376, 768, Fade(BLACK, 0.55f)); //escurece o fundo

    //cordas presas no alto da tela
    DesenharCorda((Vector2){ POP_X + 120, POP_Y - 140 }, (Vector2){ POP_X + 90, POP_Y + 18 });
    DesenharCorda((Vector2){ POP_X + POP_W - 120, POP_Y - 140 }, (Vector2){ POP_X + POP_W - 90, POP_Y + 18 });

    //placa: sombra, 3 tabuas e contorno
    DrawRectangle(POP_X + 8, POP_Y + 10, POP_W, POP_H, Fade(BLACK, 0.45f));
    Color tabua[3] = { { 168, 108, 58, 255 }, { 158, 100, 52, 255 }, { 164, 104, 55, 255 } };
    int alturaTabua = POP_H / 3;
    for (int i = 0; i < 3; i++) {
        int ty = POP_Y + i * alturaTabua;
        DrawRectangle(POP_X, ty, POP_W, alturaTabua, tabua[i]);
        DrawRectangle(POP_X, ty, POP_W, 4, (Color){ 192, 132, 74, 255 });                  //luz no topo da tabua
        DrawRectangle(POP_X, ty + alturaTabua - 4, POP_W, 4, (Color){ 112, 66, 32, 255 });
    }
    //veios da madeira
    static const int veios[][3] = {
        { 40, 30, 90 }, { 210, 18, 120 }, { 470, 40, 110 }, { 90, 75, 70 },
        { 300, 140, 100 }, { 520, 120, 80 }, { 60, 245, 120 }, { 380, 265, 140 }, { 560, 215, 50 }
    };
    for (int i = 0; i < (int)(sizeof(veios) / sizeof(veios[0])); i++) {
        DrawRectangle(POP_X + veios[i][0], POP_Y + veios[i][1], veios[i][2], 3, (Color){ 136, 84, 42, 255 });
    }
    DrawRectangleLinesEx((Rectangle){ POP_X, POP_Y, POP_W, POP_H }, 5, (Color){ 64, 36, 16, 255 });

    //nos das cordas na placa
    DrawCircle(POP_X + 90, POP_Y + 18, 9, (Color){ 120, 85, 45, 255 });
    DrawCircle(POP_X + 90, POP_Y + 18, 6, (Color){ 196, 152, 92, 255 });
    DrawCircle(POP_X + POP_W - 90, POP_Y + 18, 9, (Color){ 120, 85, 45, 255 });
    DrawCircle(POP_X + POP_W - 90, POP_Y + 18, 6, (Color){ 196, 152, 92, 255 });

    //titulo
    const char *titulo = "Nome para salvar:";
    int tamTitulo = 36;
    TextoSombra(titulo, POP_X + (POP_W - MeasureText(titulo, tamTitulo)) / 2, POP_Y + 40, tamTitulo, GOLD);

    //caixa de digitacao
    Rectangle caixa = { POP_CAIXA_X, POP_CAIXA_Y, POP_CAIXA_W, POP_CAIXA_H };
    DrawRectangleRec(caixa, (Color){ 62, 34, 14, 255 });
    DrawRectangleLinesEx(caixa, 3, popupErro ? RED : (Color){ 222, 170, 100, 255 });

    //se o texto nao couber mostra so o final dele
    int tamTexto = 28;
    const char *visivel = popupTexto;
    while (*visivel && MeasureText(visivel, tamTexto) > POP_CAIXA_W - 30) {
        visivel++;
        while (((unsigned char)*visivel & 0xC0) == 0x80) visivel++; //nao corta letra com acento no meio
    }
    int textoX = POP_CAIXA_X + 12;
    int textoY = POP_CAIXA_Y + (POP_CAIXA_H - tamTexto) / 2;
    DrawText(visivel, textoX, textoY, tamTexto, WHITE);

    //cursor piscando
    if ((int)(GetTime() * 2) % 2 == 0) {
        int cx = textoX + MeasureText(visivel, tamTexto) + 3;
        DrawRectangle(cx, textoY, 3, tamTexto, WHITE);
    }

    //contador de letras fica vermelho se passar de 15
    int letras = ContarLetras(popupTexto);
    const char *contador = TextFormat("%d/%d", letras, NOME_MAX);
    int tamContador = 20;
    TextoSombra(contador, POP_CAIXA_X + POP_CAIXA_W - MeasureText(contador, tamContador), POP_CAIXA_Y + POP_CAIXA_H + 8,
                tamContador, letras > NOME_MAX ? RED : (Color){ 245, 225, 190, 255 });

    //mensagem de erro embaixo da caixa
    if (popupErro != NULL) {
        TextoSombra(popupErro, POP_CAIXA_X, POP_CAIXA_Y + POP_CAIXA_H + 30, 26, (Color){ 255, 70, 60, 255 });
    }

    DesenharBotao(BotaoSalvar(), "Salvar");
    DesenharBotao(BotaoPular(), "Pular");

    //folhas no canto de baixo da placa
    Vector2 raiz = { POP_X + 6, POP_Y + POP_H - 30 };
    DesenharFolha(raiz, 3.6f, 70, 18);
    DesenharFolha(raiz, 2.5f, 64, 16);
    DesenharFolha(raiz, 1.75f, 56, 15);
    DesenharFolha((Vector2){ POP_X + 40, POP_Y + POP_H - 8 }, 1.2f, 50, 13);
    DesenharFolha((Vector2){ POP_X + 2, POP_Y + 40 }, 3.9f, 46, 12);
}

#endif
