#ifndef ALLEGRO_TEST_SAUVEGARDE_H
#define ALLEGRO_TEST_SAUVEGARDE_H

#include "logique_jeu.h"

int initialiser_sauvegarde(void);
int sauvegarder_etat_jeu(const EtatJeu *etat, const char *chemin);
int charger_etat_jeu(EtatJeu *etat, const char *chemin);
void fermer_sauvegarde(void);

#endif
