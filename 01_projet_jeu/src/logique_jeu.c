#include "logique_jeu.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

static int collision_rect(float x1, float y1, int w1, int h1,
                          float x2, float y2, int w2, int h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}

static void repousser_bulle_vers_haut(Bulle *bulle);

static void modifier_score(EtatJeu *etat, int variation) {
    if (!etat) {
        return;
    }

    etat->score += variation;
    if (etat->score < 0) {
        etat->score = 0;
    }
}

static void copier_configuration_dans_etat(EtatJeu *etat, const ConfigurationJeu *configuration) {
    etat->groundY = configuration->groundY;
    etat->leftLimit = configuration->leftLimit;
    etat->rightLimit = configuration->rightLimit;
    etat->vitesse = configuration->vitesseJoueur;
    etat->tirLargeur = configuration->projectileLargeur;
    etat->tirHauteur = configuration->projectileHauteur;
    etat->vitesseTir = configuration->projectileVitesse;
    etat->bonusLargeur = configuration->chapeauLargeur;
    etat->bonusHauteur = configuration->chapeauHauteur;
    etat->explosionLargeur = configuration->explosionLargeur;
    etat->explosionHauteur = configuration->explosionHauteur;
}

static int duree_bonus_ms(TypeBonus typeBonus) {
    if (typeBonus == BONUS_DOUBLE_TIR) {
        return DUREE_DOUBLE_TIR_MS;
    }
    if (typeBonus == BONUS_AUCUN) {
        return 0;
    }

    return DUREE_POUVOIR_MS;
}

static int points_pour_bulle(const Bulle *bulle) {
    int points;

    if (!bulle) {
        return 0;
    }

    switch (bulle->taille) {
        case BULLE_TRES_GRANDE:
            points = 15;
            break;
        case BULLE_GRANDE:
            points = 25;
            break;
        case BULLE_MOYENNE:
            points = 40;
            break;
        case BULLE_PETITE:
            points = 60;
            break;
        case BULLE_TAILLES_TOTAL:
            points = 0;
            break;
    }

    if (bulle->type == ENTITE_VIF_DOR) {
        points += 20;
    } else if (bulle->type == ENTITE_VOL_DE_MORT) {
        points += 40;
    }

    return points;
}

static int bonus_vitesse_niveau(const EtatJeu *etat) {
    int secondesRestantes;

    if (!etat) {
        return 0;
    }

    secondesRestantes = etat->tempsRestantMs / 1000;
    return 100 + secondesRestantes * 2;
}

static void appliquer_parametres_taille_bulle(Bulle *bulle,
                                              TailleBulle taille,
                                              const ConfigurationJeu *configuration) {
    bulle->taille = taille;
    bulle->largeur = configuration->largeurBulles[(int) taille];
    bulle->hauteur = configuration->hauteurBulles[(int) taille];

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

static void configurer_bulle(Bulle *bulle, TailleBulle taille, const ConfigurationJeu *configuration) {
    bulle->type = ENTITE_MANGE_MORT;
    appliquer_parametres_taille_bulle(bulle, taille, configuration);
}

static int reserver_bulles(EtatJeu *etat, int nouvelleCapacite) {
    Bulle *nouvellesBulles;

    if (nouvelleCapacite <= etat->capaciteBulles) {
        return 1;
    }

    nouvellesBulles = (Bulle *) realloc(etat->bulles, (size_t) nouvelleCapacite * sizeof(Bulle));
    if (!nouvellesBulles) {
        return 0;
    }

    etat->bulles = nouvellesBulles;
    etat->capaciteBulles = nouvelleCapacite;
    return 1;
}

static float vitesse_horizontale_fille(TailleBulle taille) {
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

        if (!reserver_bulles(etat, nouvelleCapacite)) {
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

static void faire_apparaitre_bonus_precis(EtatJeu *etat,
                                          const ConfigurationJeu *configuration,
                                          const Bulle *source,
                                          TypeBonus typeBonus) {
    if (!etat || !configuration || !source) {
        return;
    }

    etat->bonusVisible = 1;
    etat->typeBonusQuiTombe = (int) typeBonus;
    if (typeBonus == BONUS_CHAPEAU) {
        etat->bonusLargeur = configuration->chapeauLargeur;
        etat->bonusHauteur = configuration->chapeauHauteur;
    } else if (typeBonus == BONUS_DOUBLE_TIR) {
        etat->bonusLargeur = configuration->doubleTirLargeur;
        etat->bonusHauteur = configuration->doubleTirHauteur;
    } else {
        etat->bonusLargeur = configuration->feuLargeur;
        etat->bonusHauteur = configuration->feuHauteur;
    }
    etat->bonusX = (int) (source->x + source->largeur / 2 - etat->bonusLargeur / 2);
    etat->bonusY = (int) (source->y + source->hauteur / 2 - etat->bonusHauteur / 2);
    etat->bonusVx = source->vx >= 0.0f ? 0.45f : -0.45f;
    etat->bonusVy = -2.8f;
}

static void faire_apparaitre_bonus(EtatJeu *etat,
                                   const ConfigurationJeu *configuration,
                                   const Bulle *source) {
    TypeBonus typeBonus;

    if (!etat || !configuration || !source) {
        return;
    }

    typeBonus = (rand() % 2) == 0 ? BONUS_CHAPEAU : BONUS_LANCE_FLAMMES;
    faire_apparaitre_bonus_precis(etat, configuration, source, typeBonus);
}

static void declencher_explosion(EtatJeu *etat, const Bulle *source) {
    if (!etat || !source) {
        return;
    }

    etat->explosionActive = 1;
    etat->explosionTimer = 18;
    etat->explosionX = (int) (source->x + source->largeur / 2 - etat->explosionLargeur / 2);
    etat->explosionY = (int) (source->y + source->hauteur / 2 - etat->explosionHauteur / 2);
}

static int ajouter_eclair(EtatJeu *etat, const ConfigurationJeu *configuration, const Bulle *source) {
    Eclair *eclair;
    int largeur;
    int hauteur;

    if (!etat || !configuration || !source || etat->nbEclairs >= MAX_ECLAIRS) {
        return 0;
    }

    largeur = configuration->largeurFenetre / 120;
    hauteur = configuration->hauteurFenetre / 9;
    if (largeur < 8) {
        largeur = 8;
    }
    if (hauteur < 42) {
        hauteur = 42;
    }

    eclair = &etat->eclairs[etat->nbEclairs];
    eclair->largeur = largeur;
    eclair->hauteur = hauteur;
    eclair->x = source->x + source->largeur / 2.0f - largeur / 2.0f;
    eclair->y = source->y + source->hauteur;
    eclair->vitesseY = configuration->hauteurFenetre / 90.0f;
    if (eclair->vitesseY < 6.5f) {
        eclair->vitesseY = 6.5f;
    }

    etat->nbEclairs++;
    return 1;
}

static void supprimer_eclair(EtatJeu *etat, int index) {
    if (!etat || index < 0 || index >= etat->nbEclairs) {
        return;
    }

    etat->nbEclairs--;
    if (index != etat->nbEclairs) {
        etat->eclairs[index] = etat->eclairs[etat->nbEclairs];
    }
}

static void declencher_eclairs_mangemorts(EtatJeu *etat, const ConfigurationJeu *configuration) {
    int i;
    int nbBullesInitial;

    if (!etat || !configuration) {
        return;
    }

    nbBullesInitial = etat->nbBulles;
    for (i = 0; i < nbBullesInitial; i++) {
        if (etat->bulles[i].type == ENTITE_MANGE_MORT) {
            ajouter_eclair(etat, configuration, &etat->bulles[i]);
        }
    }
}

static void faire_lacher_mangemort_boss(EtatJeu *etat, const ConfigurationJeu *configuration, const Bulle *source) {
    float centreX;
    float centreY;
    float vitesse;

    if (!etat || !configuration || !source) {
        return;
    }

    centreX = source->x + source->largeur / 2.0f;
    centreY = source->y + source->hauteur / 2.0f;
    vitesse = vitesse_horizontale_fille(BULLE_MOYENNE);
    if (etat->bossTouchesAvantReduction % 2 == 0) {
        vitesse = -vitesse;
    }

    ajouter_bulle(etat,
                  configuration,
                  ENTITE_MANGE_MORT,
                  centreX - configuration->largeurBulles[BULLE_MOYENNE] / 2.0f,
                  centreY - configuration->hauteurBulles[BULLE_MOYENNE] / 2.0f,
                  vitesse,
                  -2.4f,
                  BULLE_MOYENNE);
}

static void reduire_boss_final(EtatJeu *etat, const ConfigurationJeu *configuration, int index) {
    Bulle *boss;
    TailleBulle nouvelleTaille;
    float centreX;
    float centreY;
    float direction;

    if (!etat || !configuration || index < 0 || index >= etat->nbBulles) {
        return;
    }

    boss = &etat->bulles[index];
    if (boss->taille == BULLE_PETITE) {
        supprimer_bulle(etat, index);
        return;
    }

    nouvelleTaille = (TailleBulle) (boss->taille + 1);
    centreX = boss->x + boss->largeur / 2.0f;
    centreY = boss->y + boss->hauteur / 2.0f;
    direction = boss->vx >= 0.0f ? 1.0f : -1.0f;

    appliquer_parametres_taille_bulle(boss, nouvelleTaille, configuration);
    boss->type = ENTITE_VOL_DE_MORT;
    boss->x = centreX - boss->largeur / 2.0f;
    boss->y = centreY - boss->hauteur / 2.0f;
    boss->vx = vitesse_horizontale_fille(nouvelleTaille) * direction;
    boss->vy = 0.0f;
}

static void separer_bulle(EtatJeu *etat, const ConfigurationJeu *configuration, int index) {
    Bulle source;
    TailleBulle nouvelleTaille;
    float centreX;
    float centreY;
    float vxFille;
    int largeurFille;
    int hauteurFille;

    if (!etat || !configuration || index < 0 || index >= etat->nbBulles) {
        return;
    }

    source = etat->bulles[index];
    if (source.taille == BULLE_PETITE) {
        if (source.type == ENTITE_VIF_DOR) {
            faire_apparaitre_bonus(etat, configuration, &source);
        }
        supprimer_bulle(etat, index);
        return;
    }

    if (source.type == ENTITE_VIF_DOR && source.taille == BULLE_MOYENNE) {
        faire_apparaitre_bonus_precis(etat, configuration, &source, BONUS_DOUBLE_TIR);
    }

    nouvelleTaille = (TailleBulle) (source.taille + 1);
    largeurFille = configuration->largeurBulles[(int) nouvelleTaille];
    hauteurFille = configuration->hauteurBulles[(int) nouvelleTaille];
    centreX = source.x + source.largeur / 2.0f;
    centreY = source.y + source.hauteur / 2.0f;
    vxFille = vitesse_horizontale_fille(nouvelleTaille);

    supprimer_bulle(etat, index);
    ajouter_bulle(etat,
                  configuration,
                  source.type,
                  centreX - largeurFille / 2.0f - largeurFille * 0.15f,
                  centreY - hauteurFille / 2.0f,
                  -vxFille,
                  0.0f,
                  nouvelleTaille);
    ajouter_bulle(etat,
                  configuration,
                  source.type,
                  centreX - largeurFille / 2.0f + largeurFille * 0.15f,
                  centreY - hauteurFille / 2.0f,
                  vxFille,
                  0.0f,
                  nouvelleTaille);
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

static void update_eclairs(EtatJeu *etat, const ConfigurationJeu *configuration) {
    int i = 0;

    if (!etat || !configuration) {
        return;
    }

    while (i < etat->nbEclairs) {
        etat->eclairs[i].y += etat->eclairs[i].vitesseY;

        if (collision_rect((float) etat->x,
                           (float) etat->y,
                           configuration->joueurLargeur,
                           configuration->joueurHauteur,
                           etat->eclairs[i].x,
                           etat->eclairs[i].y,
                           etat->eclairs[i].largeur,
                           etat->eclairs[i].hauteur)) {
            etat->perdu = 1;
            return;
        }

        if (etat->eclairs[i].y > configuration->hauteurFenetre) {
            supprimer_eclair(etat, i);
            continue;
        }

        i++;
    }
}

static int collision_tir_bulles(const EtatJeu *etat,
                                int tirX,
                                int tirY,
                                int tirLargeur,
                                int tirHauteur) {
    int i;

    for (i = 0; i < etat->nbBulles; i++) {
        if (collision_rect((float) tirX,
                           (float) tirY,
                           tirLargeur,
                           tirHauteur,
                           etat->bulles[i].x,
                           etat->bulles[i].y,
                           etat->bulles[i].largeur,
                           etat->bulles[i].hauteur)) {
            return i;
        }
    }

    return -1;
}

static void gerer_collision_tir(EtatJeu *etat,
                                const ConfigurationJeu *configuration,
                                int *tirActif,
                                int tirX,
                                int tirY,
                                int tirLargeur,
                                int tirHauteur) {
    int idxTouchee;
    Bulle touchee;

    if (!etat || !configuration || !tirActif || !(*tirActif)) {
        return;
    }

    idxTouchee = collision_tir_bulles(etat, tirX, tirY, tirLargeur, tirHauteur);
    if (idxTouchee == -1) {
        return;
    }

    touchee = etat->bulles[idxTouchee];
    modifier_score(etat, points_pour_bulle(&touchee));

    if (etat->typeBonusActif == BONUS_LANCE_FLAMMES) {
        repousser_bulle_vers_haut(&etat->bulles[idxTouchee]);
    } else {
        *tirActif = 0;
        declencher_explosion(etat, &touchee);
    }

    if (etat->typeBonusActif == BONUS_CHAPEAU) {
        supprimer_bulle(etat, idxTouchee);
    } else if (etat->typeBonusActif != BONUS_LANCE_FLAMMES &&
               touchee.type == ENTITE_VOL_DE_MORT) {
        if (etat->bossTouchesAvantReduction > 0) {
            etat->bossTouchesAvantReduction--;
            faire_lacher_mangemort_boss(etat, configuration, &touchee);
        } else {
            reduire_boss_final(etat, configuration, idxTouchee);
        }
    } else if (etat->typeBonusActif == BONUS_AUCUN ||
               etat->typeBonusActif == BONUS_DOUBLE_TIR) {
        separer_bulle(etat, configuration, idxTouchee);
    }
}

static int joueur_touche(const EtatJeu *etat, const ConfigurationJeu *configuration) {
    int i;

    for (i = 0; i < etat->nbBulles; i++) {
        if (collision_rect((float) etat->x,
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

static int joueur_touche_bonus(const EtatJeu *etat, const ConfigurationJeu *configuration) {
    if (!etat || !configuration || !etat->bonusVisible) {
        return 0;
    }

    return collision_rect((float) etat->x,
                          (float) etat->y,
                          configuration->joueurLargeur,
                          configuration->joueurHauteur,
                          (float) etat->bonusX,
                          (float) etat->bonusY,
                          etat->bonusLargeur,
                          etat->bonusHauteur);
}

static void mettre_a_jour_bonus_sol(EtatJeu *etat) {
    if (!etat || !etat->bonusVisible) {
        return;
    }

    etat->bonusVy += 0.06f;
    etat->bonusX += (int) etat->bonusVx;
    etat->bonusY += (int) etat->bonusVy;

    if (etat->bonusY + etat->bonusHauteur >= etat->groundY) {
        etat->bonusY = etat->groundY - etat->bonusHauteur;
        etat->bonusVy = -2.6f;
    }

    if (etat->bonusX < etat->leftLimit) {
        etat->bonusX = etat->leftLimit;
        etat->bonusVx = -etat->bonusVx;
    }
    if (etat->bonusX + etat->bonusLargeur > etat->rightLimit) {
        etat->bonusX = etat->rightLimit - etat->bonusLargeur;
        etat->bonusVx = -etat->bonusVx;
    }
}

static void activer_pouvoir(EtatJeu *etat, TypeBonus typeBonus) {
    if (!etat) {
        return;
    }

    etat->typeBonusActif = (int) typeBonus;
    etat->tempsBonusActifMs = duree_bonus_ms(typeBonus);
    etat->tirActif = 0;
    etat->tirSecondaireActif = 0;
}

static void mettre_a_jour_pouvoir(EtatJeu *etat, int deltaTempsMs) {
    if (!etat || etat->typeBonusActif == BONUS_AUCUN) {
        return;
    }

    etat->tempsBonusActifMs -= deltaTempsMs;
    if (etat->tempsBonusActifMs <= 0) {
        etat->tempsBonusActifMs = 0;
        activer_pouvoir(etat, BONUS_AUCUN);
    }
}

static void repousser_bulle_vers_haut(Bulle *bulle) {
    if (!bulle) {
        return;
    }

    bulle->vy = bulle->rebondSol * 1.35f;
    if (bulle->vy > -6.8f) {
        bulle->vy = -6.8f;
    }

    bulle->y -= bulle->hauteur * 0.18f;
    if (bulle->y < 0.0f) {
        bulle->y = 0.0f;
    }
}

static void vider_eclairs(EtatJeu *etat) {
    if (!etat) {
        return;
    }

    etat->nbEclairs = 0;
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
                                 BULLE_TRES_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_VIF_DOR,
                                 largeur * 0.28f,
                                 hauteur * 0.09f,
                                 0.30f,
                                 0.0f,
                                 BULLE_MOYENNE);
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
            return ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_VOL_DE_MORT,
                                 largeur * 0.46f,
                                 hauteur * 0.07f,
                                 0.24f,
                                 0.0f,
                                 BULLE_TRES_GRANDE) &&
                   ajouter_bulle(etat,
                                 configuration,
                                 ENTITE_VIF_DOR,
                                 largeur * 0.22f,
                                 hauteur * 0.08f,
                                 0.34f,
                                 0.0f,
                                 BULLE_MOYENNE);
        default:
            return 0;
    }
}

int initialiser_logique_jeu(EtatJeu *etat, const ConfigurationJeu *configuration) {
    static int hasardInitialise = 0;

    if (!etat || !configuration) {
        return 0;
    }

    if (!hasardInitialise) {
        srand((unsigned int) time(NULL));
        hasardInitialise = 1;
    }

    memset(etat, 0, sizeof(*etat));
    etat->bulles = NULL;
    etat->capaciteBulles = 0;
    etat->nbBulles = 0;
    etat->niveau = 1;
    etat->niveauMaximum = 5;

    if (!reserver_bulles(etat, 4)) {
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
    vider_eclairs(etat);
    etat->niveau = niveau;

    etat->x = configuration->largeurFenetre / 2 - configuration->joueurLargeur / 2;
    etat->y = etat->groundY - configuration->joueurHauteur;
    etat->tirActif = 0;
    etat->tirX = 0;
    etat->tirY = 0;
    etat->tirSecondaireActif = 0;
    etat->tirSecondaireX = 0;
    etat->tirSecondaireY = 0;
    etat->bonusVisible = 0;
    etat->bonusX = 0;
    etat->bonusY = 0;
    etat->bonusLargeur = configuration->chapeauLargeur;
    etat->bonusHauteur = configuration->chapeauHauteur;
    etat->bonusVx = 0.0f;
    etat->bonusVy = 0.0f;
    etat->explosionActive = 0;
    etat->explosionX = 0;
    etat->explosionY = 0;
    etat->explosionTimer = 0;
    etat->typeBonusQuiTombe = BONUS_AUCUN;
    etat->typeBonusActif = BONUS_AUCUN;
    etat->tempsBonusActifMs = 0;
    etat->tempsRestantMs = DUREE_NIVEAU_MS;
    etat->prochainDeclenchementEclairMs = DUREE_NIVEAU_MS - INTERVALLE_ECLAIR_MS;
    etat->bossTouchesAvantReduction = niveau == 5 ? 5 : 0;
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
                               const CommandesJeu *commandes,
                               int deltaTempsMs) {
    int ecartDoubleTir;

    if (!etat || !configuration || !commandes || etat->perdu || etat->gagne) {
        return;
    }

    if (deltaTempsMs < 1) {
        deltaTempsMs = 1;
    }

    etat->x += commandes->deplacementHorizontal * etat->vitesse;
    if (etat->x < etat->leftLimit) {
        etat->x = etat->leftLimit;
    }
    if (etat->x + configuration->joueurLargeur > etat->rightLimit) {
        etat->x = etat->rightLimit - configuration->joueurLargeur;
    }
    etat->y = etat->groundY - configuration->joueurHauteur;

    mettre_a_jour_pouvoir(etat, deltaTempsMs);

    if (etat->typeBonusActif == BONUS_LANCE_FLAMMES) {
        if (commandes->tirer) {
            etat->tirActif = 1;
            etat->tirLargeur = configuration->feuLargeur;
            etat->tirHauteur = configuration->feuHauteur;
            etat->tirX = etat->x + configuration->joueurLargeur / 2 - etat->tirLargeur / 2;
            etat->tirY = etat->y - etat->tirHauteur + configuration->joueurHauteur / 3;
        } else {
            etat->tirActif = 0;
        }
        etat->tirSecondaireActif = 0;
    } else if (etat->typeBonusActif == BONUS_DOUBLE_TIR &&
               commandes->tirer &&
               !etat->tirActif &&
               !etat->tirSecondaireActif) {
        ecartDoubleTir = configuration->joueurLargeur / 5;
        if (ecartDoubleTir < configuration->projectileLargeur) {
            ecartDoubleTir = configuration->projectileLargeur;
        }

        etat->tirActif = 1;
        etat->tirSecondaireActif = 1;
        etat->tirLargeur = configuration->projectileLargeur;
        etat->tirHauteur = configuration->projectileHauteur;
        etat->tirX = etat->x + configuration->joueurLargeur / 2 - etat->tirLargeur / 2 - ecartDoubleTir;
        etat->tirY = etat->y;
        etat->tirSecondaireX = etat->x + configuration->joueurLargeur / 2 - etat->tirLargeur / 2 + ecartDoubleTir;
        etat->tirSecondaireY = etat->y;
    } else if (commandes->tirer && !etat->tirActif) {
        etat->tirActif = 1;
        etat->tirLargeur = configuration->projectileLargeur;
        etat->tirHauteur = configuration->projectileHauteur;
        etat->tirX = etat->x + configuration->joueurLargeur / 2 - etat->tirLargeur / 2;
        etat->tirY = etat->y;
    }

    if (etat->tirActif && etat->typeBonusActif != BONUS_LANCE_FLAMMES) {
        etat->tirY -= etat->vitesseTir;
        if (etat->tirY + etat->tirHauteur < 0) {
            etat->tirActif = 0;
        }
    }
    if (etat->tirSecondaireActif) {
        etat->tirSecondaireY -= etat->vitesseTir;
        if (etat->tirSecondaireY + etat->tirHauteur < 0) {
            etat->tirSecondaireActif = 0;
        }
    }

    etat->tempsRestantMs -= deltaTempsMs;
    if (etat->tempsRestantMs < 0) {
        etat->tempsRestantMs = 0;
    }

    while (etat->niveau >= 3 &&
           etat->prochainDeclenchementEclairMs > 0 &&
           etat->tempsRestantMs <= etat->prochainDeclenchementEclairMs) {
        declencher_eclairs_mangemorts(etat, configuration);
        etat->prochainDeclenchementEclairMs -= INTERVALLE_ECLAIR_MS;
    }

    update_bulles(etat);
    mettre_a_jour_bonus_sol(etat);
    update_eclairs(etat, configuration);

    if (etat->perdu) {
        modifier_score(etat, -80);
        return;
    }

    gerer_collision_tir(etat,
                        configuration,
                        &etat->tirActif,
                        etat->tirX,
                        etat->tirY,
                        etat->tirLargeur,
                        etat->tirHauteur);
    if (!etat->perdu) {
        gerer_collision_tir(etat,
                            configuration,
                            &etat->tirSecondaireActif,
                            etat->tirSecondaireX,
                            etat->tirSecondaireY,
                            etat->tirLargeur,
                            etat->tirHauteur);
    }

    if (etat->explosionActive) {
        etat->explosionTimer--;
        if (etat->explosionTimer <= 0) {
            etat->explosionActive = 0;
        }
    }

    if (joueur_touche_bonus(etat, configuration)) {
        TypeBonus bonusRecupere = (TypeBonus) etat->typeBonusQuiTombe;

        etat->bonusVisible = 0;
        etat->bonusVx = 0.0f;
        etat->bonusVy = 0.0f;
        etat->typeBonusQuiTombe = BONUS_AUCUN;
        modifier_score(etat, 25);
        activer_pouvoir(etat, bonusRecupere);
    }

    if (joueur_touche(etat, configuration)) {
        etat->perdu = 1;
        modifier_score(etat, -80);
    } else if (etat->tempsRestantMs <= 0) {
        etat->perdu = 1;
        modifier_score(etat, -80);
    }

    if (!etat->perdu && etat->nbBulles == 0) {
        etat->gagne = 1;
        modifier_score(etat, bonus_vitesse_niveau(etat));
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
    etat->tirActif = 0;
    etat->tirSecondaireActif = 0;
}
