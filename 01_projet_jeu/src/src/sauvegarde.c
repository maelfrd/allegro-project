#include "sauvegarde.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int nbBulles;
    int niveau;
    int niveauMaximum;
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
    int perdu;
    int gagne;
    char pseudo[TAILLE_PSEUDO_MAX];
} EtatJeuSauvegarde;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    int taille;
    float gravite;
    float rebondSol;
    float attenuationX;
    int largeur;
    int hauteur;
} BulleSauvegardee;

static void copier_vers_sauvegarde(const EtatJeu *source, EtatJeuSauvegarde *destination) {
    destination->nbBulles = source->nbBulles;
    destination->niveau = source->niveau;
    destination->niveauMaximum = source->niveauMaximum;
    if (destination->niveau < 1) {
        destination->niveau = 1;
    }
    if (destination->niveauMaximum < 1) {
        destination->niveauMaximum = 4;
    }
    if (destination->niveau > destination->niveauMaximum) {
        destination->niveau = destination->niveauMaximum;
    }
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
    destination->perdu = source->perdu;
    destination->gagne = source->gagne;
    strncpy(destination->pseudo, source->pseudo, TAILLE_PSEUDO_MAX);
}

static void copier_bulles_vers_sauvegarde(const EtatJeu *source, BulleSauvegardee *destination) {
    int i;

    for (i = 0; i < source->nbBulles; i++) {
        destination[i].x = source->bulles[i].x;
        destination[i].y = source->bulles[i].y;
        destination[i].vx = source->bulles[i].vx;
        destination[i].vy = source->bulles[i].vy;
        destination[i].taille = (int) source->bulles[i].taille;
        destination[i].gravite = source->bulles[i].gravite;
        destination[i].rebondSol = source->bulles[i].rebondSol;
        destination[i].attenuationX = source->bulles[i].attenuationX;
        destination[i].largeur = source->bulles[i].largeur;
        destination[i].hauteur = source->bulles[i].hauteur;
    }
}

static int reserver_depuis_sauvegarde(EtatJeu *etat, int capaciteVoulue) {
    Bulle *nouvellesBulles;

    if (capaciteVoulue <= etat->capaciteBulles) {
        return 1;
    }

    nouvellesBulles = (Bulle *) realloc(etat->bulles, (size_t) capaciteVoulue * sizeof(Bulle));
    if (!nouvellesBulles) {
        return 0;
    }

    etat->bulles = nouvellesBulles;
    etat->capaciteBulles = capaciteVoulue;
    return 1;
}

static void copier_depuis_sauvegarde(const EtatJeuSauvegarde *source,
                                     const BulleSauvegardee *bullesSauvegardees,
                                     EtatJeu *destination) {
    int i;

    destination->nbBulles = source->nbBulles;
    destination->niveau = source->niveau;
    destination->niveauMaximum = source->niveauMaximum;
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
    destination->perdu = source->perdu;
    destination->gagne = source->gagne;
    strncpy(destination->pseudo, source->pseudo, TAILLE_PSEUDO_MAX);
    destination->pseudo[TAILLE_PSEUDO_MAX - 1] = '\0';

    for (i = 0; i < source->nbBulles; i++) {
        destination->bulles[i].x = bullesSauvegardees[i].x;
        destination->bulles[i].y = bullesSauvegardees[i].y;
        destination->bulles[i].vx = bullesSauvegardees[i].vx;
        destination->bulles[i].vy = bullesSauvegardees[i].vy;
        destination->bulles[i].taille = (TailleBulle) bullesSauvegardees[i].taille;
        destination->bulles[i].gravite = bullesSauvegardees[i].gravite;
        destination->bulles[i].rebondSol = bullesSauvegardees[i].rebondSol;
        destination->bulles[i].attenuationX = bullesSauvegardees[i].attenuationX;
        destination->bulles[i].largeur = bullesSauvegardees[i].largeur;
        destination->bulles[i].hauteur = bullesSauvegardees[i].hauteur;
    }
}

int initialiser_sauvegarde(void) {
    return 1;
}

int sauvegarder_etat_jeu(const EtatJeu *etat, const char *chemin) {
    EtatJeuSauvegarde sauvegarde;
    BulleSauvegardee *bullesSauvegardees;
    FILE *fichier;

    if (!etat || !chemin || chemin[0] == '\0') {
        return 0;
    }

    copier_vers_sauvegarde(etat, &sauvegarde);
    bullesSauvegardees = NULL;

    if (etat->nbBulles > 0) {
        bullesSauvegardees = (BulleSauvegardee *) malloc((size_t) etat->nbBulles * sizeof(BulleSauvegardee));
        if (!bullesSauvegardees) {
            return 0;
        }
        copier_bulles_vers_sauvegarde(etat, bullesSauvegardees);
    }

    fichier = fopen(chemin, "wb");
    if (!fichier) {
        free(bullesSauvegardees);
        return 0;
    }

    if (fwrite(&sauvegarde, sizeof(sauvegarde), 1, fichier) != 1) {
        fclose(fichier);
        free(bullesSauvegardees);
        return 0;
    }

    if (etat->nbBulles > 0 &&
        fwrite(bullesSauvegardees, sizeof(BulleSauvegardee), (size_t) etat->nbBulles, fichier) != (size_t) etat->nbBulles) {
        fclose(fichier);
        free(bullesSauvegardees);
        return 0;
    }

    fclose(fichier);
    free(bullesSauvegardees);
    return 1;
}

int charger_etat_jeu(EtatJeu *etat, const char *chemin) {
    EtatJeuSauvegarde sauvegarde;
    BulleSauvegardee *bullesSauvegardees;
    FILE *fichier;

    if (!etat || !chemin || chemin[0] == '\0') {
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

    bullesSauvegardees = NULL;
    if (sauvegarde.nbBulles > 0) {
        bullesSauvegardees = (BulleSauvegardee *) malloc((size_t) sauvegarde.nbBulles * sizeof(BulleSauvegardee));
        if (!bullesSauvegardees) {
            fclose(fichier);
            return 0;
        }

        if (fread(bullesSauvegardees,
                  sizeof(BulleSauvegardee),
                  (size_t) sauvegarde.nbBulles,
                  fichier) != (size_t) sauvegarde.nbBulles) {
            fclose(fichier);
            free(bullesSauvegardees);
            return 0;
        }
    }

    fclose(fichier);

    if (!reserver_depuis_sauvegarde(etat, sauvegarde.nbBulles > 0 ? sauvegarde.nbBulles : 4)) {
        free(bullesSauvegardees);
        return 0;
    }

    copier_depuis_sauvegarde(&sauvegarde, bullesSauvegardees, etat);
    free(bullesSauvegardees);
    return 1;
}

void fermer_sauvegarde(void) {
}
