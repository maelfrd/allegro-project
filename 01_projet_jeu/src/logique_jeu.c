#include "logique_jeu.h"

#include <stdlib.h>
#include <string.h>

static float vitesse_horizontale_relative(const ConfigurationJeu *configuration, int diviseur) {
    if (!configuration || diviseur <= VALEUR_NULLE) {
        return ZERO_FLOTTANT;
    }

    return (float) configuration->largeurFenetre / (float) diviseur;
}

static float vitesse_verticale_relative(const ConfigurationJeu *configuration, int diviseur) {
    if (!configuration || diviseur <= VALEUR_NULLE) {
        return ZERO_FLOTTANT;
    }

    return (float) configuration->hauteurFenetre / (float) diviseur;
}

static int calculer_score_bulle(const Bulle *bulle) {
    int scoreJoueur;

    if (!bulle) {
        return VALEUR_NULLE;
    }

    switch (bulle->tailleBulle) {
        case BULLE_TRES_GRANDE:
            scoreJoueur = SCORE_BULLE_TRES_GRANDE;
            break;
        case BULLE_GRANDE:
            scoreJoueur = SCORE_BULLE_GRANDE;
            break;
        case BULLE_MOYENNE:
            scoreJoueur = SCORE_BULLE_MOYENNE;
            break;
        case BULLE_PETITE:
            scoreJoueur = SCORE_BULLE_PETITE;
            break;
        case BULLE_TAILLES_TOTAL:
            scoreJoueur = VALEUR_NULLE;
            break;
    }

    if (bulle->typeEntite == ENTITE_VIF_DOR) {
        scoreJoueur += BONUS_SCORE_VIF_DOR;
    }
    if (bulle->typeEntite == ENTITE_VOL_DE_MORT) {
        scoreJoueur += BONUS_SCORE_VOL_DE_MORT;
    }

    return scoreJoueur;
}

static int rectangles_en_collision(float x1, float y1, int largeurRectangle1, int hauteurRectangle1,
                                   float x2, float y2, int largeurRectangle2, int hauteurRectangle2) {
    return (x1 < x2 + largeurRectangle2 &&
            x1 + largeurRectangle1 > x2 &&
            y1 < y2 + hauteurRectangle2 &&
            y1 + hauteurRectangle1 > y2);
}

static void copier_configuration_dans_etat(EtatJeu *etatJeu, const ConfigurationJeu *configuration) {
    etatJeu->positionSolY = configuration->positionSolY;
    etatJeu->limiteGaucheTerrain = configuration->limiteGaucheTerrain;
    etatJeu->limiteDroiteTerrain = configuration->limiteDroiteTerrain;
    etatJeu->vitesseJoueur = configuration->vitesseJoueur;
    etatJeu->largeurProjectile = configuration->projectileLargeur;
    etatJeu->hauteurProjectile = configuration->projectileHauteur;
    etatJeu->vitesseProjectile = configuration->projectileVitesse;
    etatJeu->largeurChapeau = configuration->chapeauLargeur;
    etatJeu->hauteurChapeau = configuration->chapeauHauteur;
    etatJeu->largeurExplosion = configuration->explosionLargeur;
    etatJeu->hauteurExplosion = configuration->explosionHauteur;
}

static void configurer_bulle(Bulle *bulle, TailleBulle tailleBulle, const ConfigurationJeu *configuration) {
    bulle->typeEntite = ENTITE_MANGE_MORT;
    bulle->tailleBulle = tailleBulle;
    bulle->largeur = configuration->largeurBulles[(int) tailleBulle];
    bulle->hauteur = configuration->hauteurBulles[(int) tailleBulle];
    bulle->nombreCoupsAvantDivision = VALEUR_NULLE;

    switch (tailleBulle) {
        case BULLE_TRES_GRANDE:
            bulle->gravite = vitesse_verticale_relative(configuration, DIVISEUR_GRAVITE_TRES_GRANDE);
            bulle->rebondSol = -vitesse_verticale_relative(configuration, DIVISEUR_REBOND_TRES_GRANDE);
            bulle->attenuationX = ATTENUATION_TRES_GRANDE;
            break;
        case BULLE_GRANDE:
            bulle->gravite = vitesse_verticale_relative(configuration, DIVISEUR_GRAVITE_GRANDE);
            bulle->rebondSol = -vitesse_verticale_relative(configuration, DIVISEUR_REBOND_GRANDE);
            bulle->attenuationX = ATTENUATION_GRANDE;
            break;
        case BULLE_MOYENNE:
            bulle->gravite = vitesse_verticale_relative(configuration, DIVISEUR_GRAVITE_MOYENNE);
            bulle->rebondSol = -vitesse_verticale_relative(configuration, DIVISEUR_REBOND_MOYENNE);
            bulle->attenuationX = ATTENUATION_MOYENNE;
            break;
        case BULLE_PETITE:
            bulle->gravite = vitesse_verticale_relative(configuration, DIVISEUR_GRAVITE_PETITE);
            bulle->rebondSol = -vitesse_verticale_relative(configuration, DIVISEUR_REBOND_PETITE);
            bulle->attenuationX = ATTENUATION_PETITE;
            break;
        case BULLE_TAILLES_TOTAL:
            break;
    }
}

static int assurer_capacite_bulles(EtatJeu *etatJeu, int nouvelleCapacite) {
    Bulle *nouveauTableauBulles;

    if (nouvelleCapacite <= etatJeu->capaciteMaxBullesEnJeu) {
        return VRAI;
    }

    nouveauTableauBulles = (Bulle *) realloc(etatJeu->bullesEnJeu, (size_t) nouvelleCapacite * sizeof(Bulle));
    if (!nouveauTableauBulles) {
        return FAUX;
    }

    etatJeu->bullesEnJeu = nouveauTableauBulles;
    etatJeu->capaciteMaxBullesEnJeu = nouvelleCapacite;
    return VRAI;
}

static float calculer_vitesse_horizontale_bulle_enfant(const ConfigurationJeu *configuration,
                                                       TailleBulle tailleBulle) {
    switch (tailleBulle) {
        case BULLE_TRES_GRANDE:
            return vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_ENFANT_TRES_GRANDE);
        case BULLE_GRANDE:
            return vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_ENFANT_GRANDE);
        case BULLE_MOYENNE:
            return vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_ENFANT_MOYENNE);
        case BULLE_PETITE:
            return vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_ENFANT_PETITE);
        case BULLE_TAILLES_TOTAL:
            break;
    }

    return vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_ENFANT_GRANDE);
}

static int ajouter_bulle(EtatJeu *etatJeu,
                         const ConfigurationJeu *configuration,
                         TypeEntite typeEntite,
                         float positionBulleX,
                         float positionBulleY,
                         float vitesseBulleX,
                         float vitesseBulleY,
                         TailleBulle tailleBulle) {
    Bulle *bulle;

    if (!etatJeu || !configuration) {
        return FAUX;
    }

    if (etatJeu->nombreBulles >= etatJeu->capaciteMaxBullesEnJeu) {
        int nouvelleCapacite = etatJeu->capaciteMaxBullesEnJeu == VALEUR_NULLE
                               ? CAPACITE_BULLES_INITIALE
                               : etatJeu->capaciteMaxBullesEnJeu * FACTEUR_CAPACITE_BULLES;

        if (!assurer_capacite_bulles(etatJeu, nouvelleCapacite)) {
            return FAUX;
        }
    }

    bulle = &etatJeu->bullesEnJeu[etatJeu->nombreBulles];
    configurer_bulle(bulle, tailleBulle, configuration);
    bulle->typeEntite = typeEntite;
    bulle->positionBulleX = positionBulleX;
    bulle->positionBulleY = positionBulleY;
    bulle->vitesseBulleX = vitesseBulleX;
    bulle->vitesseBulleY = vitesseBulleY;

    etatJeu->nombreBulles++;
    return VRAI;
}

static void supprimer_bulle(EtatJeu *etatJeu, int index) {
    if (!etatJeu || index < VALEUR_NULLE || index >= etatJeu->nombreBulles) {
        return;
    }

    etatJeu->nombreBulles--;
    if (index != etatJeu->nombreBulles) {
        etatJeu->bullesEnJeu[index] = etatJeu->bullesEnJeu[etatJeu->nombreBulles];
    }
}

static void faire_apparaitre_chapeau(EtatJeu *etatJeu,
                                     const ConfigurationJeu *configuration,
                                     const Bulle *bulleSource) {
    if (!etatJeu || !configuration || !bulleSource) {
        return;
    }

    etatJeu->chapeauEstVisible = VRAI;
    etatJeu->positionChapeauX = (int) (bulleSource->positionBulleX + bulleSource->largeur / DIVISEUR_CENTRE - etatJeu->largeurChapeau / DIVISEUR_CENTRE);
    etatJeu->positionChapeauY = (int) (bulleSource->positionBulleY + bulleSource->hauteur / DIVISEUR_CENTRE - etatJeu->hauteurChapeau / DIVISEUR_CENTRE);
    etatJeu->vitesseChapeauX = bulleSource->vitesseBulleX >= ZERO_FLOTTANT
                      ? vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_CHAPEAU_X)
                      : -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_CHAPEAU_X);
    etatJeu->vitesseChapeauY = -vitesse_verticale_relative(configuration, DIVISEUR_VITESSE_CHAPEAU_Y);
}

static void configurer_vol_de_mort(Bulle *bulle) {
    if (!bulle) {
        return;
    }

    bulle->typeEntite = ENTITE_VOL_DE_MORT;
    bulle->nombreCoupsAvantDivision = VIE_MAX_BOSS_VOL_DE_MORT;
}

static void declencher_explosion(EtatJeu *etatJeu, const Bulle *bulleSource) {
    if (!etatJeu || !bulleSource) {
        return;
    }

    etatJeu->explosionEstActive = VRAI;
    etatJeu->dureeExplosionRestanteImages = DUREE_EXPLOSION_IMAGES;
    etatJeu->positionExplosionX = (int) (bulleSource->positionBulleX + bulleSource->largeur / DIVISEUR_CENTRE - etatJeu->largeurExplosion / DIVISEUR_CENTRE);
    etatJeu->positionExplosionY = (int) (bulleSource->positionBulleY + bulleSource->hauteur / DIVISEUR_CENTRE - etatJeu->hauteurExplosion / DIVISEUR_CENTRE);
}

static void separer_bulle(EtatJeu *etatJeu, const ConfigurationJeu *configuration, int index) {
    Bulle bulleSource;
    TailleBulle tailleBulleEnfant;
    float centreBulleX;
    float centreBulleY;
    float vitesseHorizontaleBulleEnfant;
    int largeurBulleEnfant;
    int hauteurBulleEnfant;
    int indexNouvelleBulleGauche;
    int indexNouvelleBulleDroite;

    if (!etatJeu || !configuration || index < VALEUR_NULLE || index >= etatJeu->nombreBulles) {
        return;
    }

    bulleSource = etatJeu->bullesEnJeu[index];
    if (bulleSource.tailleBulle == BULLE_PETITE) {
        if (bulleSource.typeEntite == ENTITE_VIF_DOR) {
            faire_apparaitre_chapeau(etatJeu, configuration, &bulleSource);
        }
        supprimer_bulle(etatJeu, index);
        return;
    }

    tailleBulleEnfant = (TailleBulle) (bulleSource.tailleBulle + INDEX_SUIVANT);
    largeurBulleEnfant = configuration->largeurBulles[(int) tailleBulleEnfant];
    hauteurBulleEnfant = configuration->hauteurBulles[(int) tailleBulleEnfant];
    centreBulleX = bulleSource.positionBulleX + bulleSource.largeur / (float) DIVISEUR_CENTRE;
    centreBulleY = bulleSource.positionBulleY + bulleSource.hauteur / (float) DIVISEUR_CENTRE;
    vitesseHorizontaleBulleEnfant = calculer_vitesse_horizontale_bulle_enfant(configuration, tailleBulleEnfant);

    supprimer_bulle(etatJeu, index);
    ajouter_bulle(etatJeu,
                  configuration,
                  bulleSource.typeEntite,
                  centreBulleX - largeurBulleEnfant / (float) DIVISEUR_CENTRE - largeurBulleEnfant * DECALAGE_SEPARATION_BULLE,
                  centreBulleY - hauteurBulleEnfant / (float) DIVISEUR_CENTRE,
                  -vitesseHorizontaleBulleEnfant,
                  ZERO_FLOTTANT,
                  tailleBulleEnfant);
    indexNouvelleBulleGauche = etatJeu->nombreBulles - INDEX_SUIVANT;

    ajouter_bulle(etatJeu,
                  configuration,
                  bulleSource.typeEntite,
                  centreBulleX - largeurBulleEnfant / (float) DIVISEUR_CENTRE + largeurBulleEnfant * DECALAGE_SEPARATION_BULLE,
                  centreBulleY - hauteurBulleEnfant / (float) DIVISEUR_CENTRE,
                  vitesseHorizontaleBulleEnfant,
                  ZERO_FLOTTANT,
                  tailleBulleEnfant);
    indexNouvelleBulleDroite = etatJeu->nombreBulles - INDEX_SUIVANT;

    if (bulleSource.typeEntite == ENTITE_VOL_DE_MORT) {
        etatJeu->bullesEnJeu[indexNouvelleBulleGauche].nombreCoupsAvantDivision = VALEUR_NULLE;
        etatJeu->bullesEnJeu[indexNouvelleBulleDroite].nombreCoupsAvantDivision = VALEUR_NULLE;
    }
}

static void faire_lacher_bulle_mange_mort_moyenne(EtatJeu *etatJeu,
                                                  const ConfigurationJeu *configuration,
                                                  const Bulle *bulleSource) {
    float centreBulleX;
    float vitesseHorizontale;

    if (!etatJeu || !configuration || !bulleSource) {
        return;
    }

    centreBulleX = bulleSource->positionBulleX + bulleSource->largeur / (float) DIVISEUR_CENTRE;
    vitesseHorizontale = bulleSource->vitesseBulleX >= ZERO_FLOTTANT
                         ? -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_LACHER_BULLE)
                         : vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_LACHER_BULLE);

    ajouter_bulle(etatJeu,
                  configuration,
                  ENTITE_MANGE_MORT,
                  centreBulleX - configuration->largeurBulles[BULLE_MOYENNE] / (float) DIVISEUR_CENTRE,
                  bulleSource->positionBulleY + bulleSource->hauteur / (float) DIVISEUR_TIERS,
                  vitesseHorizontale,
                  -vitesse_verticale_relative(configuration, DIVISEUR_VITESSE_LACHER_BULLE_Y),
                  BULLE_MOYENNE);
}

static void mettre_a_jour_bulle(Bulle *bulle, const EtatJeu *etatJeu, const ConfigurationJeu *configuration) {
    float vitesseHorizontaleMin;

    if (!bulle || !etatJeu || !configuration) {
        return;
    }

    vitesseHorizontaleMin = vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_MIN_BULLE);
    bulle->vitesseBulleY += bulle->gravite;
    bulle->positionBulleX += bulle->vitesseBulleX;
    bulle->positionBulleY += bulle->vitesseBulleY;

    if (bulle->positionBulleY + bulle->hauteur >= etatJeu->positionSolY) {
        bulle->positionBulleY = (float) (etatJeu->positionSolY - bulle->hauteur);
        bulle->vitesseBulleY = bulle->rebondSol;
        bulle->vitesseBulleX *= bulle->attenuationX;

        if (bulle->vitesseBulleX > ZERO_FLOTTANT && bulle->vitesseBulleX < vitesseHorizontaleMin) {
            bulle->vitesseBulleX = vitesseHorizontaleMin;
        }
        if (bulle->vitesseBulleX < ZERO_FLOTTANT && bulle->vitesseBulleX > -vitesseHorizontaleMin) {
            bulle->vitesseBulleX = -vitesseHorizontaleMin;
        }
    }

    if (bulle->positionBulleX < etatJeu->limiteGaucheTerrain) {
        bulle->positionBulleX = (float) etatJeu->limiteGaucheTerrain;
        bulle->vitesseBulleX = -bulle->vitesseBulleX;
    }
    if (bulle->positionBulleX + bulle->largeur > etatJeu->limiteDroiteTerrain) {
        bulle->positionBulleX = (float) (etatJeu->limiteDroiteTerrain - bulle->largeur);
        bulle->vitesseBulleX = -bulle->vitesseBulleX;
    }
}

static void mettre_a_jour_toutes_les_bulles(EtatJeu *etatJeu, const ConfigurationJeu *configuration) {
    int indexBulle;

    for (indexBulle = INDEX_PREMIER; indexBulle < etatJeu->nombreBulles; indexBulle++) {
        mettre_a_jour_bulle(&etatJeu->bullesEnJeu[indexBulle], etatJeu, configuration);
    }
}

static int trouver_bulle_touchee_par_projectile(const EtatJeu *etatJeu) {
    int indexBulle;

    for (indexBulle = INDEX_PREMIER; indexBulle < etatJeu->nombreBulles; indexBulle++) {
        if (rectangles_en_collision((float) etatJeu->positionProjectileX,
                                    (float) etatJeu->positionProjectileY,
                                    etatJeu->largeurProjectile,
                                    etatJeu->hauteurProjectile,
                                    etatJeu->bullesEnJeu[indexBulle].positionBulleX,
                                    etatJeu->bullesEnJeu[indexBulle].positionBulleY,
                                    etatJeu->bullesEnJeu[indexBulle].largeur,
                                    etatJeu->bullesEnJeu[indexBulle].hauteur)) {
            return indexBulle;
        }
    }

    return INDEX_INVALIDE;
}

static int aura_ardente_touche_bulle(const EtatJeu *etatJeu,
                                     const ConfigurationJeu *configuration,
                                     const Bulle *bulle) {
    int margeAuraX;
    int margeAuraY;

    if (!etatJeu || !configuration || !bulle) {
        return FAUX;
    }

    margeAuraX = configuration->joueurLargeur / DIVISEUR_CINQUIEME;
    margeAuraY = configuration->joueurHauteur / DIVISEUR_CINQUIEME;
    if (margeAuraX < dimension_relative(configuration->largeurFenetre, DIVISEUR_AURA_MARGE)) {
        margeAuraX = dimension_relative(configuration->largeurFenetre, DIVISEUR_AURA_MARGE);
    }
    if (margeAuraY < dimension_relative(configuration->hauteurFenetre, DIVISEUR_AURA_MARGE)) {
        margeAuraY = dimension_relative(configuration->hauteurFenetre, DIVISEUR_AURA_MARGE);
    }

    return rectangles_en_collision((float) (etatJeu->positionJoueurX - margeAuraX),
                                   (float) (etatJeu->positionJoueurY - margeAuraY),
                                   configuration->joueurLargeur + margeAuraX * FACTEUR_CAPACITE_BULLES,
                                   configuration->joueurHauteur + margeAuraY * FACTEUR_CAPACITE_BULLES,
                                   bulle->positionBulleX,
                                   bulle->positionBulleY,
                                   bulle->largeur,
                                   bulle->hauteur);
}

static void gerer_coup_recu_par_bulle(EtatJeu *etatJeu,
                                      const ConfigurationJeu *configuration,
                                      int indexBulleTouchee) {
    Bulle bulleTouchee;

    if (!etatJeu || !configuration || indexBulleTouchee < VALEUR_NULLE || indexBulleTouchee >= etatJeu->nombreBulles) {
        return;
    }

    bulleTouchee = etatJeu->bullesEnJeu[indexBulleTouchee];
    declencher_explosion(etatJeu, &bulleTouchee);

    if (bulleTouchee.typeEntite == ENTITE_VOL_DE_MORT && bulleTouchee.nombreCoupsAvantDivision > VALEUR_NULLE) {
        etatJeu->bullesEnJeu[indexBulleTouchee].nombreCoupsAvantDivision--;
        etatJeu->scoreJoueur += POINTS_BOSS_TOUCHE;
        faire_lacher_bulle_mange_mort_moyenne(etatJeu, configuration, &bulleTouchee);
        if (etatJeu->bullesEnJeu[indexBulleTouchee].nombreCoupsAvantDivision > VALEUR_NULLE) {
            return;
        }
        bulleTouchee = etatJeu->bullesEnJeu[indexBulleTouchee];
    }

    etatJeu->scoreJoueur += calculer_score_bulle(&bulleTouchee);
    separer_bulle(etatJeu, configuration, indexBulleTouchee);
}

static int joueur_touche(const EtatJeu *etatJeu, const ConfigurationJeu *configuration) {
    int indexBulle;

    for (indexBulle = INDEX_PREMIER; indexBulle < etatJeu->nombreBulles; indexBulle++) {
        if (rectangles_en_collision((float) etatJeu->positionJoueurX,
                                    (float) etatJeu->positionJoueurY,
                                    configuration->joueurLargeur,
                                    configuration->joueurHauteur,
                                    etatJeu->bullesEnJeu[indexBulle].positionBulleX,
                                    etatJeu->bullesEnJeu[indexBulle].positionBulleY,
                                    etatJeu->bullesEnJeu[indexBulle].largeur,
                                    etatJeu->bullesEnJeu[indexBulle].hauteur)) {
            return VRAI;
        }
    }

    return FAUX;
}

static int joueur_touche_chapeau(const EtatJeu *etatJeu, const ConfigurationJeu *configuration) {
    if (!etatJeu || !configuration || !etatJeu->chapeauEstVisible) {
        return FAUX;
    }

    return rectangles_en_collision((float) etatJeu->positionJoueurX,
                                   (float) etatJeu->positionJoueurY,
                                   configuration->joueurLargeur,
                                   configuration->joueurHauteur,
                                   (float) etatJeu->positionChapeauX,
                                   (float) etatJeu->positionChapeauY,
                                   etatJeu->largeurChapeau,
                                   etatJeu->hauteurChapeau);
}

static void mettre_a_jour_chapeau(EtatJeu *etatJeu, const ConfigurationJeu *configuration) {
    if (!etatJeu || !configuration || !etatJeu->chapeauEstVisible) {
        return;
    }

    etatJeu->vitesseChapeauY += vitesse_verticale_relative(configuration, DIVISEUR_GRAVITE_CHAPEAU);
    etatJeu->positionChapeauX += (int) etatJeu->vitesseChapeauX;
    etatJeu->positionChapeauY += (int) etatJeu->vitesseChapeauY;

    if (etatJeu->positionChapeauY + etatJeu->hauteurChapeau >= etatJeu->positionSolY) {
        etatJeu->positionChapeauY = etatJeu->positionSolY - etatJeu->hauteurChapeau;
        etatJeu->vitesseChapeauY = -vitesse_verticale_relative(configuration, DIVISEUR_VITESSE_CHAPEAU_REBOND);
    }

    if (etatJeu->positionChapeauX < etatJeu->limiteGaucheTerrain) {
        etatJeu->positionChapeauX = etatJeu->limiteGaucheTerrain;
        etatJeu->vitesseChapeauX = -etatJeu->vitesseChapeauX;
    }
    if (etatJeu->positionChapeauX + etatJeu->largeurChapeau > etatJeu->limiteDroiteTerrain) {
        etatJeu->positionChapeauX = etatJeu->limiteDroiteTerrain - etatJeu->largeurChapeau;
        etatJeu->vitesseChapeauX = -etatJeu->vitesseChapeauX;
    }
}

static void vider_bulles(EtatJeu *etatJeu) {
    if (!etatJeu) {
        return;
    }

    etatJeu->nombreBulles = VALEUR_NULLE;
}

static int charger_niveau(EtatJeu *etatJeu, const ConfigurationJeu *configuration, int niveauActuel) {
    float largeur = (float) configuration->largeurFenetre;
    float hauteur = (float) configuration->hauteurFenetre;

    switch (niveauActuel) {
        case PREMIER_NIVEAU_JEU:
            return ajouter_bulle(etatJeu,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_1,
                                 hauteur * POSITION_Y_NIVEAU_1,
                                 -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_1),
                                 ZERO_FLOTTANT,
                                 BULLE_GRANDE);
        case VALEUR_DOUBLE:
            return ajouter_bulle(etatJeu,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_2,
                                 hauteur * POSITION_Y_NIVEAU_2,
                                 -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_2),
                                 ZERO_FLOTTANT,
                                 BULLE_TRES_GRANDE);
        case VALEUR_TRIPLE:
            return ajouter_bulle(etatJeu,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_3_GAUCHE,
                                 hauteur * POSITION_Y_NIVEAU_3_BULLES,
                                 vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_3_GAUCHE),
                                 ZERO_FLOTTANT,
                                 BULLE_GRANDE) &&
                   ajouter_bulle(etatJeu,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_3_DROITE,
                                 hauteur * POSITION_Y_NIVEAU_3_BULLES,
                                 -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_3_DROITE),
                                 ZERO_FLOTTANT,
                                 BULLE_TRES_GRANDE) &&
                   ajouter_bulle(etatJeu,
                                 configuration,
                                 ENTITE_VIF_DOR,
                                 largeur * POSITION_X_NIVEAU_3_VIF_DOR,
                                 hauteur * POSITION_Y_NIVEAU_3_VIF_DOR,
                                 vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_3_VIF_DOR),
                                 ZERO_FLOTTANT,
                                 BULLE_MOYENNE);
        case VALEUR_QUADRUPLE:
            return ajouter_bulle(etatJeu,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_4_GAUCHE,
                                 hauteur * POSITION_Y_NIVEAU_4_HAUT,
                                 vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_4_GAUCHE),
                                 ZERO_FLOTTANT,
                                 BULLE_TRES_GRANDE) &&
                   ajouter_bulle(etatJeu,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_4_CENTRE,
                                 hauteur * POSITION_Y_NIVEAU_4_CENTRE,
                                 -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_4_CENTRE),
                                 ZERO_FLOTTANT,
                                 BULLE_GRANDE) &&
                   ajouter_bulle(etatJeu,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_4_DROITE,
                                 hauteur * POSITION_Y_NIVEAU_4_HAUT,
                                 -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_4_DROITE),
                                 ZERO_FLOTTANT,
                                 BULLE_TRES_GRANDE) &&
                   ajouter_bulle(etatJeu,
                                 configuration,
                                 ENTITE_VIF_DOR,
                                 largeur * POSITION_X_NIVEAU_4_VIF_DOR,
                                 hauteur * POSITION_Y_NIVEAU_4_VIF_DOR,
                                 vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_4_VIF_DOR),
                                 ZERO_FLOTTANT,
                                 BULLE_MOYENNE);
        case VALEUR_QUINTUPLE:
            if (!ajouter_bulle(etatJeu,
                               configuration,
                               ENTITE_VOL_DE_MORT,
                               largeur * POSITION_X_NIVEAU_5_BOSS - configuration->largeurBulles[BULLE_TRES_GRANDE] / (float) DIVISEUR_CENTRE,
                               hauteur * POSITION_Y_NIVEAU_5_BOSS,
                               vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_5),
                               ZERO_FLOTTANT,
                               BULLE_TRES_GRANDE)) {
                return FAUX;
            }

            configurer_vol_de_mort(&etatJeu->bullesEnJeu[etatJeu->nombreBulles - INDEX_SUIVANT]);
            return VRAI;
        default:
            return FAUX;
    }
}

int initialiser_logique_jeu(EtatJeu *etatJeu, const ConfigurationJeu *configuration) {
    if (!etatJeu || !configuration) {
        return FAUX;
    }

    memset(etatJeu, VALEUR_NULLE, sizeof(*etatJeu));
    etatJeu->bullesEnJeu = NULL;
    etatJeu->capaciteMaxBullesEnJeu = VALEUR_NULLE;
    etatJeu->nombreBulles = VALEUR_NULLE;
    etatJeu->niveauActuel = PREMIER_NIVEAU_JEU;
    etatJeu->niveauMaximum = NIVEAU_MAXIMUM_JEU;
    etatJeu->scoreJoueur = VALEUR_NULLE;
    etatJeu->dureeRestanteAuraArdenteMs = VALEUR_NULLE;
    etatJeu->tempsRestantNiveauMs = DUREE_NIVEAU_SECONDES * VALEUR_MILLISECONDES_SECONDE;

    if (!assurer_capacite_bulles(etatJeu, CAPACITE_BULLES_INITIALE)) {
        return FAUX;
    }

    return reinitialiser_partie(etatJeu, configuration, PREMIER_NIVEAU_JEU);
}

int reinitialiser_partie(EtatJeu *etatJeu, const ConfigurationJeu *configuration, int niveauActuel) {
    if (!etatJeu || !configuration) {
        return FAUX;
    }

    if (niveauActuel < PREMIER_NIVEAU_JEU) {
        niveauActuel = PREMIER_NIVEAU_JEU;
    }
    if (niveauActuel > etatJeu->niveauMaximum) {
        niveauActuel = etatJeu->niveauMaximum;
    }

    copier_configuration_dans_etat(etatJeu, configuration);
    vider_bulles(etatJeu);
    etatJeu->niveauActuel = niveauActuel;

    etatJeu->positionJoueurX = configuration->largeurFenetre / DIVISEUR_CENTRE - configuration->joueurLargeur / DIVISEUR_CENTRE;
    etatJeu->positionJoueurY = etatJeu->positionSolY - configuration->joueurHauteur;
    etatJeu->projectileEstActif = FAUX;
    etatJeu->positionProjectileX = VALEUR_NULLE;
    etatJeu->positionProjectileY = VALEUR_NULLE;
    etatJeu->chapeauEstVisible = FAUX;
    etatJeu->positionChapeauX = VALEUR_NULLE;
    etatJeu->positionChapeauY = VALEUR_NULLE;
    etatJeu->vitesseChapeauX = ZERO_FLOTTANT;
    etatJeu->vitesseChapeauY = ZERO_FLOTTANT;
    etatJeu->explosionEstActive = FAUX;
    etatJeu->positionExplosionX = VALEUR_NULLE;
    etatJeu->positionExplosionY = VALEUR_NULLE;
    etatJeu->dureeExplosionRestanteImages = VALEUR_NULLE;
    etatJeu->auraArdenteEstActive = FAUX;
    etatJeu->dureeRestanteAuraArdenteMs = VALEUR_NULLE;
    etatJeu->partiePerdue = FAUX;
    etatJeu->partieGagnee = FAUX;
    etatJeu->tempsRestantNiveauMs = DUREE_NIVEAU_SECONDES * VALEUR_MILLISECONDES_SECONDE;

    return charger_niveau(etatJeu, configuration, niveauActuel);
}

void definir_pseudo_joueur(EtatJeu *etatJeu, const char *pseudoJoueur) {
    if (!etatJeu) {
        return;
    }

    if (!pseudoJoueur) {
        etatJeu->pseudoJoueur[CHAINE_DEBUT] = CARACTERE_FIN_CHAINE;
        return;
    }

    strncpy(etatJeu->pseudoJoueur, pseudoJoueur, TAILLE_PSEUDO_MAX - INDEX_SUIVANT);
    etatJeu->pseudoJoueur[TAILLE_PSEUDO_MAX - INDEX_SUIVANT] = CARACTERE_FIN_CHAINE;
}

void initialiser_commandes_jeu(CommandesJeu *commandes) {
    if (!commandes) {
        return;
    }

    commandes->deplacementHorizontal = VALEUR_NULLE;
    commandes->tirer = FAUX;
}

void mettre_a_jour_logique_jeu(EtatJeu *etatJeu,
                               const ConfigurationJeu *configuration,
                               const CommandesJeu *commandes) {
    int indexBulleTouchee;
    int indexBulle;

    if (!etatJeu || !configuration || !commandes || etatJeu->partiePerdue || etatJeu->partieGagnee) {
        return;
    }

    etatJeu->tempsRestantNiveauMs -= DUREE_IMAGE_LOGIQUE_MS;
    if (etatJeu->tempsRestantNiveauMs <= VALEUR_NULLE) {
        etatJeu->tempsRestantNiveauMs = VALEUR_NULLE;
        etatJeu->partiePerdue = VRAI;
        return;
    }

    etatJeu->positionJoueurX += commandes->deplacementHorizontal * etatJeu->vitesseJoueur;
    if (etatJeu->positionJoueurX < etatJeu->limiteGaucheTerrain) {
        etatJeu->positionJoueurX = etatJeu->limiteGaucheTerrain;
    }
    if (etatJeu->positionJoueurX + configuration->joueurLargeur > etatJeu->limiteDroiteTerrain) {
        etatJeu->positionJoueurX = etatJeu->limiteDroiteTerrain - configuration->joueurLargeur;
    }
    etatJeu->positionJoueurY = etatJeu->positionSolY - configuration->joueurHauteur;

    if (commandes->tirer && !etatJeu->projectileEstActif) {
        etatJeu->projectileEstActif = VRAI;
        etatJeu->positionProjectileX = etatJeu->positionJoueurX + configuration->joueurLargeur / DIVISEUR_CENTRE - etatJeu->largeurProjectile / DIVISEUR_CENTRE;
        etatJeu->positionProjectileY = etatJeu->positionJoueurY;
    }

    if (etatJeu->projectileEstActif) {
        etatJeu->positionProjectileY -= etatJeu->vitesseProjectile;
        if (etatJeu->positionProjectileY + etatJeu->hauteurProjectile < VALEUR_NULLE) {
            etatJeu->projectileEstActif = FAUX;
        }
    }

    mettre_a_jour_toutes_les_bulles(etatJeu, configuration);
    mettre_a_jour_chapeau(etatJeu, configuration);

    if (etatJeu->projectileEstActif) {
        indexBulleTouchee = trouver_bulle_touchee_par_projectile(etatJeu);
        if (indexBulleTouchee != INDEX_INVALIDE) {
            etatJeu->projectileEstActif = FAUX;
            gerer_coup_recu_par_bulle(etatJeu, configuration, indexBulleTouchee);
        }
    }

    if (etatJeu->explosionEstActive) {
        etatJeu->dureeExplosionRestanteImages--;
        if (etatJeu->dureeExplosionRestanteImages <= VALEUR_NULLE) {
            etatJeu->explosionEstActive = FAUX;
        }
    }

    if (joueur_touche_chapeau(etatJeu, configuration)) {
        etatJeu->chapeauEstVisible = FAUX;
        etatJeu->vitesseChapeauX = ZERO_FLOTTANT;
        etatJeu->vitesseChapeauY = ZERO_FLOTTANT;
        etatJeu->auraArdenteEstActive = VRAI;
        etatJeu->dureeRestanteAuraArdenteMs = DUREE_AURA_ARDENTE_MS;
        etatJeu->scoreJoueur += POINTS_BONUS_CHAPEAU;
    }

    if (etatJeu->auraArdenteEstActive) {
        /* L'aura détruit toute bulle qui s'approche trop près du joueur. */
        for (indexBulle = etatJeu->nombreBulles - INDEX_SUIVANT; indexBulle >= INDEX_PREMIER; indexBulle--) {
            if (aura_ardente_touche_bulle(etatJeu, configuration, &etatJeu->bullesEnJeu[indexBulle])) {
                gerer_coup_recu_par_bulle(etatJeu, configuration, indexBulle);
            }
        }

        etatJeu->dureeRestanteAuraArdenteMs -= DUREE_IMAGE_LOGIQUE_MS;
        if (etatJeu->dureeRestanteAuraArdenteMs <= VALEUR_NULLE) {
            etatJeu->dureeRestanteAuraArdenteMs = VALEUR_NULLE;
            etatJeu->auraArdenteEstActive = FAUX;
        }
    }

    if (joueur_touche(etatJeu, configuration)) {
        etatJeu->partiePerdue = VRAI;
    }

    if (etatJeu->nombreBulles == VALEUR_NULLE) {
        etatJeu->partieGagnee = VRAI;
    }
}

void fermer_logique_jeu(EtatJeu *etatJeu) {
    if (!etatJeu) {
        return;
    }

    free(etatJeu->bullesEnJeu);
    etatJeu->bullesEnJeu = NULL;
    etatJeu->nombreBulles = VALEUR_NULLE;
    etatJeu->capaciteMaxBullesEnJeu = VALEUR_NULLE;
    etatJeu->projectileEstActif = FAUX;
}
