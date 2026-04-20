#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include <allegro.h>

#include "logique_jeu.h"

#define NOMBRE_FONDS_NIVEAUX 5

typedef struct {
    BITMAP *fond;
    BITMAP *fondsNiveaux[NOMBRE_FONDS_NIVEAUX];
    BITMAP *player;
    BITMAP *tir;
    BITMAP *feu;
    BITMAP *chapeau;
    BITMAP *explosion;
    BITMAP *sprites[BULLE_TAILLES_TOTAL];
    BITMAP *spritesVifDor[BULLE_TAILLES_TOTAL];
    BITMAP *spritesVolDeMort[BULLE_TAILLES_TOTAL];
    BITMAP *buffer;
} RessourcesJeu;

int initialiser_affichage(const char *fond_path,
                          int largeur_defaut,
                          int hauteur_defaut,
                          int profondeur_couleur);
int ressource_existe(const char *chemin);
BITMAP *charger_bitmap_ou_erreur(const char *chemin);
BITMAP *resize_bitmap(BITMAP *src, float scale);
int charger_ressources_jeu(RessourcesJeu *ressources,
                           const char *fond_path,
                           const char *player_path,
                           const char *mangemort_path,
                           const char *vifdor_path,
                           const char *tir_path,
                           const char *feu_path,
                           const char *chapeau_path,
                           const char *explosion_path);
void dessiner_menu_depart(const RessourcesJeu *ressources, int selection, int repriseDisponible);
void dessiner_menu_parametres(const RessourcesJeu *ressources,
                              int selection,
                              int modeDemonstrationActif);
void dessiner_ecran_information(const RessourcesJeu *ressources,
                                const char *titre,
                                const char *ligne1,
                                const char *ligne2,
                                const char *ligne3);
void dessiner_saisie_pseudo(const RessourcesJeu *ressources, const char *pseudo);
void dessiner_decompte_depart(const RessourcesJeu *ressources, const EtatJeu *etat, int valeur);
void dessiner_jeu(const RessourcesJeu *ressources, const EtatJeu *etat);
void liberer_bitmap(BITMAP **bitmap);
void liberer_ressources_jeu(RessourcesJeu *ressources);
void fermer_affichage(void);

#endif
