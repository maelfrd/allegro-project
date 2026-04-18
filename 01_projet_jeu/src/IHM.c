#include "IHM.h"

#include <allegro.h>

void traiter_ihm(EtatJeu *etat, const BITMAP *player, ActionsIHM *actions) {
    if (!etat || !player || !actions) {
        return;
    }

    actions->quitter = key[KEY_ESC];
    actions->sauvegarder = 0;
    actions->charger = 0;

    if (!etat->perdu && !etat->gagne) {
        if (key[KEY_LEFT]) {
            etat->x -= etat->speed;
        }
        if (key[KEY_RIGHT]) {
            etat->x += etat->speed;
        }

        if (etat->x < etat->leftLimit) {
            etat->x = etat->leftLimit;
        }
        if (etat->x + player->w > etat->rightLimit) {
            etat->x = etat->rightLimit - player->w;
        }

        etat->y = etat->groundY - player->h;

        if (key[KEY_UP] && !etat->oldUpState && !etat->projectileActive) {
            etat->projectileActive = 1;
            etat->projectileX = etat->x + player->w / 2 - etat->projectileW / 2;
            etat->projectileY = etat->y;
        }
    }

    if (key[KEY_S] && !etat->oldSaveState) {
        actions->sauvegarder = 1;
    }
    if (key[KEY_L] && !etat->oldLoadState) {
        actions->charger = 1;
    }

    etat->oldUpState = key[KEY_UP];
    etat->oldSaveState = key[KEY_S];
    etat->oldLoadState = key[KEY_L];
}
