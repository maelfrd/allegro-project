#ifndef ALLEGRO_TEST_SAUVEGARDE_H
#define ALLEGRO_TEST_SAUVEGARDE_H

#include "logique_jeu.h"

int initialiser_sauvegarde(void);
int sauvegarder_etat_jeu(const EtatJeu *etatJeu, const char *cheminSauvegarde);
int charger_etat_jeu(EtatJeu *etatJeu, const char *cheminSauvegarde);
int charger_pseudo_sauvegarde(const char *cheminSauvegarde, char *pseudoJoueur, int taillePseudoJoueur);
void fermer_sauvegarde(void);

#endif
