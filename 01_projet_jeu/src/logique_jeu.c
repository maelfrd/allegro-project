#include "logique_jeu.h"

#include <stdlib.h>
#include <string.h>

static int collision_rect(float x1, float y1, int w1, int h1,
                          float x2, float y2, int w2, int h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
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
            faire_apparaitre_chapeau(etat, &source);
        }
        supprimer_bulle(etat, index);
        return;
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

static int collision_projectile_bulles(const EtatJeu *etat) {
    int i;

    for (i = 0; i < etat->nbBulles; i++) {
        if (collision_rect((float) etat->projectileX,
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

static int joueur_touche_chapeau(const EtatJeu *etat, const ConfigurationJeu *configuration) {
    if (!etat || !configuration || !etat->chapeauVisible) {
        return 0;
    }

    return collision_rect((float) etat->x,
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
    etat->niveauMaximum = 4;

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
    etat->modeFeuActif = 0;
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
    int idxTouchee;

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
        idxTouchee = collision_projectile_bulles(etat);
        if (idxTouchee != -1) {
            Bulle touchee = etat->bulles[idxTouchee];

            etat->projectileActive = 0;
            declencher_explosion(etat, &touchee);
            if (etat->modeFeuActif) {
                supprimer_bulle(etat, idxTouchee);
            } else {
                separer_bulle(etat, configuration, idxTouchee);
            }
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
        etat->modeFeuActif = 1;
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
