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
