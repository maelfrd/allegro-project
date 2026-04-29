#include "sauvegarde.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION_FORMAT_SAUVEGARDE 1
#define NOMBRE_MAXIMUM_BULLES_SAUVEGARDE 1024
#define NOM_FORMAT_SAUVEGARDE "SAUVEGARDE_JEU_ALLEGRO"

typedef struct {
    int nombreBullesEnJeu;
    int niveauActuel;
    int niveauMaximum;
    int positionSolY;
    int limiteGaucheTerrain;
    int limiteDroiteTerrain;
    int positionJoueurX;
    int positionJoueurY;
    int vitesseJoueur;
    int projectileEstActif;
    int positionProjectileX;
    int positionProjectileY;
    int largeurProjectile;
    int hauteurProjectile;
    int vitesseProjectile;
    int chapeauEstVisible;
    int positionChapeauX;
    int positionChapeauY;
    int largeurChapeau;
    int hauteurChapeau;
    float vitesseChapeauX;
    float vitesseChapeauY;
    int explosionEstActive;
    int positionExplosionX;
    int positionExplosionY;
    int largeurExplosion;
    int hauteurExplosion;
    int dureeExplosionRestanteImages;
    int bonusAuraArdenteEstActif;
    int dureeBonusAuraArdenteRestanteMs;
    int partieEstPerdue;
    int partieEstGagnee;
    int scoreJoueur;
    int tempsRestantNiveauMs;
    char pseudoJoueur[TAILLE_PSEUDO_MAX];
} DonneesGeneralesSauvegarde;

typedef struct {
    float positionBulleX;
    float positionBulleY;
    float vitesseBulleX;
    float vitesseBulleY;
    int typeBulle;
    int tailleBulle;
    float graviteBulle;
    float rebondBulleAuSol;
    float attenuationHorizontaleBulle;
    int largeurBulle;
    int hauteurBulle;
    int nombreCoupsRestantsAvantDivision;
} DonneesBulleSauvegardee;

static const char *nom_type_bulle(int typeBulle) {
    switch (typeBulle) {
        case ENTITE_MANGE_MORT:
            return "mange_mort";
        case ENTITE_VIF_DOR:
            return "vif_dor";
        case ENTITE_VOL_DE_MORT:
            return "vol_de_mort";
    }

    return "inconnu";
}

static int type_bulle_depuis_nom(const char *nomTypeBulle, int *typeBulle) {
    if (!nomTypeBulle || !typeBulle) {
        return FAUX;
    }

    if (strcmp(nomTypeBulle, "mange_mort") == VALEUR_NULLE) {
        *typeBulle = ENTITE_MANGE_MORT;
        return VRAI;
    }

    if (strcmp(nomTypeBulle, "vif_dor") == VALEUR_NULLE) {
        *typeBulle = ENTITE_VIF_DOR;
        return VRAI;
    }

    if (strcmp(nomTypeBulle, "vol_de_mort") == VALEUR_NULLE) {
        *typeBulle = ENTITE_VOL_DE_MORT;
        return VRAI;
    }

    return FAUX;
}

static const char *nom_taille_bulle(int tailleBulle) {
    switch (tailleBulle) {
        case BULLE_TRES_GRANDE:
            return "tres_grande";
        case BULLE_GRANDE:
            return "grande";
        case BULLE_MOYENNE:
            return "moyenne";
        case BULLE_PETITE:
            return "petite";
        case BULLE_TAILLES_TOTAL:
            break;
    }

    return "inconnue";
}

static int taille_bulle_depuis_nom(const char *nomTailleBulle, int *tailleBulle) {
    if (!nomTailleBulle || !tailleBulle) {
        return FAUX;
    }

    if (strcmp(nomTailleBulle, "tres_grande") == VALEUR_NULLE) {
        *tailleBulle = BULLE_TRES_GRANDE;
        return VRAI;
    }

    if (strcmp(nomTailleBulle, "grande") == VALEUR_NULLE) {
        *tailleBulle = BULLE_GRANDE;
        return VRAI;
    }

    if (strcmp(nomTailleBulle, "moyenne") == VALEUR_NULLE) {
        *tailleBulle = BULLE_MOYENNE;
        return VRAI;
    }

    if (strcmp(nomTailleBulle, "petite") == VALEUR_NULLE) {
        *tailleBulle = BULLE_PETITE;
        return VRAI;
    }

    return FAUX;
}

static int partie_peut_etre_sauvegardee(const EtatJeu *etatJeu) {
    return etatJeu &&
           etatJeu->pseudoJoueur[CHAINE_DEBUT] != CARACTERE_FIN_CHAINE &&
           !etatJeu->partiePerdue &&
           !etatJeu->partieGagnee &&
           etatJeu->tempsRestantNiveauMs > VALEUR_NULLE &&
           etatJeu->bullesEnJeu &&
           etatJeu->nombreBulles > VALEUR_NULLE &&
           etatJeu->nombreBulles <= NOMBRE_MAXIMUM_BULLES_SAUVEGARDE;
}

static int donnees_generales_sont_valides(const DonneesGeneralesSauvegarde *donneesGenerales) {
    if (!donneesGenerales) {
        return FAUX;
    }

    if (donneesGenerales->pseudoJoueur[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE ||
        donneesGenerales->nombreBullesEnJeu <= VALEUR_NULLE ||
        donneesGenerales->nombreBullesEnJeu > NOMBRE_MAXIMUM_BULLES_SAUVEGARDE ||
        donneesGenerales->tempsRestantNiveauMs <= VALEUR_NULLE ||
        donneesGenerales->partieEstPerdue ||
        donneesGenerales->partieEstGagnee) {
        return FAUX;
    }

    if (donneesGenerales->niveauActuel < PREMIER_NIVEAU_JEU ||
        donneesGenerales->niveauActuel > NIVEAU_MAXIMUM_JEU ||
        donneesGenerales->niveauMaximum < PREMIER_NIVEAU_JEU ||
        donneesGenerales->niveauMaximum > NIVEAU_MAXIMUM_JEU) {
        return FAUX;
    }

    return VRAI;
}

static int donnees_bulle_sont_valides(const DonneesBulleSauvegardee *donneesBulle) {
    if (!donneesBulle) {
        return FAUX;
    }

    return donneesBulle->typeBulle >= ENTITE_MANGE_MORT &&
           donneesBulle->typeBulle <= ENTITE_VOL_DE_MORT &&
           donneesBulle->tailleBulle >= BULLE_TRES_GRANDE &&
           donneesBulle->tailleBulle < BULLE_TAILLES_TOTAL &&
           donneesBulle->largeurBulle > VALEUR_NULLE &&
           donneesBulle->hauteurBulle > VALEUR_NULLE;
}

static int lire_mot_attendu(FILE *fichierSauvegarde, const char *motAttendu) {
    char motLu[64];

    if (!fichierSauvegarde || !motAttendu) {
        return FAUX;
    }

    return fscanf(fichierSauvegarde, "%63s", motLu) == VALEUR_UNITAIRE &&
           strcmp(motLu, motAttendu) == VALEUR_NULLE;
}

static int lire_valeur_entiere(FILE *fichierSauvegarde, const char *nomChamp, int *valeurLue) {
    return lire_mot_attendu(fichierSauvegarde, nomChamp) &&
           fscanf(fichierSauvegarde, "%d", valeurLue) == VALEUR_UNITAIRE;
}

static int lire_valeur_flottante(FILE *fichierSauvegarde, const char *nomChamp, float *valeurLue) {
    return lire_mot_attendu(fichierSauvegarde, nomChamp) &&
           fscanf(fichierSauvegarde, "%f", valeurLue) == VALEUR_UNITAIRE;
}

static int lire_pseudo_joueur(FILE *fichierSauvegarde, char *pseudoJoueur, int taillePseudoJoueur) {
    if (!lire_mot_attendu(fichierSauvegarde, "pseudo_joueur") ||
        fscanf(fichierSauvegarde, "%31s", pseudoJoueur) != VALEUR_UNITAIRE) {
        return FAUX;
    }

    pseudoJoueur[taillePseudoJoueur - INDEX_SUIVANT] = CARACTERE_FIN_CHAINE;
    return VRAI;
}

static int lire_type_bulle(FILE *fichierSauvegarde, int *typeBulle) {
    char nomTypeBulle[64];

    if (!lire_mot_attendu(fichierSauvegarde, "type_bulle") ||
        fscanf(fichierSauvegarde, "%63s", nomTypeBulle) != VALEUR_UNITAIRE) {
        return FAUX;
    }

    return type_bulle_depuis_nom(nomTypeBulle, typeBulle);
}

static int lire_taille_bulle(FILE *fichierSauvegarde, int *tailleBulle) {
    char nomTailleBulle[64];

    if (!lire_mot_attendu(fichierSauvegarde, "taille_bulle") ||
        fscanf(fichierSauvegarde, "%63s", nomTailleBulle) != VALEUR_UNITAIRE) {
        return FAUX;
    }

    return taille_bulle_depuis_nom(nomTailleBulle, tailleBulle);
}

static int lire_donnees_generales(FILE *fichierSauvegarde,
                                  DonneesGeneralesSauvegarde *donneesGenerales) {
    char nomFormatSauvegarde[64];
    int versionFormatSauvegarde;

    if (!fichierSauvegarde || !donneesGenerales) {
        return FAUX;
    }

    memset(donneesGenerales, VALEUR_NULLE, sizeof(*donneesGenerales));
    if (fseek(fichierSauvegarde, VALEUR_NULLE, SEEK_SET) != VALEUR_NULLE ||
        fscanf(fichierSauvegarde, "%63s %d", nomFormatSauvegarde, &versionFormatSauvegarde) != VALEUR_DOUBLE ||
        strcmp(nomFormatSauvegarde, NOM_FORMAT_SAUVEGARDE) != VALEUR_NULLE ||
        versionFormatSauvegarde != VERSION_FORMAT_SAUVEGARDE) {
        return FAUX;
    }

    if (!lire_pseudo_joueur(fichierSauvegarde, donneesGenerales->pseudoJoueur, TAILLE_PSEUDO_MAX) ||
        !lire_valeur_entiere(fichierSauvegarde, "temps_restant_niveau_ms", &donneesGenerales->tempsRestantNiveauMs) ||
        !lire_valeur_entiere(fichierSauvegarde, "score_joueur", &donneesGenerales->scoreJoueur) ||
        !lire_valeur_entiere(fichierSauvegarde, "bonus_aura_ardente_actif", &donneesGenerales->bonusAuraArdenteEstActif) ||
        !lire_valeur_entiere(fichierSauvegarde,
                             "temps_bonus_aura_ardente_restant_ms",
                             &donneesGenerales->dureeBonusAuraArdenteRestanteMs) ||
        !lire_valeur_entiere(fichierSauvegarde, "niveau_actuel", &donneesGenerales->niveauActuel) ||
        !lire_valeur_entiere(fichierSauvegarde, "niveau_maximum", &donneesGenerales->niveauMaximum) ||
        !lire_valeur_entiere(fichierSauvegarde, "nombre_bulles_en_jeu", &donneesGenerales->nombreBullesEnJeu) ||
        !lire_valeur_entiere(fichierSauvegarde, "position_sol_y", &donneesGenerales->positionSolY) ||
        !lire_valeur_entiere(fichierSauvegarde, "limite_gauche_terrain", &donneesGenerales->limiteGaucheTerrain) ||
        !lire_valeur_entiere(fichierSauvegarde, "limite_droite_terrain", &donneesGenerales->limiteDroiteTerrain) ||
        !lire_valeur_entiere(fichierSauvegarde, "position_joueur_x", &donneesGenerales->positionJoueurX) ||
        !lire_valeur_entiere(fichierSauvegarde, "position_joueur_y", &donneesGenerales->positionJoueurY) ||
        !lire_valeur_entiere(fichierSauvegarde, "vitesse_joueur", &donneesGenerales->vitesseJoueur) ||
        !lire_valeur_entiere(fichierSauvegarde, "projectile_actif", &donneesGenerales->projectileEstActif) ||
        !lire_valeur_entiere(fichierSauvegarde, "position_projectile_x", &donneesGenerales->positionProjectileX) ||
        !lire_valeur_entiere(fichierSauvegarde, "position_projectile_y", &donneesGenerales->positionProjectileY) ||
        !lire_valeur_entiere(fichierSauvegarde, "largeur_projectile", &donneesGenerales->largeurProjectile) ||
        !lire_valeur_entiere(fichierSauvegarde, "hauteur_projectile", &donneesGenerales->hauteurProjectile) ||
        !lire_valeur_entiere(fichierSauvegarde, "vitesse_projectile", &donneesGenerales->vitesseProjectile) ||
        !lire_valeur_entiere(fichierSauvegarde, "chapeau_visible", &donneesGenerales->chapeauEstVisible) ||
        !lire_valeur_entiere(fichierSauvegarde, "position_chapeau_x", &donneesGenerales->positionChapeauX) ||
        !lire_valeur_entiere(fichierSauvegarde, "position_chapeau_y", &donneesGenerales->positionChapeauY) ||
        !lire_valeur_entiere(fichierSauvegarde, "largeur_chapeau", &donneesGenerales->largeurChapeau) ||
        !lire_valeur_entiere(fichierSauvegarde, "hauteur_chapeau", &donneesGenerales->hauteurChapeau) ||
        !lire_valeur_flottante(fichierSauvegarde, "vitesse_chapeau_x", &donneesGenerales->vitesseChapeauX) ||
        !lire_valeur_flottante(fichierSauvegarde, "vitesse_chapeau_y", &donneesGenerales->vitesseChapeauY) ||
        !lire_valeur_entiere(fichierSauvegarde, "explosion_active", &donneesGenerales->explosionEstActive) ||
        !lire_valeur_entiere(fichierSauvegarde, "position_explosion_x", &donneesGenerales->positionExplosionX) ||
        !lire_valeur_entiere(fichierSauvegarde, "position_explosion_y", &donneesGenerales->positionExplosionY) ||
        !lire_valeur_entiere(fichierSauvegarde, "largeur_explosion", &donneesGenerales->largeurExplosion) ||
        !lire_valeur_entiere(fichierSauvegarde, "hauteur_explosion", &donneesGenerales->hauteurExplosion) ||
        !lire_valeur_entiere(fichierSauvegarde,
                             "duree_explosion_restante_images",
                             &donneesGenerales->dureeExplosionRestanteImages) ||
        !lire_valeur_entiere(fichierSauvegarde, "partie_perdue", &donneesGenerales->partieEstPerdue) ||
        !lire_valeur_entiere(fichierSauvegarde, "partie_gagnee", &donneesGenerales->partieEstGagnee) ||
        !lire_mot_attendu(fichierSauvegarde, "liste_bulles")) {
        return FAUX;
    }

    return donnees_generales_sont_valides(donneesGenerales);
}

static int reserver_tableau_bulles(EtatJeu *etatJeu, int nombreBullesNecessaire) {
    Bulle *nouveauTableauBulles;
    int capaciteBullesNecessaire;

    if (!etatJeu || nombreBullesNecessaire <= VALEUR_NULLE) {
        return FAUX;
    }

    if (nombreBullesNecessaire <= etatJeu->capaciteMaxBullesEnJeu) {
        return VRAI;
    }

    capaciteBullesNecessaire = etatJeu->capaciteMaxBullesEnJeu > VALEUR_NULLE
                               ? etatJeu->capaciteMaxBullesEnJeu
                               : CAPACITE_BULLES_INITIALE;

    while (capaciteBullesNecessaire < nombreBullesNecessaire) {
        capaciteBullesNecessaire *= FACTEUR_CAPACITE_BULLES;
    }

    nouveauTableauBulles = (Bulle *) realloc(etatJeu->bullesEnJeu,
                                             (size_t) capaciteBullesNecessaire * sizeof(Bulle));
    if (!nouveauTableauBulles) {
        return FAUX;
    }

    etatJeu->bullesEnJeu = nouveauTableauBulles;
    etatJeu->capaciteMaxBullesEnJeu = capaciteBullesNecessaire;
    return VRAI;
}

static void remplir_donnees_generales_depuis_etat(const EtatJeu *etatJeu,
                                                  DonneesGeneralesSauvegarde *donneesGenerales) {
    memset(donneesGenerales, VALEUR_NULLE, sizeof(*donneesGenerales));

    donneesGenerales->nombreBullesEnJeu = etatJeu->nombreBulles;
    donneesGenerales->niveauActuel = etatJeu->niveauActuel;
    donneesGenerales->niveauMaximum = etatJeu->niveauMaximum;
    donneesGenerales->positionSolY = etatJeu->positionSolY;
    donneesGenerales->limiteGaucheTerrain = etatJeu->limiteGaucheTerrain;
    donneesGenerales->limiteDroiteTerrain = etatJeu->limiteDroiteTerrain;
    donneesGenerales->positionJoueurX = etatJeu->positionJoueurX;
    donneesGenerales->positionJoueurY = etatJeu->positionJoueurY;
    donneesGenerales->vitesseJoueur = etatJeu->vitesseJoueur;
    donneesGenerales->projectileEstActif = etatJeu->projectileEstActif;
    donneesGenerales->positionProjectileX = etatJeu->positionProjectileX;
    donneesGenerales->positionProjectileY = etatJeu->positionProjectileY;
    donneesGenerales->largeurProjectile = etatJeu->largeurProjectile;
    donneesGenerales->hauteurProjectile = etatJeu->hauteurProjectile;
    donneesGenerales->vitesseProjectile = etatJeu->vitesseProjectile;
    donneesGenerales->chapeauEstVisible = etatJeu->chapeauEstVisible;
    donneesGenerales->positionChapeauX = etatJeu->positionChapeauX;
    donneesGenerales->positionChapeauY = etatJeu->positionChapeauY;
    donneesGenerales->largeurChapeau = etatJeu->largeurChapeau;
    donneesGenerales->hauteurChapeau = etatJeu->hauteurChapeau;
    donneesGenerales->vitesseChapeauX = etatJeu->vitesseChapeauX;
    donneesGenerales->vitesseChapeauY = etatJeu->vitesseChapeauY;
    donneesGenerales->explosionEstActive = etatJeu->explosionEstActive;
    donneesGenerales->positionExplosionX = etatJeu->positionExplosionX;
    donneesGenerales->positionExplosionY = etatJeu->positionExplosionY;
    donneesGenerales->largeurExplosion = etatJeu->largeurExplosion;
    donneesGenerales->hauteurExplosion = etatJeu->hauteurExplosion;
    donneesGenerales->dureeExplosionRestanteImages = etatJeu->dureeExplosionRestanteImages;
    donneesGenerales->bonusAuraArdenteEstActif = etatJeu->auraArdenteEstActive;
    donneesGenerales->dureeBonusAuraArdenteRestanteMs = etatJeu->dureeRestanteAuraArdenteMs;
    donneesGenerales->partieEstPerdue = etatJeu->partiePerdue;
    donneesGenerales->partieEstGagnee = etatJeu->partieGagnee;
    donneesGenerales->scoreJoueur = etatJeu->scoreJoueur;
    donneesGenerales->tempsRestantNiveauMs = etatJeu->tempsRestantNiveauMs;

    strncpy(donneesGenerales->pseudoJoueur, etatJeu->pseudoJoueur, TAILLE_PSEUDO_MAX - INDEX_SUIVANT);
    donneesGenerales->pseudoJoueur[TAILLE_PSEUDO_MAX - INDEX_SUIVANT] = CARACTERE_FIN_CHAINE;
}

static void remplir_donnees_bulle_depuis_etat(const Bulle *bulleJeu,
                                              DonneesBulleSauvegardee *donneesBulle) {
    donneesBulle->positionBulleX = bulleJeu->positionBulleX;
    donneesBulle->positionBulleY = bulleJeu->positionBulleY;
    donneesBulle->vitesseBulleX = bulleJeu->vitesseBulleX;
    donneesBulle->vitesseBulleY = bulleJeu->vitesseBulleY;
    donneesBulle->typeBulle = (int) bulleJeu->typeEntite;
    donneesBulle->tailleBulle = (int) bulleJeu->tailleBulle;
    donneesBulle->graviteBulle = bulleJeu->gravite;
    donneesBulle->rebondBulleAuSol = bulleJeu->rebondSol;
    donneesBulle->attenuationHorizontaleBulle = bulleJeu->attenuationX;
    donneesBulle->largeurBulle = bulleJeu->largeur;
    donneesBulle->hauteurBulle = bulleJeu->hauteur;
    donneesBulle->nombreCoupsRestantsAvantDivision = bulleJeu->nombreCoupsAvantDivision;
}

static void appliquer_donnees_generales_a_etat(const DonneesGeneralesSauvegarde *donneesGenerales,
                                               EtatJeu *etatJeu) {
    etatJeu->nombreBulles = donneesGenerales->nombreBullesEnJeu;
    etatJeu->niveauActuel = donneesGenerales->niveauActuel;
    etatJeu->niveauMaximum = donneesGenerales->niveauMaximum;
    etatJeu->positionSolY = donneesGenerales->positionSolY;
    etatJeu->limiteGaucheTerrain = donneesGenerales->limiteGaucheTerrain;
    etatJeu->limiteDroiteTerrain = donneesGenerales->limiteDroiteTerrain;
    etatJeu->positionJoueurX = donneesGenerales->positionJoueurX;
    etatJeu->positionJoueurY = donneesGenerales->positionJoueurY;
    etatJeu->vitesseJoueur = donneesGenerales->vitesseJoueur;
    etatJeu->projectileEstActif = donneesGenerales->projectileEstActif;
    etatJeu->positionProjectileX = donneesGenerales->positionProjectileX;
    etatJeu->positionProjectileY = donneesGenerales->positionProjectileY;
    etatJeu->largeurProjectile = donneesGenerales->largeurProjectile;
    etatJeu->hauteurProjectile = donneesGenerales->hauteurProjectile;
    etatJeu->vitesseProjectile = donneesGenerales->vitesseProjectile;
    etatJeu->chapeauEstVisible = donneesGenerales->chapeauEstVisible;
    etatJeu->positionChapeauX = donneesGenerales->positionChapeauX;
    etatJeu->positionChapeauY = donneesGenerales->positionChapeauY;
    etatJeu->largeurChapeau = donneesGenerales->largeurChapeau;
    etatJeu->hauteurChapeau = donneesGenerales->hauteurChapeau;
    etatJeu->vitesseChapeauX = donneesGenerales->vitesseChapeauX;
    etatJeu->vitesseChapeauY = donneesGenerales->vitesseChapeauY;
    etatJeu->explosionEstActive = donneesGenerales->explosionEstActive;
    etatJeu->positionExplosionX = donneesGenerales->positionExplosionX;
    etatJeu->positionExplosionY = donneesGenerales->positionExplosionY;
    etatJeu->largeurExplosion = donneesGenerales->largeurExplosion;
    etatJeu->hauteurExplosion = donneesGenerales->hauteurExplosion;
    etatJeu->dureeExplosionRestanteImages = donneesGenerales->dureeExplosionRestanteImages;
    etatJeu->auraArdenteEstActive = donneesGenerales->bonusAuraArdenteEstActif;
    etatJeu->dureeRestanteAuraArdenteMs = donneesGenerales->dureeBonusAuraArdenteRestanteMs;
    etatJeu->partiePerdue = FAUX;
    etatJeu->partieGagnee = FAUX;
    etatJeu->scoreJoueur = donneesGenerales->scoreJoueur;
    etatJeu->tempsRestantNiveauMs = donneesGenerales->tempsRestantNiveauMs;

    strncpy(etatJeu->pseudoJoueur, donneesGenerales->pseudoJoueur, TAILLE_PSEUDO_MAX - INDEX_SUIVANT);
    etatJeu->pseudoJoueur[TAILLE_PSEUDO_MAX - INDEX_SUIVANT] = CARACTERE_FIN_CHAINE;
}

static void appliquer_donnees_bulle_a_etat(const DonneesBulleSauvegardee *donneesBulle,
                                           Bulle *bulleJeu) {
    bulleJeu->positionBulleX = donneesBulle->positionBulleX;
    bulleJeu->positionBulleY = donneesBulle->positionBulleY;
    bulleJeu->vitesseBulleX = donneesBulle->vitesseBulleX;
    bulleJeu->vitesseBulleY = donneesBulle->vitesseBulleY;
    bulleJeu->typeEntite = (TypeEntite) donneesBulle->typeBulle;
    bulleJeu->tailleBulle = (TailleBulle) donneesBulle->tailleBulle;
    bulleJeu->gravite = donneesBulle->graviteBulle;
    bulleJeu->rebondSol = donneesBulle->rebondBulleAuSol;
    bulleJeu->attenuationX = donneesBulle->attenuationHorizontaleBulle;
    bulleJeu->largeur = donneesBulle->largeurBulle;
    bulleJeu->hauteur = donneesBulle->hauteurBulle;
    bulleJeu->nombreCoupsAvantDivision = donneesBulle->nombreCoupsRestantsAvantDivision;
}

static int ecrire_donnees_generales(FILE *fichierSauvegarde,
                                    const DonneesGeneralesSauvegarde *donneesGenerales) {
    if (!fichierSauvegarde || !donneesGenerales) {
        return FAUX;
    }

    return fprintf(fichierSauvegarde,
                   NOM_FORMAT_SAUVEGARDE " %d\n"
                   "pseudo_joueur %s\n"
                   "temps_restant_niveau_ms %d\n"
                   "score_joueur %d\n"
                   "bonus_aura_ardente_actif %d\n"
                   "temps_bonus_aura_ardente_restant_ms %d\n"
                   "niveau_actuel %d\n"
                   "niveau_maximum %d\n"
                   "nombre_bulles_en_jeu %d\n"
                   "position_sol_y %d\n"
                   "limite_gauche_terrain %d\n"
                   "limite_droite_terrain %d\n"
                   "position_joueur_x %d\n"
                   "position_joueur_y %d\n"
                   "vitesse_joueur %d\n"
                   "projectile_actif %d\n"
                   "position_projectile_x %d\n"
                   "position_projectile_y %d\n"
                   "largeur_projectile %d\n"
                   "hauteur_projectile %d\n"
                   "vitesse_projectile %d\n"
                   "chapeau_visible %d\n"
                   "position_chapeau_x %d\n"
                   "position_chapeau_y %d\n"
                   "largeur_chapeau %d\n"
                   "hauteur_chapeau %d\n"
                   "vitesse_chapeau_x %.9g\n"
                   "vitesse_chapeau_y %.9g\n"
                   "explosion_active %d\n"
                   "position_explosion_x %d\n"
                   "position_explosion_y %d\n"
                   "largeur_explosion %d\n"
                   "hauteur_explosion %d\n"
                   "duree_explosion_restante_images %d\n"
                   "partie_perdue %d\n"
                   "partie_gagnee %d\n"
                   "liste_bulles\n",
                   VERSION_FORMAT_SAUVEGARDE,
                   donneesGenerales->pseudoJoueur,
                   donneesGenerales->tempsRestantNiveauMs,
                   donneesGenerales->scoreJoueur,
                   donneesGenerales->bonusAuraArdenteEstActif,
                   donneesGenerales->dureeBonusAuraArdenteRestanteMs,
                   donneesGenerales->niveauActuel,
                   donneesGenerales->niveauMaximum,
                   donneesGenerales->nombreBullesEnJeu,
                   donneesGenerales->positionSolY,
                   donneesGenerales->limiteGaucheTerrain,
                   donneesGenerales->limiteDroiteTerrain,
                   donneesGenerales->positionJoueurX,
                   donneesGenerales->positionJoueurY,
                   donneesGenerales->vitesseJoueur,
                   donneesGenerales->projectileEstActif,
                   donneesGenerales->positionProjectileX,
                   donneesGenerales->positionProjectileY,
                   donneesGenerales->largeurProjectile,
                   donneesGenerales->hauteurProjectile,
                   donneesGenerales->vitesseProjectile,
                   donneesGenerales->chapeauEstVisible,
                   donneesGenerales->positionChapeauX,
                   donneesGenerales->positionChapeauY,
                   donneesGenerales->largeurChapeau,
                   donneesGenerales->hauteurChapeau,
                   donneesGenerales->vitesseChapeauX,
                   donneesGenerales->vitesseChapeauY,
                   donneesGenerales->explosionEstActive,
                   donneesGenerales->positionExplosionX,
                   donneesGenerales->positionExplosionY,
                   donneesGenerales->largeurExplosion,
                   donneesGenerales->hauteurExplosion,
                   donneesGenerales->dureeExplosionRestanteImages,
                   donneesGenerales->partieEstPerdue,
                   donneesGenerales->partieEstGagnee) > VALEUR_NULLE;
}

static int ecrire_donnees_bulle(FILE *fichierSauvegarde,
                                const DonneesBulleSauvegardee *donneesBulle,
                                int numeroBulle) {
    if (!fichierSauvegarde || !donneesBulle) {
        return FAUX;
    }

    return fprintf(fichierSauvegarde,
                   "bulle_%d\n"
                   "type_bulle %s\n"
                   "taille_bulle %s\n"
                   "position_bulle_x %.9g\n"
                   "position_bulle_y %.9g\n"
                   "vitesse_bulle_x %.9g\n"
                   "vitesse_bulle_y %.9g\n"
                   "gravite_bulle %.9g\n"
                   "rebond_bulle_au_sol %.9g\n"
                   "attenuation_horizontale_bulle %.9g\n"
                   "largeur_bulle %d\n"
                   "hauteur_bulle %d\n"
                   "coups_restants_avant_division %d\n",
                   numeroBulle,
                   nom_type_bulle(donneesBulle->typeBulle),
                   nom_taille_bulle(donneesBulle->tailleBulle),
                   donneesBulle->positionBulleX,
                   donneesBulle->positionBulleY,
                   donneesBulle->vitesseBulleX,
                   donneesBulle->vitesseBulleY,
                   donneesBulle->graviteBulle,
                   donneesBulle->rebondBulleAuSol,
                   donneesBulle->attenuationHorizontaleBulle,
                   donneesBulle->largeurBulle,
                   donneesBulle->hauteurBulle,
                   donneesBulle->nombreCoupsRestantsAvantDivision) > VALEUR_NULLE;
}

static int lire_donnees_bulle(FILE *fichierSauvegarde,
                              DonneesBulleSauvegardee *donneesBulle,
                              int numeroBulle) {
    char libelleBulleAttendu[64];

    snprintf(libelleBulleAttendu, sizeof(libelleBulleAttendu), "bulle_%d", numeroBulle);
    if (!lire_mot_attendu(fichierSauvegarde, libelleBulleAttendu) ||
        !lire_type_bulle(fichierSauvegarde, &donneesBulle->typeBulle) ||
        !lire_taille_bulle(fichierSauvegarde, &donneesBulle->tailleBulle) ||
        !lire_valeur_flottante(fichierSauvegarde, "position_bulle_x", &donneesBulle->positionBulleX) ||
        !lire_valeur_flottante(fichierSauvegarde, "position_bulle_y", &donneesBulle->positionBulleY) ||
        !lire_valeur_flottante(fichierSauvegarde, "vitesse_bulle_x", &donneesBulle->vitesseBulleX) ||
        !lire_valeur_flottante(fichierSauvegarde, "vitesse_bulle_y", &donneesBulle->vitesseBulleY) ||
        !lire_valeur_flottante(fichierSauvegarde, "gravite_bulle", &donneesBulle->graviteBulle) ||
        !lire_valeur_flottante(fichierSauvegarde, "rebond_bulle_au_sol", &donneesBulle->rebondBulleAuSol) ||
        !lire_valeur_flottante(fichierSauvegarde,
                               "attenuation_horizontale_bulle",
                               &donneesBulle->attenuationHorizontaleBulle) ||
        !lire_valeur_entiere(fichierSauvegarde, "largeur_bulle", &donneesBulle->largeurBulle) ||
        !lire_valeur_entiere(fichierSauvegarde, "hauteur_bulle", &donneesBulle->hauteurBulle) ||
        !lire_valeur_entiere(fichierSauvegarde,
                             "coups_restants_avant_division",
                             &donneesBulle->nombreCoupsRestantsAvantDivision)) {
        return FAUX;
    }

    return donnees_bulle_sont_valides(donneesBulle);
}

int initialiser_sauvegarde(void) {
    return VRAI;
}

int sauvegarder_etat_jeu(const EtatJeu *etatJeu, const char *cheminSauvegarde) {
    DonneesGeneralesSauvegarde donneesGenerales;
    DonneesBulleSauvegardee donneesBulle;
    FILE *fichierSauvegarde;
    int indexBulle;

    if (!partie_peut_etre_sauvegardee(etatJeu) ||
        !cheminSauvegarde ||
        cheminSauvegarde[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE) {
        return FAUX;
    }

    fichierSauvegarde = fopen(cheminSauvegarde, "w");
    if (!fichierSauvegarde) {
        return FAUX;
    }

    remplir_donnees_generales_depuis_etat(etatJeu, &donneesGenerales);
    if (!ecrire_donnees_generales(fichierSauvegarde, &donneesGenerales)) {
        fclose(fichierSauvegarde);
        return FAUX;
    }

    for (indexBulle = INDEX_PREMIER; indexBulle < etatJeu->nombreBulles; indexBulle++) {
        remplir_donnees_bulle_depuis_etat(&etatJeu->bullesEnJeu[indexBulle], &donneesBulle);
        if (!donnees_bulle_sont_valides(&donneesBulle) ||
            !ecrire_donnees_bulle(fichierSauvegarde, &donneesBulle, indexBulle + INDEX_SUIVANT)) {
            fclose(fichierSauvegarde);
            return FAUX;
        }
    }

    fclose(fichierSauvegarde);
    return VRAI;
}

int charger_etat_jeu(EtatJeu *etatJeu, const char *cheminSauvegarde) {
    DonneesGeneralesSauvegarde donneesGenerales;
    DonneesBulleSauvegardee donneesBulle;
    FILE *fichierSauvegarde;
    int indexBulle;

    if (!etatJeu || !cheminSauvegarde || cheminSauvegarde[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE) {
        return FAUX;
    }

    fichierSauvegarde = fopen(cheminSauvegarde, "r");
    if (!fichierSauvegarde) {
        return FAUX;
    }

    if (!lire_donnees_generales(fichierSauvegarde, &donneesGenerales) ||
        !reserver_tableau_bulles(etatJeu, donneesGenerales.nombreBullesEnJeu)) {
        fclose(fichierSauvegarde);
        return FAUX;
    }

    appliquer_donnees_generales_a_etat(&donneesGenerales, etatJeu);
    for (indexBulle = INDEX_PREMIER; indexBulle < donneesGenerales.nombreBullesEnJeu; indexBulle++) {
        if (!lire_donnees_bulle(fichierSauvegarde, &donneesBulle, indexBulle + INDEX_SUIVANT)) {
            etatJeu->nombreBulles = VALEUR_NULLE;
            fclose(fichierSauvegarde);
            return FAUX;
        }

        appliquer_donnees_bulle_a_etat(&donneesBulle, &etatJeu->bullesEnJeu[indexBulle]);
    }

    fclose(fichierSauvegarde);
    return VRAI;
}

int charger_pseudo_sauvegarde(const char *cheminSauvegarde, char *pseudoJoueur, int taillePseudoJoueur) {
    DonneesGeneralesSauvegarde donneesGenerales;
    FILE *fichierSauvegarde;

    if (!cheminSauvegarde || !pseudoJoueur || taillePseudoJoueur <= VALEUR_UNITAIRE) {
        return FAUX;
    }

    pseudoJoueur[CHAINE_DEBUT] = CARACTERE_FIN_CHAINE;
    fichierSauvegarde = fopen(cheminSauvegarde, "r");
    if (!fichierSauvegarde) {
        return FAUX;
    }

    if (!lire_donnees_generales(fichierSauvegarde, &donneesGenerales)) {
        fclose(fichierSauvegarde);
        return FAUX;
    }

    fclose(fichierSauvegarde);
    strncpy(pseudoJoueur, donneesGenerales.pseudoJoueur, (size_t) taillePseudoJoueur - INDEX_SUIVANT);
    pseudoJoueur[taillePseudoJoueur - INDEX_SUIVANT] = CARACTERE_FIN_CHAINE;
    return VRAI;
}

void fermer_sauvegarde(void) {
}
