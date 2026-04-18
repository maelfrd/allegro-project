#ifndef ALLEGRO_TEST_IHM_H
#define ALLEGRO_TEST_IHM_H

#include "logique_jeu.h"

typedef struct {
    int quitter;
    int sauvegarder;
    int charger;
} ActionsIHM;

void traiter_ihm(EtatJeu *etat, const BITMAP *player, ActionsIHM *actions);

#endif
