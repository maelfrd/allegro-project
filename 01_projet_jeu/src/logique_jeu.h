#ifndef LOGIQUE_JEU_H
#define LOGIQUE_JEU_H

#include <allegro.h>

#define MAX_BULLES 32

typedef enum {
    BULLE_TRES_GRANDE = 0,
    BULLE_GRANDE,
    BULLE_MOYENNE,
    BULLE_PETITE
} TailleBulle;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    int active;
    TailleBulle taille;
    float gravite;
    float rebondSol;
    float attenuationX;
    BITMAP *sprite;
} Bulle;

typedef struct {
    Bulle bulles[MAX_BULLES];
    int groundY;
    int leftLimit;
    int rightLimit;
    int x;
    int y;
    int speed;
    int projectileActive;
    int projectileX;
    int projectileY;
    int projectileW;
    int projectileH;
    int projectileSpeed;
    int oldUpState;
    int oldSaveState;
    int oldLoadState;
    int perdu;
    int gagne;
} EtatJeu;

int initialiser_logique_jeu(EtatJeu *etat, BITMAP *sprites[], int playerW, int playerH);
void mettre_a_jour_logique_jeu(EtatJeu *etat, BITMAP *sprites[], int playerW, int playerH);
void reassigner_sprites_bulles(EtatJeu *etat, BITMAP *sprites[]);
void fermer_logique_jeu(EtatJeu *etat);

#endif
