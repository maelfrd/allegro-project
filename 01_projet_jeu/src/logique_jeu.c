#include "logique_jeu.h"

#include <stdlib.h>
#include <string.h>

#define DUREE_AURA_ARDENTE_MS 5000
#define NOMBRE_COUPS_AVANT_DIVISION_VOL_DE_MORT 5

static int calculer_score_bulle(const Bulle *bulle) {
    int score;

    if (!bulle) {
        return 0;
    }

    switch (bulle->taille) {
        case BULLE_TRES_GRANDE:
            score = 40;
            break;
        case BULLE_GRANDE:
            score = 70;
            break;
        case BULLE_MOYENNE:
            score = 110;
            break;
        case BULLE_PETITE:
            score = 160;
            break;
        case BULLE_TAILLES_TOTAL:
            score = 0;
            break;
    }

    if (bulle->type == ENTITE_VIF_DOR) {
        score += 140;
    }
    if (bulle->type == ENTITE_VOL_DE_MORT) {
        score += 220;
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
    bulle->nombreCoupsAvantDivision = 0;

    switch (taille) {
        case BULLE_TRES_GRANDE:
            bulle->gravite = 0.035f;
            bulle->rebondSol = -5.4f;
            bulle->attenuationX = 0.995f;
            break;
        case BULLE_GRANDE:
            bulle->gravite = 0.035f;
            bulle->rebondSol = -5.0f;
            bulle->attenuationX = 0.992f;
            break;
        case BULLE_MOYENNE:
            bulle->gravite = 0.030f;
            bulle->rebondSol = -4.5f;
            bulle->attenuationX = 0.989f;
            break;
        case BULLE_PETITE:
            bulle->gravite = 0.028f;
            bulle->rebondSol = -5.0f;
            bulle->attenuationX = 0.985f;
            break;
        case BULLE_TAILLES_TOTAL:
            break;
    }
}

static int assurer_capacite_bulles(EtatJeu *etat, int nouvelleCapacite) {
    Bulle *nouveauTableauBulles;

    if (nouvelleCapacite <= etat->capaciteBulles) {
        return 1;
    }

    nouveauTableauBulles = (Bulle *) realloc(etat->bulles, (size_t) nouvelleCapacite * sizeof(Bulle));
    if (!nouveauTableauBulles) {
        return 0;
    }

    etat->bulles = nouveauTableauBulles;
    etat->capaciteBulles = nouvelleCapacite;
    return 1;
}

static float calculer_vitesse_horizontale_bulle_enfant(TailleBulle taille) {
    switch (taille) {
        case BULLE_TRES_GRANDE:
            return 0.18f;
        case BULLE_GRANDE:
            return 0.20f;
        case BULLE_MOYENNE:
            return 0.22f;
        case BULLE_PETITE:
            return 0.25f;
        case BULLE_TAILLES_TOTAL:
            break;
    }

    return 0.20f;
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
        return 0;
    }

    if (etat->nbBulles >= etat->capaciteBulles) {
        int nouvelleCapacite = etat->capaciteBulles == 0 ? 4 : etat->capaciteBulles * 2;

        if (!assurer_capacite_bulles(etat, nouvelleCapacite)) {
            return 0;
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
    return 1;
}

static void supprimer_bulle(EtatJeu *etat, int index) {
    if (!etat || index < 0 || index >= etat->nbBulles) {
        return;
    }

    etat->nbBulles--;
    if (index != etat->nbBulles) {
        etat->bulles[index] = etat->bulles[etat->nbBulles];
    }
}

static void faire_apparaitre_chapeau(EtatJeu *etat, const Bulle *source) {
    if (!etat || !source) {
        return;
    }

    etat->chapeauVisible = 1;
    etat->chapeauX = (int) (source->x + source->largeur / 2 - etat->chapeauW / 2);
    etat->chapeauY = (int) (source->y + source->hauteur / 2 - etat->chapeauH / 2);
    etat->chapeauVx = source->vx >= 0.0f ? 0.45f : -0.45f;
    etat->chapeauVy = -2.8f;
}

static void configurer_vol_de_mort(Bulle *bulle) {
    if (!bulle) {
        return;
    }

    bulle->type = ENTITE_VOL_DE_MORT;
    bulle->nombreCoupsAvantDivision = NOMBRE_COUPS_AVANT_DIVISION_VOL_DE_MORT;
}

static void declencher_explosion(EtatJeu *etat, const Bulle *source) {
    if (!etat || !source) {
        return;
    }

    etat->explosionActive = 1;
    etat->explosionTimer = 18;
    etat->explosionX = (int) (source->x + source->largeur / 2 - etat->explosionW / 2);
    etat->explosionY = (int) (source->y + source->hauteur / 2 - etat->explosionH / 2);
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

    if (!etat || !configuration || index < 0 || index >= etat->nbBulles) {
        return;
    }

    source = etat->bulles[index];
    if (source.taille == BULLE_PETITE) {
        if (source.type == ENTITE_VIF_DOR) {
            faire_apparaitre_chapeau(etat, &source);
        }
        supprimer_bulle(etat, index);
        return;
    }

    tailleBulleEnfant = (TailleBulle) (source.taille + 1);
    largeurBulleEnfant = configuration->largeurBulles[(int) tailleBulleEnfant];
    hauteurBulleEnfant = configuration->hauteurBulles[(int) tailleBulleEnfant];
    centreBulleX = source.x + source.largeur / 2.0f;
    centreBulleY = source.y + source.hauteur / 2.0f;
    vitesseHorizontaleBulleEnfant = calculer_vitesse_horizontale_bulle_enfant(tailleBulleEnfant);

    supprimer_bulle(etat, index);
    ajouter_bulle(etat,
                  configuration,
                  source.type,
                  centreBulleX - largeurBulleEnfant / 2.0f - largeurBulleEnfant * 0.15f,
                  centreBulleY - hauteurBulleEnfant / 2.0f,
                  -vitesseHorizontaleBulleEnfant,
                  0.0f,
                  tailleBulleEnfant);
    indexNouvelleBulleGauche = etat->nbBulles - 1;

    ajouter_bulle(etat,
                  configuration,
                  source.type,
                  centreBulleX - largeurBulleEnfant / 2.0f + largeurBulleEnfant * 0.15f,
                  centreBulleY - hauteurBulleEnfant / 2.0f,
                  vitesseHorizontaleBulleEnfant,
                  0.0f,
                  tailleBulleEnfant);
    indexNouvelleBulleDroite = etat->nbBulles - 1;

    if (source.type == ENTITE_VOL_DE_MORT) {
        etat->bulles[indexNouvelleBulleGauche].nombreCoupsAvantDivision = 0;
        etat->bulles[indexNouvelleBulleDroite].nombreCoupsAvantDivision = 0;
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

    centreBulleX = source->x + source->largeur / 2.0f;
    vitesseHorizontale = source->vx >= 0.0f ? -0.35f : 0.35f;

    ajouter_bulle(etat,
                  configuration,
                  ENTITE_MANGE_MORT,
                  centreBulleX - configuration->largeurBulles[BULLE_MOYENNE] / 2.0f,
                  source->y + source->hauteur / 3.0f,
                  vitesseHorizontale,
                  -2.4f,
                  BULLE_MOYENNE);
}

static void update_bulle(Bulle *bulle, const EtatJeu *etat) {
    if (!bulle || !etat) {
        return;
    }

    bulle->vy += bulle->gravite;
    bulle->x += bulle->vx;
    bulle->y += bulle->vy;

    if (bulle->y + bulle->hauteur >= etat->groundY) {
        bulle->y = (float) (etat->groundY - bulle->hauteur);
        bulle->vy = bulle->rebondSol;
        bulle->vx *= bulle->attenuationX;

        if (bulle->vx > 0.0f && bulle->vx < 0.35f) {
            bulle->vx = 0.35f;
        }
        if (bulle->vx < 0.0f && bulle->vx > -0.35f) {
            bulle->vx = -0.35f;
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

static void update_bulles(EtatJeu *etat) {
    int i;

    for (i = 0; i < etat->nbBulles; i++) {
        update_bulle(&etat->bulles[i], etat);
    }
}

static int collision_projectile_bulles(const EtatJeu *etat) {
    int i;

    for (i = 0; i < etat->nbBulles; i++) {
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

    return -1;
}

static int aura_ardente_touche_bulle(const EtatJeu *etat,
                                     const ConfigurationJeu *configuration,
                                     const Bulle *bulle) {
    int margeAuraX;
    int margeAuraY;

    if (!etat || !configuration || !bulle) {
        return 0;
    }

    margeAuraX = configuration->joueurLargeur / 5;
    margeAuraY = configuration->joueurHauteur / 5;
    if (margeAuraX < 12) {
        margeAuraX = 12;
    }
    if (margeAuraY < 12) {
        margeAuraY = 12;
    }

    return rectangles_en_collision((float) (etat->x - margeAuraX),
                                   (float) (etat->y - margeAuraY),
                                   configuration->joueurLargeur + margeAuraX * 2,
                                   configuration->joueurHauteur + margeAuraY * 2,
                                   bulle->x,
                                   bulle->y,
                                   bulle->largeur,
                                   bulle->hauteur);
}

static void gerer_coup_recu_par_bulle(EtatJeu *etat,
                                      const ConfigurationJeu *configuration,
                                      int indexBulleTouchee) {
    Bulle bulleTouchee;

    if (!etat || !configuration || indexBulleTouchee < 0 || indexBulleTouchee >= etat->nbBulles) {
        return;
    }

    bulleTouchee = etat->bulles[indexBulleTouchee];
    declencher_explosion(etat, &bulleTouchee);

    if (bulleTouchee.type == ENTITE_VOL_DE_MORT && bulleTouchee.nombreCoupsAvantDivision > 0) {
        etat->bulles[indexBulleTouchee].nombreCoupsAvantDivision--;
        etat->score += 120;
        faire_lacher_bulle_mange_mort_moyenne(etat, configuration, &bulleTouchee);
        return;
    }

    etat->score += calculer_score_bulle(&bulleTouchee);
    separer_bulle(etat, configuration, indexBulleTouchee);
}

static int joueur_touche(const EtatJeu *etat, const ConfigurationJeu *configuration) {
    int i;

    for (i = 0; i < etat->nbBulles; i++) {
        if (rectangles_en_collision((float) etat->x,
                                    (float) etat->y,
                                    configuration->joueurLargeur,
                                    configuration->joueurHauteur,
                                    etat->bulles[i].x,
                                    etat->bulles[i].y,
                                    etat->bulles[i].largeur,
                                    etat->bulles[i].hauteur)) {
            return 1;
        }
    }

    return 0;
}

static int joueur_touche_chapeau(const EtatJeu *etat, const ConfigurationJeu *configuration) {
    if (!etat || !configuration || !etat->chapeauVisible) {
        return 0;
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

static void mettre_a_jour_chapeau(EtatJeu *etat) {
    if (!etat || !etat->chapeauVisible) {
        return;
    }

    etat->chapeauVy += 0.06f;
    etat->chapeauX += (int) etat->chapeauVx;
    etat->chapeauY += (int) etat->chapeauVy;

    if (etat->chapeauY + etat->chapeauH >= etat->groundY) {
        etat->chapeauY = etat->groundY - etat->chapeauH;
        etat->chapeauVy = -2.6f;
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

    etat->nbBulles = 0;
}

static int charger_niveau(EtatJeu *etat, const ConfigurationJeu *configuration, int niveau) {
    float largeur = (float) configuration->largeurFenetre;
    float hauteur = (float) configuration->hauteurFenetre;

    switch (niveau) {
        case 1:
            return ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * 0.68f,
                                 hauteur * 0.14f,
                                 -0.18f,
                                 0.0f,
                                 BULLE_GRANDE);
        case 2:
            return ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * 0.72f,
                                 hauteur * 0.11f,
                                 -0.22f,
                                 0.0f,
                                 BULLE_TRES_GRANDE);
        case 3:
            return ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * 0.28f,
                                 hauteur * 0.11f,
                                 0.20f,
                                 0.0f,
                                 BULLE_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * 0.72f,
                                 hauteur * 0.11f,
                                 -0.28f,
                                 0.0f,
                                 BULLE_TRES_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_VIF_DOR,
                                 largeur * 0.48f,
                                 hauteur * 0.08f,
                                 0.32f,
                                 0.0f,
                                 BULLE_MOYENNE);
        case 4:
            return ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * 0.20f,
                                 hauteur * 0.10f,
                                 0.28f,
                                 0.0f,
                                 BULLE_TRES_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * 0.52f,
                                 hauteur * 0.17f,
                                 -0.24f,
                                 0.0f,
                                 BULLE_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_MANGE_MORT,
                                 largeur * 0.78f,
                                 hauteur * 0.10f,
                                 -0.35f,
                                  0.0f,
                                 BULLE_TRES_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_VIF_DOR,
                                 largeur * 0.50f,
                                 hauteur * 0.07f,
                                 0.36f,
                                 0.0f,
                                 BULLE_MOYENNE);
        case 5:
            if (!ajouter_bulle(etat,
                               configuration,
                               ENTITE_VOL_DE_MORT,
                               largeur * 0.50f - configuration->largeurBulles[BULLE_TRES_GRANDE] / 2.0f,
                               hauteur * 0.10f,
                               0.28f,
                               0.0f,
                               BULLE_TRES_GRANDE)) {
                return 0;
            }

            configurer_vol_de_mort(&etat->bulles[etat->nbBulles - 1]);
            return 1;
        default:
            return 0;
    }
}

int initialiser_logique_jeu(EtatJeu *etat, const ConfigurationJeu *configuration) {
    if (!etat || !configuration) {
        return 0;
    }

    memset(etat, 0, sizeof(*etat));
    etat->bulles = NULL;
    etat->capaciteBulles = 0;
    etat->nbBulles = 0;
    etat->niveau = 1;
    etat->niveauMaximum = 5;
    etat->score = 0;
    etat->dureeRestanteAuraArdenteMs = 0;

    if (!assurer_capacite_bulles(etat, 4)) {
        return 0;
    }

    return reinitialiser_partie(etat, configuration, 1);
}

int reinitialiser_partie(EtatJeu *etat, const ConfigurationJeu *configuration, int niveau) {
    if (!etat || !configuration) {
        return 0;
    }

    if (niveau < 1) {
        niveau = 1;
    }
    if (niveau > etat->niveauMaximum) {
        niveau = etat->niveauMaximum;
    }

    copier_configuration_dans_etat(etat, configuration);
    vider_bulles(etat);
    etat->niveau = niveau;

    etat->x = configuration->largeurFenetre / 2 - configuration->joueurLargeur / 2;
    etat->y = etat->groundY - configuration->joueurHauteur;
    etat->projectileActive = 0;
    etat->projectileX = 0;
    etat->projectileY = 0;
    etat->chapeauVisible = 0;
    etat->chapeauX = 0;
    etat->chapeauY = 0;
    etat->chapeauVx = 0.0f;
    etat->chapeauVy = 0.0f;
    etat->explosionActive = 0;
    etat->explosionX = 0;
    etat->explosionY = 0;
    etat->explosionTimer = 0;
    etat->auraArdenteActive = 0;
    etat->dureeRestanteAuraArdenteMs = 0;
    etat->perdu = 0;
    etat->gagne = 0;

    return charger_niveau(etat, configuration, niveau);
}

void definir_pseudo_joueur(EtatJeu *etat, const char *pseudo) {
    if (!etat) {
        return;
    }

    if (!pseudo) {
        etat->pseudo[0] = '\0';
        return;
    }

    strncpy(etat->pseudo, pseudo, TAILLE_PSEUDO_MAX - 1);
    etat->pseudo[TAILLE_PSEUDO_MAX - 1] = '\0';
}

void initialiser_commandes_jeu(CommandesJeu *commandes) {
    if (!commandes) {
        return;
    }

    commandes->deplacementHorizontal = 0;
    commandes->tirer = 0;
}

void mettre_a_jour_logique_jeu(EtatJeu *etat,
                               const ConfigurationJeu *configuration,
                               const CommandesJeu *commandes) {
    int indexBulleTouchee;
    int indexBulle;

    if (!etat || !configuration || !commandes || etat->perdu || etat->gagne) {
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
        etat->projectileActive = 1;
        etat->projectileX = etat->x + configuration->joueurLargeur / 2 - etat->projectileW / 2;
        etat->projectileY = etat->y;
    }

    if (etat->projectileActive) {
        etat->projectileY -= etat->projectileSpeed;
        if (etat->projectileY + etat->projectileH < 0) {
            etat->projectileActive = 0;
        }
    }

    update_bulles(etat);
    mettre_a_jour_chapeau(etat);

    if (etat->projectileActive) {
        indexBulleTouchee = collision_projectile_bulles(etat);
        if (indexBulleTouchee != -1) {
            etat->projectileActive = 0;
            gerer_coup_recu_par_bulle(etat, configuration, indexBulleTouchee);
        }
    }

    if (etat->explosionActive) {
        etat->explosionTimer--;
        if (etat->explosionTimer <= 0) {
            etat->explosionActive = 0;
        }
    }

    if (joueur_touche_chapeau(etat, configuration)) {
        etat->chapeauVisible = 0;
        etat->chapeauVx = 0.0f;
        etat->chapeauVy = 0.0f;
        etat->auraArdenteActive = 1;
        etat->dureeRestanteAuraArdenteMs = DUREE_AURA_ARDENTE_MS;
        etat->score += 250;
    }

    if (etat->auraArdenteActive) {
        /* L'aura détruit toute bulle qui s'approche trop près du joueur. */
        for (indexBulle = etat->nbBulles - 1; indexBulle >= 0; indexBulle--) {
            if (aura_ardente_touche_bulle(etat, configuration, &etat->bulles[indexBulle])) {
                gerer_coup_recu_par_bulle(etat, configuration, indexBulle);
            }
        }

        etat->dureeRestanteAuraArdenteMs -= 16;
        if (etat->dureeRestanteAuraArdenteMs <= 0) {
            etat->dureeRestanteAuraArdenteMs = 0;
            etat->auraArdenteActive = 0;
        }
    }

    if (joueur_touche(etat, configuration)) {
        etat->perdu = 1;
    }

    if (etat->nbBulles == 0) {
        etat->gagne = 1;
    }
}

void fermer_logique_jeu(EtatJeu *etat) {
    if (!etat) {
        return;
    }

    free(etat->bulles);
    etat->bulles = NULL;
    etat->nbBulles = 0;
    etat->capaciteBulles = 0;
    etat->projectileActive = 0;
}
