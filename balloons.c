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
