#include "logique_jeu.h"

static int collision_rect(int x1, int y1, int w1, int h1,
                          int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}

static void configurer_bulle(Bulle *b, TailleBulle taille, BITMAP *sprites[]) {
    b->taille = taille;
    b->sprite = sprites[(int) taille];

    switch (taille) {
        case BULLE_TRES_GRANDE:
            b->gravite = 0.28f;
            b->rebondSol = -16.0f;
            b->attenuationX = 0.995f;
            break;
        case BULLE_GRANDE:
            b->gravite = 0.26f;
            b->rebondSol = -13.0f;
            b->attenuationX = 0.992f;
            break;
        case BULLE_MOYENNE:
            b->gravite = 0.24f;
            b->rebondSol = -10.0f;
            b->attenuationX = 0.989f;
            break;
        case BULLE_PETITE:
            b->gravite = 0.22f;
            b->rebondSol = -7.0f;
            b->attenuationX = 0.985f;
            break;
    }
}

static int trouver_case_libre(Bulle bulles[]) {
    int i;

    for (i = 0; i < MAX_BULLES; i++) {
        if (!bulles[i].active) {
            return i;
        }
    }
    return -1;
}

static float vitesse_horizontale_fille(TailleBulle taille) {
    switch (taille) {
        case BULLE_TRES_GRANDE:
            return 1.8f;
        case BULLE_GRANDE:
            return 2.4f;
        case BULLE_MOYENNE:
            return 3.0f;
        case BULLE_PETITE:
            return 3.8f;
    }
    return 3.0f;
}

static int ajouter_bulle(Bulle bulles[], BITMAP *sprites[],
                         float x, float y, float vx, float vy,
                         TailleBulle taille) {
    int idx;

    idx = trouver_case_libre(bulles);
    if (idx == -1) {
        return -1;
    }

    configurer_bulle(&bulles[idx], taille, sprites);
    bulles[idx].x = x;
    bulles[idx].y = y;
    bulles[idx].vx = vx;
    bulles[idx].vy = vy;
    bulles[idx].active = 1;

    return idx;
}

static void separer_bulle(Bulle bulles[], BITMAP *sprites[], int index) {
    Bulle source;
    TailleBulle nouvelleTaille;
    float centreX;
    float centreY;
    float vxFille;
    BITMAP *spriteFille;

    if (index < 0 || index >= MAX_BULLES || !bulles[index].active) {
        return;
    }

    source = bulles[index];
    if (source.taille == BULLE_PETITE) {
        bulles[index].active = 0;
        return;
    }

    nouvelleTaille = (TailleBulle) (source.taille + 1);
    spriteFille = sprites[(int) nouvelleTaille];
    centreX = source.x + source.sprite->w / 2.0f;
    centreY = source.y + source.sprite->h / 2.0f;
    vxFille = vitesse_horizontale_fille(nouvelleTaille);

    bulles[index].active = 0;

    ajouter_bulle(bulles,
                  sprites,
                  centreX - spriteFille->w / 2.0f - 8,
                  centreY - spriteFille->h / 2.0f,
                  -vxFille,
                  0.0f,
                  nouvelleTaille);
    ajouter_bulle(bulles,
                  sprites,
                  centreX - spriteFille->w / 2.0f + 8,
                  centreY - spriteFille->h / 2.0f,
                  vxFille,
                  0.0f,
                  nouvelleTaille);
}

static void update_bulle(Bulle *b, int groundY) {
    if (!b->active || !b->sprite) {
        return;
    }

    b->vy += b->gravite;
    b->x += b->vx;
    b->y += b->vy;

    if (b->y + b->sprite->h >= groundY) {
        b->y = groundY - b->sprite->h;
        b->vy = b->rebondSol;
        b->vx *= b->attenuationX;

        if (b->vx > 0.0f && b->vx < 1.5f) {
            b->vx = 1.5f;
        }
        if (b->vx < 0.0f && b->vx > -1.5f) {
            b->vx = -1.5f;
        }
    }

    if (b->x < 0) {
        b->x = 0;
        b->vx = -b->vx;
    }
    if (b->x + b->sprite->w > SCREEN_W) {
        b->x = SCREEN_W - b->sprite->w;
        b->vx = -b->vx;
    }
}

static void update_bulles(Bulle bulles[], int groundY) {
    int i;

    for (i = 0; i < MAX_BULLES; i++) {
        update_bulle(&bulles[i], groundY);
    }
}

static int collision_projectile_bulles(Bulle bulles[],
                                       int projX, int projY, int projW, int projH) {
    int i;

    for (i = 0; i < MAX_BULLES; i++) {
        if (!bulles[i].active || !bulles[i].sprite) {
            continue;
        }

        if (collision_rect(projX,
                           projY,
                           projW,
                           projH,
                           (int) bulles[i].x,
                           (int) bulles[i].y,
                           bulles[i].sprite->w,
                           bulles[i].sprite->h)) {
            return i;
        }
    }
    return -1;
}

static int check_player_collision(Bulle bulles[], int px, int py, int pw, int ph) {
    int i;

    for (i = 0; i < MAX_BULLES; i++) {
        if (!bulles[i].active || !bulles[i].sprite) {
            continue;
        }

        if (collision_rect(px,
                           py,
                           pw,
                           ph,
                           (int) bulles[i].x,
                           (int) bulles[i].y,
                           bulles[i].sprite->w,
                           bulles[i].sprite->h)) {
            return 1;
        }
    }
    return 0;
}

static int reste_des_bulles(Bulle bulles[]) {
    int i;

    for (i = 0; i < MAX_BULLES; i++) {
        if (bulles[i].active) {
            return 1;
        }
    }
    return 0;
}

void reassigner_sprites_bulles(EtatJeu *etat, BITMAP *sprites[]) {
    int i;

    if (!etat || !sprites) {
        return;
    }

    for (i = 0; i < MAX_BULLES; i++) {
        if (!etat->bulles[i].active) {
            etat->bulles[i].sprite = NULL;
            continue;
        }
        configurer_bulle(&etat->bulles[i], etat->bulles[i].taille, sprites);
    }
}

int initialiser_logique_jeu(EtatJeu *etat, BITMAP *sprites[], int playerW, int playerH) {
    int i;

    if (!etat || !sprites) {
        return 0;
    }

    etat->groundY = SCREEN_H - 120;
    etat->leftLimit = 0;
    etat->rightLimit = SCREEN_W;
    etat->x = SCREEN_W / 2 - playerW / 2;
    etat->y = etat->groundY - playerH;
    etat->speed = 6;
    etat->projectileActive = 0;
    etat->projectileX = 0;
    etat->projectileY = 0;
    etat->projectileW = 8;
    etat->projectileH = 20;
    etat->projectileSpeed = 12;
    etat->oldUpState = 0;
    etat->oldSaveState = 0;
    etat->oldLoadState = 0;
    etat->perdu = 0;
    etat->gagne = 0;

    for (i = 0; i < MAX_BULLES; i++) {
        etat->bulles[i].active = 0;
        etat->bulles[i].x = 0.0f;
        etat->bulles[i].y = 0.0f;
        etat->bulles[i].vx = 0.0f;
        etat->bulles[i].vy = 0.0f;
        etat->bulles[i].taille = BULLE_PETITE;
        etat->bulles[i].gravite = 0.0f;
        etat->bulles[i].rebondSol = 0.0f;
        etat->bulles[i].attenuationX = 1.0f;
        etat->bulles[i].sprite = NULL;
    }

    ajouter_bulle(etat->bulles,
                  sprites,
                  SCREEN_W - 320.0f,
                  80.0f,
                  -3.2f,
                  0.0f,
                  BULLE_TRES_GRANDE);
    etat->y = etat->groundY - playerH;

    return 1;
}

void mettre_a_jour_logique_jeu(EtatJeu *etat, BITMAP *sprites[], int playerW, int playerH) {
    int idxTouchee;

    if (!etat || !sprites || etat->perdu || etat->gagne) {
        return;
    }

    if (etat->projectileActive) {
        etat->projectileY -= etat->projectileSpeed;
        if (etat->projectileY + etat->projectileH < 0) {
            etat->projectileActive = 0;
        }
    }

    update_bulles(etat->bulles, etat->groundY);

    if (etat->projectileActive) {
        idxTouchee = collision_projectile_bulles(etat->bulles,
                                                 etat->projectileX,
                                                 etat->projectileY,
                                                 etat->projectileW,
                                                 etat->projectileH);
        if (idxTouchee != -1) {
            etat->projectileActive = 0;
            separer_bulle(etat->bulles, sprites, idxTouchee);
        }
    }

    if (check_player_collision(etat->bulles, etat->x, etat->y, playerW, playerH)) {
        etat->perdu = 1;
    }

    if (!reste_des_bulles(etat->bulles)) {
        etat->gagne = 1;
    }
}

void fermer_logique_jeu(EtatJeu *etat) {
    if (!etat) {
        return;
    }

    etat->projectileActive = 0;
}
