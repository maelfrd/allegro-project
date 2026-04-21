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
    int auraArdenteActive;
    int dureeRestanteAuraArdenteMs;
    int perdu;
    int gagne;
    int score;
    int tempsRestantNiveauMs;
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
    int auraArdenteActive;
    int dureeRestanteAuraArdenteMs;
    int perdu;
    int gagne;
    int score;
    char pseudo[TAILLE_PSEUDO_MAX];
} EtatJeuSauvegardeV1;

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
    int nombreCoupsAvantDivision;
} BulleSauvegardee;

static void copier_vers_sauvegarde(const EtatJeu *source, EtatJeuSauvegarde *destination) {
    destination->nbBulles = source->nbBulles;
    destination->niveau = source->niveau;
    destination->niveauMaximum = source->niveauMaximum;
    if (destination->niveau < PREMIER_NIVEAU_JEU) {
        destination->niveau = PREMIER_NIVEAU_JEU;
    }
    if (destination->niveauMaximum < PREMIER_NIVEAU_JEU) {
        destination->niveauMaximum = NIVEAU_MAXIMUM_JEU;
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
    destination->chapeauVisible = source->chapeauVisible;
    destination->chapeauX = source->chapeauX;
    destination->chapeauY = source->chapeauY;
    destination->chapeauW = source->chapeauW;
    destination->chapeauH = source->chapeauH;
    destination->chapeauVx = source->chapeauVx;
    destination->chapeauVy = source->chapeauVy;
    destination->explosionActive = source->explosionActive;
    destination->explosionX = source->explosionX;
    destination->explosionY = source->explosionY;
    destination->explosionW = source->explosionW;
    destination->explosionH = source->explosionH;
    destination->explosionTimer = source->explosionTimer;
    destination->auraArdenteActive = source->auraArdenteActive;
    destination->dureeRestanteAuraArdenteMs = source->dureeRestanteAuraArdenteMs;
    destination->perdu = source->perdu;
    destination->gagne = source->gagne;
    destination->score = source->score;
    destination->tempsRestantNiveauMs = source->tempsRestantNiveauMs;
    strncpy(destination->pseudo, source->pseudo, TAILLE_PSEUDO_MAX);
}

static void convertir_sauvegarde_v1(const EtatJeuSauvegardeV1 *source, EtatJeuSauvegarde *destination) {
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
    destination->chapeauVisible = source->chapeauVisible;
    destination->chapeauX = source->chapeauX;
    destination->chapeauY = source->chapeauY;
    destination->chapeauW = source->chapeauW;
    destination->chapeauH = source->chapeauH;
    destination->chapeauVx = source->chapeauVx;
    destination->chapeauVy = source->chapeauVy;
    destination->explosionActive = source->explosionActive;
    destination->explosionX = source->explosionX;
    destination->explosionY = source->explosionY;
    destination->explosionW = source->explosionW;
    destination->explosionH = source->explosionH;
    destination->explosionTimer = source->explosionTimer;
    destination->auraArdenteActive = source->auraArdenteActive;
    destination->dureeRestanteAuraArdenteMs = source->dureeRestanteAuraArdenteMs;
    destination->perdu = source->perdu;
    destination->gagne = source->gagne;
    destination->score = source->score;
    destination->tempsRestantNiveauMs = DUREE_NIVEAU_SECONDES * VALEUR_MILLISECONDES_SECONDE;
    strncpy(destination->pseudo, source->pseudo, TAILLE_PSEUDO_MAX);
}

static void copier_bulles_vers_sauvegarde(const EtatJeu *source, BulleSauvegardee *destination) {
    int i;

    for (i = INDEX_PREMIER; i < source->nbBulles; i++) {
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
        destination[i].nombreCoupsAvantDivision = source->bulles[i].nombreCoupsAvantDivision;
    }
}

static int reserver_depuis_sauvegarde(EtatJeu *etat, int capaciteVoulue) {
    Bulle *nouvellesBulles;

    if (capaciteVoulue <= etat->capaciteBulles) {
        return VRAI;
    }

    nouvellesBulles = (Bulle *) realloc(etat->bulles, (size_t) capaciteVoulue * sizeof(Bulle));
    if (!nouvellesBulles) {
        return FAUX;
    }

    etat->bulles = nouvellesBulles;
    etat->capaciteBulles = capaciteVoulue;
    return VRAI;
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
    destination->chapeauVisible = source->chapeauVisible;
    destination->chapeauX = source->chapeauX;
    destination->chapeauY = source->chapeauY;
    destination->chapeauW = source->chapeauW;
    destination->chapeauH = source->chapeauH;
    destination->chapeauVx = source->chapeauVx;
    destination->chapeauVy = source->chapeauVy;
    destination->explosionActive = source->explosionActive;
    destination->explosionX = source->explosionX;
    destination->explosionY = source->explosionY;
    destination->explosionW = source->explosionW;
    destination->explosionH = source->explosionH;
    destination->explosionTimer = source->explosionTimer;
    destination->auraArdenteActive = source->auraArdenteActive;
    destination->dureeRestanteAuraArdenteMs = source->dureeRestanteAuraArdenteMs;
    destination->perdu = source->perdu;
    destination->gagne = source->gagne;
    destination->score = source->score;
    destination->tempsRestantNiveauMs = source->tempsRestantNiveauMs;
    if (destination->tempsRestantNiveauMs <= VALEUR_NULLE) {
        destination->tempsRestantNiveauMs = DUREE_NIVEAU_SECONDES * VALEUR_MILLISECONDES_SECONDE;
    }
    strncpy(destination->pseudo, source->pseudo, TAILLE_PSEUDO_MAX);
    destination->pseudo[TAILLE_PSEUDO_MAX - INDEX_SUIVANT] = CARACTERE_FIN_CHAINE;

    for (i = INDEX_PREMIER; i < source->nbBulles; i++) {
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
        destination->bulles[i].nombreCoupsAvantDivision = bullesSauvegardees[i].nombreCoupsAvantDivision;
    }
}

int initialiser_sauvegarde(void) {
    return VRAI;
}

int sauvegarder_etat_jeu(const EtatJeu *etat, const char *chemin) {
    EtatJeuSauvegarde sauvegarde;
    BulleSauvegardee *bullesSauvegardees;
    FILE *fichier;

    if (!etat || !chemin || chemin[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE) {
        return FAUX;
    }

    copier_vers_sauvegarde(etat, &sauvegarde);
    bullesSauvegardees = NULL;

    if (etat->nbBulles > VALEUR_NULLE) {
        bullesSauvegardees = (BulleSauvegardee *) malloc((size_t) etat->nbBulles * sizeof(BulleSauvegardee));
        if (!bullesSauvegardees) {
            return FAUX;
        }
        copier_bulles_vers_sauvegarde(etat, bullesSauvegardees);
    }

    fichier = fopen(chemin, "wb");
    if (!fichier) {
        free(bullesSauvegardees);
        return FAUX;
    }

    if (fwrite(&sauvegarde, sizeof(sauvegarde), VALEUR_UNITAIRE, fichier) != VALEUR_UNITAIRE) {
        fclose(fichier);
        free(bullesSauvegardees);
        return FAUX;
    }

    if (etat->nbBulles > VALEUR_NULLE &&
        fwrite(bullesSauvegardees, sizeof(BulleSauvegardee), (size_t) etat->nbBulles, fichier) != (size_t) etat->nbBulles) {
        fclose(fichier);
        free(bullesSauvegardees);
        return FAUX;
    }

    fclose(fichier);
    free(bullesSauvegardees);
    return VRAI;
}

int charger_etat_jeu(EtatJeu *etat, const char *chemin) {
    EtatJeuSauvegarde sauvegarde;
    EtatJeuSauvegardeV1 sauvegardeV1;
    BulleSauvegardee *bullesSauvegardees;
    FILE *fichier;
    long tailleFichier;
    long tailleSauvegardeAttendue;

    if (!etat || !chemin || chemin[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE) {
        return FAUX;
    }

    fichier = fopen(chemin, "rb");
    if (!fichier) {
        return FAUX;
    }

    if (fseek(fichier, VALEUR_NULLE, SEEK_END) != VALEUR_NULLE) {
        fclose(fichier);
        return FAUX;
    }
    tailleFichier = ftell(fichier);
    if (tailleFichier < VALEUR_NULLE || fseek(fichier, VALEUR_NULLE, SEEK_SET) != VALEUR_NULLE) {
        fclose(fichier);
        return FAUX;
    }

    if (fread(&sauvegarde, sizeof(sauvegarde), VALEUR_UNITAIRE, fichier) != VALEUR_UNITAIRE) {
        fclose(fichier);
        return FAUX;
    }

    tailleSauvegardeAttendue = (long) sizeof(EtatJeuSauvegarde) +
                               (long) sauvegarde.nbBulles * (long) sizeof(BulleSauvegardee);
    if (tailleFichier != tailleSauvegardeAttendue) {
        if (fseek(fichier, VALEUR_NULLE, SEEK_SET) != VALEUR_NULLE ||
            fread(&sauvegardeV1, sizeof(sauvegardeV1), VALEUR_UNITAIRE, fichier) != VALEUR_UNITAIRE) {
            fclose(fichier);
            return FAUX;
        }

        convertir_sauvegarde_v1(&sauvegardeV1, &sauvegarde);
        tailleSauvegardeAttendue = (long) sizeof(EtatJeuSauvegardeV1) +
                                   (long) sauvegarde.nbBulles * (long) sizeof(BulleSauvegardee);
        if (tailleFichier != tailleSauvegardeAttendue) {
            fclose(fichier);
            return FAUX;
        }
    }

    bullesSauvegardees = NULL;
    if (sauvegarde.nbBulles > VALEUR_NULLE) {
        bullesSauvegardees = (BulleSauvegardee *) malloc((size_t) sauvegarde.nbBulles * sizeof(BulleSauvegardee));
        if (!bullesSauvegardees) {
            fclose(fichier);
            return FAUX;
        }

        if (fread(bullesSauvegardees,
                  sizeof(BulleSauvegardee),
                  (size_t) sauvegarde.nbBulles,
                  fichier) != (size_t) sauvegarde.nbBulles) {
            fclose(fichier);
            free(bullesSauvegardees);
            return FAUX;
        }
    }

    fclose(fichier);

    if (!reserver_depuis_sauvegarde(etat,
                                    sauvegarde.nbBulles > VALEUR_NULLE
                                    ? sauvegarde.nbBulles
                                    : CAPACITE_BULLES_INITIALE)) {
        free(bullesSauvegardees);
        return FAUX;
    }

    copier_depuis_sauvegarde(&sauvegarde, bullesSauvegardees, etat);
    free(bullesSauvegardees);
    return VRAI;
}

void fermer_sauvegarde(void) {
}
