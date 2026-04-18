#include "sauvegarde.h"

#include <stdio.h>

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    int active;
    int taille;
    float gravite;
    float rebondSol;
    float attenuationX;
} BulleSauvegardee;

typedef struct {
    BulleSauvegardee bulles[MAX_BULLES];
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
} EtatJeuSauvegarde;

static void copier_vers_sauvegarde(const EtatJeu *source, EtatJeuSauvegarde *destination) {
    int i;

    destination->groundY = source->groundY;
    destination->leftLimit = source->leftLimit;
    destination->rightLimit = source->rightLimit;
    destination->x = source->x;
    destination->y = source->y;
    destination->speed = source->speed;
    destination->projectileActive = source->projectileActive;
    destination->projectileX = source->projectileX;
    destination->projectileY = source->projectileY;
    destination->projectileW = source->projectileW;
    destination->projectileH = source->projectileH;
    destination->projectileSpeed = source->projectileSpeed;
    destination->oldUpState = source->oldUpState;
    destination->oldSaveState = source->oldSaveState;
    destination->oldLoadState = source->oldLoadState;
    destination->perdu = source->perdu;
    destination->gagne = source->gagne;

    for (i = 0; i < MAX_BULLES; i++) {
        destination->bulles[i].x = source->bulles[i].x;
        destination->bulles[i].y = source->bulles[i].y;
        destination->bulles[i].vx = source->bulles[i].vx;
        destination->bulles[i].vy = source->bulles[i].vy;
        destination->bulles[i].active = source->bulles[i].active;
        destination->bulles[i].taille = (int) source->bulles[i].taille;
        destination->bulles[i].gravite = source->bulles[i].gravite;
        destination->bulles[i].rebondSol = source->bulles[i].rebondSol;
        destination->bulles[i].attenuationX = source->bulles[i].attenuationX;
    }
}

static void copier_depuis_sauvegarde(const EtatJeuSauvegarde *source, EtatJeu *destination) {
    int i;

    destination->groundY = source->groundY;
    destination->leftLimit = source->leftLimit;
    destination->rightLimit = source->rightLimit;
    destination->x = source->x;
    destination->y = source->y;
    destination->speed = source->speed;
    destination->projectileActive = source->projectileActive;
    destination->projectileX = source->projectileX;
    destination->projectileY = source->projectileY;
    destination->projectileW = source->projectileW;
    destination->projectileH = source->projectileH;
    destination->projectileSpeed = source->projectileSpeed;
    destination->oldUpState = source->oldUpState;
    destination->oldSaveState = source->oldSaveState;
    destination->oldLoadState = source->oldLoadState;
    destination->perdu = source->perdu;
    destination->gagne = source->gagne;

    for (i = 0; i < MAX_BULLES; i++) {
        destination->bulles[i].x = source->bulles[i].x;
        destination->bulles[i].y = source->bulles[i].y;
        destination->bulles[i].vx = source->bulles[i].vx;
        destination->bulles[i].vy = source->bulles[i].vy;
        destination->bulles[i].active = source->bulles[i].active;
        destination->bulles[i].taille = (TailleBulle) source->bulles[i].taille;
        destination->bulles[i].gravite = source->bulles[i].gravite;
        destination->bulles[i].rebondSol = source->bulles[i].rebondSol;
        destination->bulles[i].attenuationX = source->bulles[i].attenuationX;
        destination->bulles[i].sprite = NULL;
    }
}

int initialiser_sauvegarde(void) {
    return 1;
}

int sauvegarder_etat_jeu(const EtatJeu *etat, const char *chemin) {
    EtatJeuSauvegarde sauvegarde;
    FILE *fichier;

    if (!etat || !chemin || chemin[0] == '\0') {
        return 0;
    }

    copier_vers_sauvegarde(etat, &sauvegarde);

    fichier = fopen(chemin, "wb");
    if (!fichier) {
        return 0;
    }

    if (fwrite(&sauvegarde, sizeof(sauvegarde), 1, fichier) != 1) {
        fclose(fichier);
        return 0;
    }

    fclose(fichier);
    return 1;
}

int charger_etat_jeu(EtatJeu *etat, BITMAP *sprites[], const char *chemin) {
    EtatJeuSauvegarde sauvegarde;
    FILE *fichier;

    if (!etat || !sprites || !chemin || chemin[0] == '\0') {
        return 0;
    }

    fichier = fopen(chemin, "rb");
    if (!fichier) {
        return 0;
    }

    if (fread(&sauvegarde, sizeof(sauvegarde), 1, fichier) != 1) {
        fclose(fichier);
        return 0;
    }

    fclose(fichier);

    copier_depuis_sauvegarde(&sauvegarde, etat);
    reassigner_sprites_bulles(etat, sprites);

    return 1;
}

void fermer_sauvegarde(void) {
}
