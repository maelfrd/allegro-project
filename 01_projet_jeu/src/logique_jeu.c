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

static int dimension_relative(int reference, int diviseur) {
    int valeur;

    if (reference <= VALEUR_NULLE || diviseur <= VALEUR_NULLE) {
        return VALEUR_NULLE;
    }

    valeur = reference / diviseur;
    if (valeur <= VALEUR_NULLE) {
        valeur = reference / reference;
    }

    return valeur;
}

static int calculer_score_bulle(const Bulle *bulle) {
    int score;

    if (!bulle) {
        return VALEUR_NULLE;
    }

    switch (bulle->taille) {
        case BULLE_TRES_GRANDE:
            score = SCORE_BULLE_TRES_GRANDE;
            break;
        case BULLE_GRANDE:
            score = SCORE_BULLE_GRANDE;
            break;
        case BULLE_MOYENNE:
            score = SCORE_BULLE_MOYENNE;
            break;
        case BULLE_PETITE:
            score = SCORE_BULLE_PETITE;
            break;
        case BULLE_TAILLES_TOTAL:
            score = VALEUR_NULLE;
            break;
    }

    if (bulle->type == ENTITE_VIF_DOR) {
        score += BONUS_SCORE_VIF_DOR;
    }
    if (bulle->type == ENTITE_VOL_DE_MORT) {
        score += BONUS_SCORE_VOL_DE_MORT;
    }

    return score;
}

static int rectangles_en_collision(float x1, float y1, int largeurRectangle1, int hauteurRectangle1,
                                   float x2, float y2, int largeurRectangle2, int hauteurRectangle2) {
    return (x1 < x2 + largeurRectangle2 &&
            x1 + largeurRectangle1 > x2 &&
            y1 < y2 + hauteurRectangle2 &&
            y1 + hauteurRectangle1 > y2);
}

static void copier_configuration_dans_etat(EtatJeu *etat, const ConfigurationJeu *configuration) {
    etat->groundY = configuration->groundY;
    etat->leftLimit = configuration->leftLimit;
    etat->rightLimit = configuration->rightLimit;
    etat->speed = configuration->vitesseJoueur;
    etat->projectileW = configuration->projectileLargeur;
    etat->projectileH = configuration->projectileHauteur;
    etat->projectileSpeed = configuration->projectileVitesse;
    etat->chapeauW = configuration->chapeauLargeur;
    etat->chapeauH = configuration->chapeauHauteur;
    etat->explosionW = configuration->explosionLargeur;
    etat->explosionH = configuration->explosionHauteur;
}

static void configurer_bulle(Bulle *bulle, TailleBulle taille, const ConfigurationJeu *configuration) {
    bulle->type = ENTITE_MANGE_MORT;
    bulle->taille = taille;
    bulle->largeur = configuration->largeurBulles[(int) taille];
    bulle->hauteur = configuration->hauteurBulles[(int) taille];
    bulle->nombreCoupsAvantDivision = VALEUR_NULLE;

    switch (taille) {
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

static int assurer_capacite_bulles(EtatJeu *etat, int nouvelleCapacite) {
    Bulle *nouveauTableauBulles;

    if (nouvelleCapacite <= etat->capaciteBulles) {
        return VRAI;
    }

    nouveauTableauBulles = (Bulle *) realloc(etat->bulles, (size_t) nouvelleCapacite * sizeof(Bulle));
    if (!nouveauTableauBulles) {
        return FAUX;
    }

    etat->bulles = nouveauTableauBulles;
    etat->capaciteBulles = nouvelleCapacite;
    return VRAI;
}

static float calculer_vitesse_horizontale_bulle_enfant(const ConfigurationJeu *configuration,
                                                       TailleBulle taille) {
    switch (taille) {
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

static int ajouter_bulle(EtatJeu *etat,
                         const ConfigurationJeu *configuration,
                         TypeEntite type,
                         float x,
                         float y,
                         float vx,
                         float vy,
                         TailleBulle taille) {
    Bulle *bulle;

    if (!etat || !configuration) {
        return FAUX;
    }

    if (etat->nbBulles >= etat->capaciteBulles) {
        int nouvelleCapacite = etat->capaciteBulles == VALEUR_NULLE
                               ? CAPACITE_BULLES_INITIALE
                               : etat->capaciteBulles * FACTEUR_CAPACITE_BULLES;

        if (!assurer_capacite_bulles(etat, nouvelleCapacite)) {
            return FAUX;
        }
    }

    bulle = &etat->bulles[etat->nbBulles];
    configurer_bulle(bulle, taille, configuration);
    bulle->type = type;
    bulle->x = x;
    bulle->y = y;
    bulle->vx = vx;
    bulle->vy = vy;

    etat->nbBulles++;
    return VRAI;
}

static void supprimer_bulle(EtatJeu *etat, int index) {
    if (!etat || index < VALEUR_NULLE || index >= etat->nbBulles) {
        return;
    }

    etat->nbBulles--;
    if (index != etat->nbBulles) {
        etat->bulles[index] = etat->bulles[etat->nbBulles];
    }
}

static void faire_apparaitre_chapeau(EtatJeu *etat,
                                     const ConfigurationJeu *configuration,
                                     const Bulle *source) {
    if (!etat || !configuration || !source) {
        return;
    }

    etat->chapeauVisible = VRAI;
    etat->chapeauX = (int) (source->x + source->largeur / DIVISEUR_CENTRE - etat->chapeauW / DIVISEUR_CENTRE);
    etat->chapeauY = (int) (source->y + source->hauteur / DIVISEUR_CENTRE - etat->chapeauH / DIVISEUR_CENTRE);
    etat->chapeauVx = source->vx >= ZERO_FLOTTANT
                      ? vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_CHAPEAU_X)
                      : -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_CHAPEAU_X);
    etat->chapeauVy = -vitesse_verticale_relative(configuration, DIVISEUR_VITESSE_CHAPEAU_Y);
}

static void configurer_vol_de_mort(Bulle *bulle) {
    if (!bulle) {
        return;
    }

    bulle->type = ENTITE_VOL_DE_MORT;
    bulle->nombreCoupsAvantDivision = VIE_MAX_BOSS_VOL_DE_MORT;
}

static void declencher_explosion(EtatJeu *etat, const Bulle *source) {
    if (!etat || !source) {
        return;
    }

    etat->explosionActive = VRAI;
    etat->explosionTimer = DUREE_EXPLOSION_IMAGES;
    etat->explosionX = (int) (source->x + source->largeur / DIVISEUR_CENTRE - etat->explosionW / DIVISEUR_CENTRE);
    etat->explosionY = (int) (source->y + source->hauteur / DIVISEUR_CENTRE - etat->explosionH / DIVISEUR_CENTRE);
}

static void separer_bulle(EtatJeu *etat, const ConfigurationJeu *configuration, int index) {
    Bulle source;
    TailleBulle tailleBulleEnfant;
    float centreBulleX;
    float centreBulleY;
    float vitesseHorizontaleBulleEnfant;
    int largeurBulleEnfant;
    int hauteurBulleEnfant;
    int indexNouvelleBulleGauche;
    int indexNouvelleBulleDroite;

    if (!etat || !configuration || index < VALEUR_NULLE || index >= etat->nbBulles) {
        return;
    }

    source = etat->bulles[index];
    if (source.taille == BULLE_PETITE) {
        if (source.type == ENTITE_VIF_DOR) {
            faire_apparaitre_chapeau(etat, configuration, &source);
        }
        supprimer_bulle(etat, index);
        return;
    }

    tailleBulleEnfant = (TailleBulle) (source.taille + INDEX_SUIVANT);
    largeurBulleEnfant = configuration->largeurBulles[(int) tailleBulleEnfant];
    hauteurBulleEnfant = configuration->hauteurBulles[(int) tailleBulleEnfant];
    centreBulleX = source.x + source.largeur / (float) DIVISEUR_CENTRE;
    centreBulleY = source.y + source.hauteur / (float) DIVISEUR_CENTRE;
    vitesseHorizontaleBulleEnfant = calculer_vitesse_horizontale_bulle_enfant(configuration, tailleBulleEnfant);

    supprimer_bulle(etat, index);
    ajouter_bulle(etat,
                  configuration,
                  source.type,
                  centreBulleX - largeurBulleEnfant / (float) DIVISEUR_CENTRE - largeurBulleEnfant * DECALAGE_SEPARATION_BULLE,
                  centreBulleY - hauteurBulleEnfant / (float) DIVISEUR_CENTRE,
                  -vitesseHorizontaleBulleEnfant,
                  ZERO_FLOTTANT,
                  tailleBulleEnfant);
    indexNouvelleBulleGauche = etat->nbBulles - INDEX_SUIVANT;

    ajouter_bulle(etat,
                  configuration,
                  source.type,
                  centreBulleX - largeurBulleEnfant / (float) DIVISEUR_CENTRE + largeurBulleEnfant * DECALAGE_SEPARATION_BULLE,
                  centreBulleY - hauteurBulleEnfant / (float) DIVISEUR_CENTRE,
                  vitesseHorizontaleBulleEnfant,
                  ZERO_FLOTTANT,
                  tailleBulleEnfant);
    indexNouvelleBulleDroite = etat->nbBulles - INDEX_SUIVANT;

    if (source.type == ENTITE_VOL_DE_MORT) {
        etat->bulles[indexNouvelleBulleGauche].nombreCoupsAvantDivision = VALEUR_NULLE;
        etat->bulles[indexNouvelleBulleDroite].nombreCoupsAvantDivision = VALEUR_NULLE;
    }
}

static void faire_lacher_bulle_mange_mort_moyenne(EtatJeu *etat,
                                                  const ConfigurationJeu *configuration,
                                                  const Bulle *source) {
    float centreBulleX;
    float vitesseHorizontale;

    if (!etat || !configuration || !source) {
        return;
    }

    centreBulleX = source->x + source->largeur / (float) DIVISEUR_CENTRE;
    vitesseHorizontale = source->vx >= ZERO_FLOTTANT
                         ? -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_LACHER_BULLE)
                         : vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_LACHER_BULLE);

    ajouter_bulle(etat,
                  configuration,
                  ENTITE_MANGE_MORT,
                  centreBulleX - configuration->largeurBulles[BULLE_MOYENNE] / (float) DIVISEUR_CENTRE,
                  source->y + source->hauteur / (float) DIVISEUR_TIERS,
                  vitesseHorizontale,
                  -vitesse_verticale_relative(configuration, DIVISEUR_VITESSE_LACHER_BULLE_Y),
                  BULLE_MOYENNE);
}

static void mettre_a_jour_bulle(Bulle *bulle, const EtatJeu *etat, const ConfigurationJeu *configuration) {
    float vitesseHorizontaleMin;

    if (!bulle || !etat || !configuration) {
        return;
    }

    vitesseHorizontaleMin = vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_MIN_BULLE);
    bulle->vy += bulle->gravite;
    bulle->x += bulle->vx;
    bulle->y += bulle->vy;

    if (bulle->y + bulle->hauteur >= etat->groundY) {
        bulle->y = (float) (etat->groundY - bulle->hauteur);
        bulle->vy = bulle->rebondSol;
        bulle->vx *= bulle->attenuationX;

        if (bulle->vx > ZERO_FLOTTANT && bulle->vx < vitesseHorizontaleMin) {
            bulle->vx = vitesseHorizontaleMin;
        }
        if (bulle->vx < ZERO_FLOTTANT && bulle->vx > -vitesseHorizontaleMin) {
            bulle->vx = -vitesseHorizontaleMin;
        }
    }

    if (bulle->x < etat->leftLimit) {
        bulle->x = (float) etat->leftLimit;
        bulle->vx = -bulle->vx;
    }
    if (bulle->x + bulle->largeur > etat->rightLimit) {
        bulle->x = (float) (etat->rightLimit - bulle->largeur);
        bulle->vx = -bulle->vx;
    }
}

static void mettre_a_jour_toutes_les_bulles(EtatJeu *etat, const ConfigurationJeu *configuration) {
    int i;

    for (i = INDEX_PREMIER; i < etat->nbBulles; i++) {
        mettre_a_jour_bulle(&etat->bulles[i], etat, configuration);
    }
}

static int trouver_bulle_touchee_par_projectile(const EtatJeu *etat) {
    int i;

    for (i = INDEX_PREMIER; i < etat->nbBulles; i++) {
        if (rectangles_en_collision((float) etat->projectileX,
                                    (float) etat->projectileY,
                                    etat->projectileW,
                                    etat->projectileH,
                                    etat->bulles[i].x,
                                    etat->bulles[i].y,
                                    etat->bulles[i].largeur,
                                    etat->bulles[i].hauteur)) {
            return i;
        }
    }

    return INDEX_INVALIDE;
}

static int aura_ardente_touche_bulle(const EtatJeu *etat,
                                     const ConfigurationJeu *configuration,
                                     const Bulle *bulle) {
    int margeAuraX;
    int margeAuraY;

    if (!etat || !configuration || !bulle) {
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

    return rectangles_en_collision((float) (etat->x - margeAuraX),
                                   (float) (etat->y - margeAuraY),
                                   configuration->joueurLargeur + margeAuraX * FACTEUR_CAPACITE_BULLES,
                                   configuration->joueurHauteur + margeAuraY * FACTEUR_CAPACITE_BULLES,
                                   bulle->x,
                                   bulle->y,
                                   bulle->largeur,
                                   bulle->hauteur);
}

static void gerer_coup_recu_par_bulle(EtatJeu *etat,
                                      const ConfigurationJeu *configuration,
                                      int indexBulleTouchee) {
    Bulle bulleTouchee;

    if (!etat || !configuration || indexBulleTouchee < VALEUR_NULLE || indexBulleTouchee >= etat->nbBulles) {
        return;
    }

    bulleTouchee = etat->bulles[indexBulleTouchee];
    declencher_explosion(etat, &bulleTouchee);

    if (bulleTouchee.type == ENTITE_VOL_DE_MORT && bulleTouchee.nombreCoupsAvantDivision > VALEUR_NULLE) {
        etat->bulles[indexBulleTouchee].nombreCoupsAvantDivision--;
        etat->score += POINTS_BOSS_TOUCHE;
        faire_lacher_bulle_mange_mort_moyenne(etat, configuration, &bulleTouchee);
        if (etat->bulles[indexBulleTouchee].nombreCoupsAvantDivision > VALEUR_NULLE) {
            return;
        }
        bulleTouchee = etat->bulles[indexBulleTouchee];
    }

    etat->score += calculer_score_bulle(&bulleTouchee);
    separer_bulle(etat, configuration, indexBulleTouchee);
}

static int joueur_touche(const EtatJeu *etat, const ConfigurationJeu *configuration) {
    int i;

    for (i = INDEX_PREMIER; i < etat->nbBulles; i++) {
        if (rectangles_en_collision((float) etat->x,
                                    (float) etat->y,
                                    configuration->joueurLargeur,
                                    configuration->joueurHauteur,
                                    etat->bulles[i].x,
                                    etat->bulles[i].y,
                                    etat->bulles[i].largeur,
                                    etat->bulles[i].hauteur)) {
            return VRAI;
        }
    }

    return FAUX;
}

static int joueur_touche_chapeau(const EtatJeu *etat, const ConfigurationJeu *configuration) {
    if (!etat || !configuration || !etat->chapeauVisible) {
        return FAUX;
    }

    return rectangles_en_collision((float) etat->x,
                                   (float) etat->y,
                                   configuration->joueurLargeur,
                                   configuration->joueurHauteur,
                                   (float) etat->chapeauX,
                                   (float) etat->chapeauY,
                                   etat->chapeauW,
                                   etat->chapeauH);
}

static void mettre_a_jour_chapeau(EtatJeu *etat, const ConfigurationJeu *configuration) {
    if (!etat || !configuration || !etat->chapeauVisible) {
        return;
    }

    etat->chapeauVy += vitesse_verticale_relative(configuration, DIVISEUR_GRAVITE_CHAPEAU);
    etat->chapeauX += (int) etat->chapeauVx;
    etat->chapeauY += (int) etat->chapeauVy;

    if (etat->chapeauY + etat->chapeauH >= etat->groundY) {
        etat->chapeauY = etat->groundY - etat->chapeauH;
        etat->chapeauVy = -vitesse_verticale_relative(configuration, DIVISEUR_VITESSE_CHAPEAU_REBOND);
    }

    if (etat->chapeauX < etat->leftLimit) {
        etat->chapeauX = etat->leftLimit;
        etat->chapeauVx = -etat->chapeauVx;
    }
    if (etat->chapeauX + etat->chapeauW > etat->rightLimit) {
        etat->chapeauX = etat->rightLimit - etat->chapeauW;
        etat->chapeauVx = -etat->chapeauVx;
    }
}

static void vider_bulles(EtatJeu *etat) {
    if (!etat) {
        return;
    }

    etat->nbBulles = VALEUR_NULLE;
}

static int charger_niveau(EtatJeu *etat, const ConfigurationJeu *configuration, int niveau) {
    float largeur = (float) configuration->largeurFenetre;
    float hauteur = (float) configuration->hauteurFenetre;

    switch (niveau) {
        case PREMIER_NIVEAU_JEU:
            return ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_1,
                                 hauteur * POSITION_Y_NIVEAU_1,
                                 -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_1),
                                 ZERO_FLOTTANT,
                                 BULLE_GRANDE);
        case VALEUR_DOUBLE:
            return ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_2,
                                 hauteur * POSITION_Y_NIVEAU_2,
                                 -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_2),
                                 ZERO_FLOTTANT,
                                 BULLE_TRES_GRANDE);
        case VALEUR_TRIPLE:
            return ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_3_GAUCHE,
                                 hauteur * POSITION_Y_NIVEAU_3_BULLES,
                                 vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_3_GAUCHE),
                                 ZERO_FLOTTANT,
                                 BULLE_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_3_DROITE,
                                 hauteur * POSITION_Y_NIVEAU_3_BULLES,
                                 -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_3_DROITE),
                                 ZERO_FLOTTANT,
                                 BULLE_TRES_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_VIF_DOR,
                                 largeur * POSITION_X_NIVEAU_3_VIF_DOR,
                                 hauteur * POSITION_Y_NIVEAU_3_VIF_DOR,
                                 vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_3_VIF_DOR),
                                 ZERO_FLOTTANT,
                                 BULLE_MOYENNE);
        case VALEUR_QUADRUPLE:
            return ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_4_GAUCHE,
                                 hauteur * POSITION_Y_NIVEAU_4_HAUT,
                                 vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_4_GAUCHE),
                                 ZERO_FLOTTANT,
                                 BULLE_TRES_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_4_CENTRE,
                                 hauteur * POSITION_Y_NIVEAU_4_CENTRE,
                                 -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_4_CENTRE),
                                 ZERO_FLOTTANT,
                                 BULLE_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * POSITION_X_NIVEAU_4_DROITE,
                                 hauteur * POSITION_Y_NIVEAU_4_HAUT,
                                 -vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_4_DROITE),
                                 ZERO_FLOTTANT,
                                 BULLE_TRES_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_VIF_DOR,
                                 largeur * POSITION_X_NIVEAU_4_VIF_DOR,
                                 hauteur * POSITION_Y_NIVEAU_4_VIF_DOR,
                                 vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_4_VIF_DOR),
                                 ZERO_FLOTTANT,
                                 BULLE_MOYENNE);
        case VALEUR_QUINTUPLE:
            if (!ajouter_bulle(etat,
                               configuration,
                               ENTITE_VOL_DE_MORT,
                               largeur * POSITION_X_NIVEAU_5_BOSS - configuration->largeurBulles[BULLE_TRES_GRANDE] / (float) DIVISEUR_CENTRE,
                               hauteur * POSITION_Y_NIVEAU_5_BOSS,
                               vitesse_horizontale_relative(configuration, DIVISEUR_VITESSE_NIVEAU_5),
                               ZERO_FLOTTANT,
                               BULLE_TRES_GRANDE)) {
                return FAUX;
            }

            configurer_vol_de_mort(&etat->bulles[etat->nbBulles - INDEX_SUIVANT]);
            return VRAI;
        default:
            return FAUX;
    }
}

int initialiser_logique_jeu(EtatJeu *etat, const ConfigurationJeu *configuration) {
    if (!etat || !configuration) {
        return FAUX;
    }

    memset(etat, VALEUR_NULLE, sizeof(*etat));
    etat->bulles = NULL;
    etat->capaciteBulles = VALEUR_NULLE;
    etat->nbBulles = VALEUR_NULLE;
    etat->niveau = PREMIER_NIVEAU_JEU;
    etat->niveauMaximum = NIVEAU_MAXIMUM_JEU;
    etat->score = VALEUR_NULLE;
    etat->dureeRestanteAuraArdenteMs = VALEUR_NULLE;
    etat->tempsRestantNiveauMs = DUREE_NIVEAU_SECONDES * VALEUR_MILLISECONDES_SECONDE;

    if (!assurer_capacite_bulles(etat, CAPACITE_BULLES_INITIALE)) {
        return FAUX;
    }

    return reinitialiser_partie(etat, configuration, PREMIER_NIVEAU_JEU);
}

int reinitialiser_partie(EtatJeu *etat, const ConfigurationJeu *configuration, int niveau) {
    if (!etat || !configuration) {
        return FAUX;
    }

    if (niveau < PREMIER_NIVEAU_JEU) {
        niveau = PREMIER_NIVEAU_JEU;
    }
    if (niveau > etat->niveauMaximum) {
        niveau = etat->niveauMaximum;
    }

    copier_configuration_dans_etat(etat, configuration);
    vider_bulles(etat);
    etat->niveau = niveau;

    etat->x = configuration->largeurFenetre / DIVISEUR_CENTRE - configuration->joueurLargeur / DIVISEUR_CENTRE;
    etat->y = etat->groundY - configuration->joueurHauteur;
    etat->projectileActive = FAUX;
    etat->projectileX = VALEUR_NULLE;
    etat->projectileY = VALEUR_NULLE;
    etat->chapeauVisible = FAUX;
    etat->chapeauX = VALEUR_NULLE;
    etat->chapeauY = VALEUR_NULLE;
    etat->chapeauVx = ZERO_FLOTTANT;
    etat->chapeauVy = ZERO_FLOTTANT;
    etat->explosionActive = FAUX;
    etat->explosionX = VALEUR_NULLE;
    etat->explosionY = VALEUR_NULLE;
    etat->explosionTimer = VALEUR_NULLE;
    etat->auraArdenteActive = FAUX;
    etat->dureeRestanteAuraArdenteMs = VALEUR_NULLE;
    etat->perdu = FAUX;
    etat->gagne = FAUX;
    etat->tempsRestantNiveauMs = DUREE_NIVEAU_SECONDES * VALEUR_MILLISECONDES_SECONDE;

    return charger_niveau(etat, configuration, niveau);
}

void definir_pseudo_joueur(EtatJeu *etat, const char *pseudo) {
    if (!etat) {
        return;
    }

    if (!pseudo) {
        etat->pseudo[CHAINE_DEBUT] = CARACTERE_FIN_CHAINE;
        return;
    }

    strncpy(etat->pseudo, pseudo, TAILLE_PSEUDO_MAX - INDEX_SUIVANT);
    etat->pseudo[TAILLE_PSEUDO_MAX - INDEX_SUIVANT] = CARACTERE_FIN_CHAINE;
}

void initialiser_commandes_jeu(CommandesJeu *commandes) {
    if (!commandes) {
        return;
    }

    commandes->deplacementHorizontal = VALEUR_NULLE;
    commandes->tirer = FAUX;
}

void mettre_a_jour_logique_jeu(EtatJeu *etat,
                               const ConfigurationJeu *configuration,
                               const CommandesJeu *commandes) {
    int indexBulleTouchee;
    int indexBulle;

    if (!etat || !configuration || !commandes || etat->perdu || etat->gagne) {
        return;
    }

    etat->tempsRestantNiveauMs -= DUREE_IMAGE_LOGIQUE_MS;
    if (etat->tempsRestantNiveauMs <= VALEUR_NULLE) {
        etat->tempsRestantNiveauMs = VALEUR_NULLE;
        etat->perdu = VRAI;
        return;
    }

    etat->x += commandes->deplacementHorizontal * etat->speed;
    if (etat->x < etat->leftLimit) {
        etat->x = etat->leftLimit;
    }
    if (etat->x + configuration->joueurLargeur > etat->rightLimit) {
        etat->x = etat->rightLimit - configuration->joueurLargeur;
    }
    etat->y = etat->groundY - configuration->joueurHauteur;

    if (commandes->tirer && !etat->projectileActive) {
        etat->projectileActive = VRAI;
        etat->projectileX = etat->x + configuration->joueurLargeur / DIVISEUR_CENTRE - etat->projectileW / DIVISEUR_CENTRE;
        etat->projectileY = etat->y;
    }

    if (etat->projectileActive) {
        etat->projectileY -= etat->projectileSpeed;
        if (etat->projectileY + etat->projectileH < VALEUR_NULLE) {
            etat->projectileActive = FAUX;
        }
    }

    mettre_a_jour_toutes_les_bulles(etat, configuration);
    mettre_a_jour_chapeau(etat, configuration);

    if (etat->projectileActive) {
        indexBulleTouchee = trouver_bulle_touchee_par_projectile(etat);
        if (indexBulleTouchee != INDEX_INVALIDE) {
            etat->projectileActive = FAUX;
            gerer_coup_recu_par_bulle(etat, configuration, indexBulleTouchee);
        }
    }

    if (etat->explosionActive) {
        etat->explosionTimer--;
        if (etat->explosionTimer <= VALEUR_NULLE) {
            etat->explosionActive = FAUX;
        }
    }

    if (joueur_touche_chapeau(etat, configuration)) {
        etat->chapeauVisible = FAUX;
        etat->chapeauVx = ZERO_FLOTTANT;
        etat->chapeauVy = ZERO_FLOTTANT;
        etat->auraArdenteActive = VRAI;
        etat->dureeRestanteAuraArdenteMs = DUREE_AURA_ARDENTE_MS;
        etat->score += POINTS_BONUS_CHAPEAU;
    }

    if (etat->auraArdenteActive) {
        /* L'aura détruit toute bulle qui s'approche trop près du joueur. */
        for (indexBulle = etat->nbBulles - INDEX_SUIVANT; indexBulle >= INDEX_PREMIER; indexBulle--) {
            if (aura_ardente_touche_bulle(etat, configuration, &etat->bulles[indexBulle])) {
                gerer_coup_recu_par_bulle(etat, configuration, indexBulle);
            }
        }

        etat->dureeRestanteAuraArdenteMs -= DUREE_IMAGE_LOGIQUE_MS;
        if (etat->dureeRestanteAuraArdenteMs <= VALEUR_NULLE) {
            etat->dureeRestanteAuraArdenteMs = VALEUR_NULLE;
            etat->auraArdenteActive = FAUX;
        }
    }

    if (joueur_touche(etat, configuration)) {
        etat->perdu = VRAI;
    }

    if (etat->nbBulles == VALEUR_NULLE) {
        etat->gagne = VRAI;
    }
}

void fermer_logique_jeu(EtatJeu *etat) {
    if (!etat) {
        return;
    }

    free(etat->bulles);
    etat->bulles = NULL;
    etat->nbBulles = VALEUR_NULLE;
    etat->capaciteBulles = VALEUR_NULLE;
    etat->projectileActive = FAUX;
}
