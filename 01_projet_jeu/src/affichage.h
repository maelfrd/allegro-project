#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include <allegro.h>

#include "logique_jeu.h"

typedef struct {
    BITMAP *fond;
    BITMAP *player;
    BITMAP *sprites[4];
    BITMAP *buffer;
} RessourcesJeu;

int initialiser_affichage(int largeur, int hauteur, int profondeur_couleur);
BITMAP *charger_bitmap_ou_erreur(const char *chemin);
BITMAP *resize_bitmap(BITMAP *src, float scale);
int charger_ressources_jeu(RessourcesJeu *ressources,
                           const char *fond_path,
                           const char *player_path,
                           const char *bulle_path);
void dessiner_jeu(const RessourcesJeu *ressources, const EtatJeu *etat);
void liberer_bitmap(BITMAP **bitmap);
void liberer_ressources_jeu(RessourcesJeu *ressources);
void fermer_affichage(void);

#endif
