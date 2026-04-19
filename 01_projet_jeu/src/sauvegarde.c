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
    int vitesse;
    int tirActif;
    int tirX;
    int tirY;
    int tirLargeur;
    int tirHauteur;
    int vitesseTir;
    int bonusVisible;
    int bonusX;
    int bonusY;
    int bonusLargeur;
    int bonusHauteur;
    float bonusVx;
    float bonusVy;
    int explosionActive;
    int explosionX;
    int explosionY;
    int explosionLargeur;
    int explosionHauteur;
    int explosionTimer;
    int typeBonusQuiTombe;
    int typeBonusActif;
    int tempsBonusActifMs;
    int tempsRestantMs;
    int prochainDeclenchementEclairMs;
    int nbEclairs;
    int bossTouchesAvantReduction;
    int perdu;
    int gagne;
    char pseudo[TAILLE_PSEUDO_MAX];
} EtatJeuSauvegarde;

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
    int chapeauVisible;
    int chapeauX;
    int chapeauY;
    int chapeauW;
    int chapeauH;
    float chapeauVx;
    float chapeauVy;
    int explosionActive;
    int explosionX;
    int explosionY;
    int explosionW;
    int explosionH;
    int explosionTimer;
    int modeFeuActif;
    int perdu;
    int gagne;
    char pseudo[TAILLE_PSEUDO_MAX];
} EtatJeuSauvegardeLegacy;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    int type;
    int taille;
    float gravite;
    float rebondSol;
    float attenuationX;
    int largeur;
    int hauteur;
} BulleSauvegardee;

typedef struct {
    float x;
    float y;
    float vitesseY;
    int largeur;
    int hauteur;
} EclairSauvegarde;

static void copier_vers_sauvegarde(const EtatJeu *source, EtatJeuSauvegarde *destination) {
    destination->nbBulles = source->nbBulles;
    destination->niveau = source->niveau;
    destination->niveauMaximum = source->niveauMaximum;
    if (destination->niveau < 1) {
        destination->niveau = 1;
    }
    if (destination->niveauMaximum < 1) {
        destination->niveauMaximum = 5;
    }
    if (destination->niveau > destination->niveauMaximum) {
        destination->niveau = destination->niveauMaximum;
    }
    destination->groundY = source->groundY;
    destination->leftLimit = source->leftLimit;
    destination->rightLimit = source->rightLimit;
    destination->x = source->x;
    destination->y = source->y;
    destination->vitesse = source->vitesse;
    destination->tirActif = source->tirActif;
    destination->tirX = source->tirX;
    destination->tirY = source->tirY;
    destination->tirLargeur = source->tirLargeur;
    destination->tirHauteur = source->tirHauteur;
    destination->vitesseTir = source->vitesseTir;
    destination->bonusVisible = source->bonusVisible;
    destination->bonusX = source->bonusX;
    destination->bonusY = source->bonusY;
    destination->bonusLargeur = source->bonusLargeur;
    destination->bonusHauteur = source->bonusHauteur;
    destination->bonusVx = source->bonusVx;
    destination->bonusVy = source->bonusVy;
    destination->explosionActive = source->explosionActive;
    destination->explosionX = source->explosionX;
    destination->explosionY = source->explosionY;
    destination->explosionLargeur = source->explosionLargeur;
    destination->explosionHauteur = source->explosionHauteur;
    destination->explosionTimer = source->explosionTimer;
    destination->typeBonusQuiTombe = source->typeBonusQuiTombe;
    destination->typeBonusActif = source->typeBonusActif;
    destination->tempsBonusActifMs = source->tempsBonusActifMs;
    destination->tempsRestantMs = source->tempsRestantMs;
    destination->prochainDeclenchementEclairMs = source->prochainDeclenchementEclairMs;
    destination->nbEclairs = source->nbEclairs;
    destination->bossTouchesAvantReduction = source->bossTouchesAvantReduction;
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
        destination[i].type = (int) source->bulles[i].type;
        destination[i].taille = (int) source->bulles[i].taille;
        destination[i].gravite = source->bulles[i].gravite;
        destination[i].rebondSol = source->bulles[i].rebondSol;
        destination[i].attenuationX = source->bulles[i].attenuationX;
        destination[i].largeur = source->bulles[i].largeur;
        destination[i].hauteur = source->bulles[i].hauteur;
    }
}

static void copier_eclairs_vers_sauvegarde(const EtatJeu *source, EclairSauvegarde *destination) {
    int i;

    for (i = 0; i < source->nbEclairs; i++) {
        destination[i].x = source->eclairs[i].x;
        destination[i].y = source->eclairs[i].y;
        destination[i].vitesseY = source->eclairs[i].vitesseY;
        destination[i].largeur = source->eclairs[i].largeur;
        destination[i].hauteur = source->eclairs[i].hauteur;
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
                                     const EclairSauvegarde *eclairsSauvegardes,
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
    destination->vitesse = source->vitesse;
    destination->tirActif = source->tirActif;
    destination->tirX = source->tirX;
    destination->tirY = source->tirY;
    destination->tirLargeur = source->tirLargeur;
    destination->tirHauteur = source->tirHauteur;
    destination->vitesseTir = source->vitesseTir;
    destination->tirSecondaireActif = 0;
    destination->tirSecondaireX = 0;
    destination->tirSecondaireY = 0;
    destination->bonusVisible = source->bonusVisible;
    destination->bonusX = source->bonusX;
    destination->bonusY = source->bonusY;
    destination->bonusLargeur = source->bonusLargeur;
    destination->bonusHauteur = source->bonusHauteur;
    destination->bonusVx = source->bonusVx;
    destination->bonusVy = source->bonusVy;
    destination->explosionActive = source->explosionActive;
    destination->explosionX = source->explosionX;
    destination->explosionY = source->explosionY;
    destination->explosionLargeur = source->explosionLargeur;
    destination->explosionHauteur = source->explosionHauteur;
    destination->explosionTimer = source->explosionTimer;
    destination->typeBonusQuiTombe = source->typeBonusQuiTombe;
    destination->typeBonusActif = source->typeBonusActif;
    destination->tempsBonusActifMs = source->tempsBonusActifMs;
    destination->tempsRestantMs = source->tempsRestantMs;
    destination->prochainDeclenchementEclairMs = source->prochainDeclenchementEclairMs;
    destination->nbEclairs = source->nbEclairs;
    destination->bossTouchesAvantReduction = source->bossTouchesAvantReduction;
    destination->perdu = source->perdu;
    destination->gagne = source->gagne;
    strncpy(destination->pseudo, source->pseudo, TAILLE_PSEUDO_MAX);
    destination->pseudo[TAILLE_PSEUDO_MAX - 1] = '\0';

    for (i = 0; i < source->nbBulles; i++) {
        destination->bulles[i].x = bullesSauvegardees[i].x;
        destination->bulles[i].y = bullesSauvegardees[i].y;
        destination->bulles[i].vx = bullesSauvegardees[i].vx;
        destination->bulles[i].vy = bullesSauvegardees[i].vy;
        destination->bulles[i].type = (TypeEntite) bullesSauvegardees[i].type;
        destination->bulles[i].taille = (TailleBulle) bullesSauvegardees[i].taille;
        destination->bulles[i].gravite = bullesSauvegardees[i].gravite;
        destination->bulles[i].rebondSol = bullesSauvegardees[i].rebondSol;
        destination->bulles[i].attenuationX = bullesSauvegardees[i].attenuationX;
        destination->bulles[i].largeur = bullesSauvegardees[i].largeur;
        destination->bulles[i].hauteur = bullesSauvegardees[i].hauteur;
    }

    for (i = 0; i < source->nbEclairs && i < MAX_ECLAIRS; i++) {
        destination->eclairs[i].x = eclairsSauvegardes[i].x;
        destination->eclairs[i].y = eclairsSauvegardes[i].y;
        destination->eclairs[i].vitesseY = eclairsSauvegardes[i].vitesseY;
        destination->eclairs[i].largeur = eclairsSauvegardes[i].largeur;
        destination->eclairs[i].hauteur = eclairsSauvegardes[i].hauteur;
    }
}

static void copier_depuis_sauvegarde_legacy(const EtatJeuSauvegardeLegacy *source,
                                            const BulleSauvegardee *bullesSauvegardees,
                                            EtatJeu *destination) {
    int i;

    destination->nbBulles = source->nbBulles;
    destination->niveau = source->niveau;
    destination->niveauMaximum = source->niveauMaximum < 1 ? 5 : source->niveauMaximum;
    destination->groundY = source->groundY;
    destination->leftLimit = source->leftLimit;
    destination->rightLimit = source->rightLimit;
    destination->x = source->x;
    destination->y = source->y;
    destination->vitesse = source->speed;
    destination->tirActif = source->projectileActive;
    destination->tirX = source->projectileX;
    destination->tirY = source->projectileY;
    destination->tirLargeur = source->projectileW;
    destination->tirHauteur = source->projectileH;
    destination->vitesseTir = source->projectileSpeed;
    destination->tirSecondaireActif = 0;
    destination->tirSecondaireX = 0;
    destination->tirSecondaireY = 0;
    destination->bonusVisible = source->chapeauVisible;
    destination->bonusX = source->chapeauX;
    destination->bonusY = source->chapeauY;
    destination->bonusLargeur = source->chapeauW;
    destination->bonusHauteur = source->chapeauH;
    destination->bonusVx = source->chapeauVx;
    destination->bonusVy = source->chapeauVy;
    destination->explosionActive = source->explosionActive;
    destination->explosionX = source->explosionX;
    destination->explosionY = source->explosionY;
    destination->explosionLargeur = source->explosionW;
    destination->explosionHauteur = source->explosionH;
    destination->explosionTimer = source->explosionTimer;
    destination->typeBonusQuiTombe = source->chapeauVisible ? BONUS_CHAPEAU : BONUS_AUCUN;
    destination->typeBonusActif = source->modeFeuActif ? BONUS_LANCE_FLAMMES : BONUS_AUCUN;
    destination->tempsBonusActifMs = source->modeFeuActif ? DUREE_POUVOIR_MS : 0;
    destination->tempsRestantMs = DUREE_NIVEAU_MS;
    destination->prochainDeclenchementEclairMs = DUREE_NIVEAU_MS - INTERVALLE_ECLAIR_MS;
    destination->nbEclairs = 0;
    destination->bossTouchesAvantReduction = destination->niveau == 5 ? 5 : 0;
    destination->score = 0;
    destination->perdu = source->perdu;
    destination->gagne = source->gagne;
    strncpy(destination->pseudo, source->pseudo, TAILLE_PSEUDO_MAX);
    destination->pseudo[TAILLE_PSEUDO_MAX - 1] = '\0';

    for (i = 0; i < source->nbBulles; i++) {
        destination->bulles[i].x = bullesSauvegardees[i].x;
        destination->bulles[i].y = bullesSauvegardees[i].y;
        destination->bulles[i].vx = bullesSauvegardees[i].vx;
        destination->bulles[i].vy = bullesSauvegardees[i].vy;
        destination->bulles[i].type = (TypeEntite) bullesSauvegardees[i].type;
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
    EclairSauvegarde *eclairsSauvegardes;
    FILE *fichier;

    if (!etat || !chemin || chemin[0] == '\0') {
        return 0;
    }

    copier_vers_sauvegarde(etat, &sauvegarde);
    bullesSauvegardees = NULL;
    eclairsSauvegardes = NULL;

    if (etat->nbBulles > 0) {
        bullesSauvegardees = (BulleSauvegardee *) malloc((size_t) etat->nbBulles * sizeof(BulleSauvegardee));
        if (!bullesSauvegardees) {
            return 0;
        }
        copier_bulles_vers_sauvegarde(etat, bullesSauvegardees);
    }

    if (etat->nbEclairs > 0) {
        eclairsSauvegardes = (EclairSauvegarde *) malloc((size_t) etat->nbEclairs * sizeof(EclairSauvegarde));
        if (!eclairsSauvegardes) {
            free(bullesSauvegardees);
            return 0;
        }
        copier_eclairs_vers_sauvegarde(etat, eclairsSauvegardes);
    }

    fichier = fopen(chemin, "wb");
    if (!fichier) {
        free(bullesSauvegardees);
        free(eclairsSauvegardes);
        return 0;
    }

    if (fwrite(&sauvegarde, sizeof(sauvegarde), 1, fichier) != 1) {
        fclose(fichier);
        free(bullesSauvegardees);
        free(eclairsSauvegardes);
        return 0;
    }

    if (etat->nbBulles > 0 &&
        fwrite(bullesSauvegardees, sizeof(BulleSauvegardee), (size_t) etat->nbBulles, fichier) != (size_t) etat->nbBulles) {
        fclose(fichier);
        free(bullesSauvegardees);
        free(eclairsSauvegardes);
        return 0;
    }

    if (etat->nbEclairs > 0 &&
        fwrite(eclairsSauvegardes, sizeof(EclairSauvegarde), (size_t) etat->nbEclairs, fichier) != (size_t) etat->nbEclairs) {
        fclose(fichier);
        free(bullesSauvegardees);
        free(eclairsSauvegardes);
        return 0;
    }

    if (fwrite(&etat->score, sizeof(etat->score), 1, fichier) != 1) {
        fclose(fichier);
        free(bullesSauvegardees);
        free(eclairsSauvegardes);
        return 0;
    }

    fclose(fichier);
    free(bullesSauvegardees);
    free(eclairsSauvegardes);
    return 1;
}

int charger_etat_jeu(EtatJeu *etat, const char *chemin) {
    EtatJeuSauvegarde sauvegarde;
    EtatJeuSauvegardeLegacy sauvegardeLegacy;
    BulleSauvegardee *bullesSauvegardees;
    EclairSauvegarde *eclairsSauvegardes;
    FILE *fichier;
    int formatLegacy;
    int scoreCharge;

    if (!etat || !chemin || chemin[0] == '\0') {
        return 0;
    }

    fichier = fopen(chemin, "rb");
    if (!fichier) {
        return 0;
    }

    formatLegacy = 0;
    scoreCharge = 0;
    if (fread(&sauvegarde, sizeof(sauvegarde), 1, fichier) != 1) {
        rewind(fichier);
        if (fread(&sauvegardeLegacy, sizeof(sauvegardeLegacy), 1, fichier) != 1) {
            fclose(fichier);
            return 0;
        }
        formatLegacy = 1;
    }

    bullesSauvegardees = NULL;
    eclairsSauvegardes = NULL;
    if ((formatLegacy ? sauvegardeLegacy.nbBulles : sauvegarde.nbBulles) > 0) {
        int nbBulles = formatLegacy ? sauvegardeLegacy.nbBulles : sauvegarde.nbBulles;

        bullesSauvegardees = (BulleSauvegardee *) malloc((size_t) nbBulles * sizeof(BulleSauvegardee));
        if (!bullesSauvegardees) {
            fclose(fichier);
            return 0;
        }

        if (fread(bullesSauvegardees,
                  sizeof(BulleSauvegardee),
                  (size_t) nbBulles,
                  fichier) != (size_t) nbBulles) {
            fclose(fichier);
            free(bullesSauvegardees);
            return 0;
        }
    }

    if (!formatLegacy && sauvegarde.nbEclairs > 0) {
        if (sauvegarde.nbEclairs > MAX_ECLAIRS) {
            fclose(fichier);
            free(bullesSauvegardees);
            return 0;
        }

        eclairsSauvegardes = (EclairSauvegarde *) malloc((size_t) sauvegarde.nbEclairs * sizeof(EclairSauvegarde));
        if (!eclairsSauvegardes) {
            fclose(fichier);
            free(bullesSauvegardees);
            return 0;
        }

        if (fread(eclairsSauvegardes,
                  sizeof(EclairSauvegarde),
                  (size_t) sauvegarde.nbEclairs,
                  fichier) != (size_t) sauvegarde.nbEclairs) {
            fclose(fichier);
            free(bullesSauvegardees);
            free(eclairsSauvegardes);
            return 0;
        }
    }

    if (!formatLegacy) {
        int scoreSauvegarde;

        if (fread(&scoreSauvegarde, sizeof(scoreSauvegarde), 1, fichier) == 1) {
            scoreCharge = scoreSauvegarde;
        }
    }

    fclose(fichier);

    if (!reserver_depuis_sauvegarde(etat,
                                    (formatLegacy ? sauvegardeLegacy.nbBulles : sauvegarde.nbBulles) > 0
                                            ? (formatLegacy ? sauvegardeLegacy.nbBulles : sauvegarde.nbBulles)
                                            : 4)) {
        free(bullesSauvegardees);
        free(eclairsSauvegardes);
        return 0;
    }

    if (formatLegacy) {
        copier_depuis_sauvegarde_legacy(&sauvegardeLegacy, bullesSauvegardees, etat);
    } else {
        copier_depuis_sauvegarde(&sauvegarde, bullesSauvegardees, eclairsSauvegardes, etat);
        etat->score = scoreCharge;
    }
    if (etat->score < 0) {
        etat->score = 0;
    }
    free(bullesSauvegardees);
    free(eclairsSauvegardes);
    return 1;
}

void fermer_sauvegarde(void) {
}
