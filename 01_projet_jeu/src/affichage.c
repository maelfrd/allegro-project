#include "affichage.h"

#include <stdio.h>
#include <string.h>

enum {
    MENU_NOUVELLE_PARTIE,
    MENU_REPRENDRE_PARTIE,
    MENU_QUITTER,
    MENU_PARAMETRES,
    MENU_REGLES
};

static int obtenir_largeur_zone_dessin(const BITMAP *bitmap) {
    (void) bitmap;
    return screen ? SCREEN_W : VALEUR_NULLE;
}

static int obtenir_hauteur_zone_dessin(const BITMAP *bitmap) {
    (void) bitmap;
    return screen ? SCREEN_H : VALEUR_NULLE;
}

static int plus_petite_dimension(int largeur, int hauteur) {
    if (largeur <= VALEUR_NULLE) {
        return hauteur;
    }
    if (hauteur <= VALEUR_NULLE) {
        return largeur;
    }
    return largeur < hauteur ? largeur : hauteur;
}

static int dimension_relative_proportionnelle(int reference, int numerateur, int denominateur) {
    int valeur;

    if (reference <= VALEUR_NULLE || numerateur <= VALEUR_NULLE || denominateur <= VALEUR_NULLE) {
        return FAUX;
    }

    valeur = reference * numerateur / denominateur;
    if (valeur <= VALEUR_NULLE) {
        valeur = reference / reference;
    }

    return valeur;
}

static int dimension_relative_zone(const BITMAP *bitmap, int numerateur, int denominateur) {
    return dimension_relative_proportionnelle(plus_petite_dimension(obtenir_largeur_zone_dessin(bitmap),
                                                                    obtenir_hauteur_zone_dessin(bitmap)),
                                              numerateur,
                                              denominateur);
}

static void afficher_buffer_a_l_ecran(BITMAP *buffer) {
    if (!buffer || !screen) {
        return;
    }

    blit(buffer, screen, VALEUR_NULLE, VALEUR_NULLE, VALEUR_NULLE, VALEUR_NULLE, SCREEN_W, SCREEN_H);
}

static void initialiser_ressources_vides(RessourcesJeu *ressources) {
    int indexElement;

    ressources->fond = NULL;
    for (indexElement = INDEX_PREMIER; indexElement < NOMBRE_FONDS_NIVEAUX; indexElement++) {
        ressources->fondsNiveaux[indexElement] = NULL;
    }
    ressources->player = NULL;
    ressources->tir = NULL;
    ressources->feu = NULL;
    ressources->chapeau = NULL;
    ressources->explosion = NULL;
    ressources->buffer = NULL;
    for (indexElement = INDEX_PREMIER; indexElement < BULLE_TAILLES_TOTAL; indexElement++) {
        ressources->sprites[indexElement] = NULL;
        ressources->spritesVifDor[indexElement] = NULL;
        ressources->spritesVolDeMort[indexElement] = NULL;
    }
}

static void dessiner_boite_centrale(BITMAP *buffer,
                                    int demiLargeur,
                                    int demiHauteur,
                                    int couleurFond,
                                    int couleurContour) {
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);
    int centreX = largeurZoneDessin / DIVISEUR_CENTRE;
    int centreY = hauteurZoneDessin / DIVISEUR_CENTRE;

    rectfill(buffer,
             centreX - demiLargeur,
             centreY - demiHauteur,
             centreX + demiLargeur,
             centreY + demiHauteur,
             couleurFond);
    rect(buffer,
         centreX - demiLargeur,
         centreY - demiHauteur,
         centreX + demiLargeur,
         centreY + demiHauteur,
         couleurContour);
}

static void afficher_message_centre(BITMAP *buffer,
                                    int demiLargeur,
                                    int couleurBord,
                                    int couleurTexte,
                                    const char *message) {
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);

    dessiner_boite_centrale(buffer,
                            demiLargeur,
                            hauteurZoneDessin / DIVISEUR_QUATORZIEME,
                            makecol(COULEUR_NOIR_R, COULEUR_NOIR_G, COULEUR_NOIR_B),
                            couleurBord);

    textout_centre_ex(buffer,
                      font,
                      message,
                      largeurZoneDessin / DIVISEUR_CENTRE,
                      hauteurZoneDessin / DIVISEUR_CENTRE - text_height(font) / DIVISEUR_CENTRE,
                      couleurTexte,
                      COULEUR_TRANSPARENTE_TEXTE);
}

static BITMAP *obtenir_fond_niveau(const RessourcesJeu *ressources, int niveauActuel) {
    if (!ressources) {
        return NULL;
    }

    if (niveauActuel >= PREMIER_NIVEAU_JEU && niveauActuel <= NOMBRE_FONDS_NIVEAUX && ressources->fondsNiveaux[niveauActuel - INDEX_SUIVANT]) {
        return ressources->fondsNiveaux[niveauActuel - INDEX_SUIVANT];
    }

    return ressources->fond;
}

static void dessiner_fond_scene(BITMAP *buffer, const RessourcesJeu *ressources) {
    BITMAP *fond = obtenir_fond_niveau(ressources, PREMIER_NIVEAU_JEU);
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);

    clear_to_color(buffer, makecol(COULEUR_NOIR_R, COULEUR_NOIR_G, COULEUR_NOIR_B));

    if (fond) {
        stretch_blit(fond,
                     buffer,
                     VALEUR_NULLE,
                     VALEUR_NULLE,
                     fond->w,
                     fond->h,
                     VALEUR_NULLE,
                     VALEUR_NULLE,
                     largeurZoneDessin,
                     hauteurZoneDessin);
    }
}

static void dessiner_panneau_menu(BITMAP *buffer, int demiLargeur, int demiHauteur) {
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);
    int insetCadre = dimension_relative_zone(buffer,
                                             NUMERATEUR_REDIMENSION_RELATIVE,
                                             DIVISEUR_DEUX_CENT_CINQUANTE_SIXIEME);

    dessiner_boite_centrale(buffer,
                            demiLargeur,
                            demiHauteur,
                            makecol(COULEUR_PANNEAU_MENU_FOND_R, COULEUR_PANNEAU_MENU_FOND_G, COULEUR_PANNEAU_MENU_FOND_B),
                            makecol(COULEUR_PANNEAU_MENU_CONTOUR_R, COULEUR_PANNEAU_MENU_CONTOUR_G, COULEUR_PANNEAU_MENU_CONTOUR_B));
    rect(buffer,
         largeurZoneDessin / DIVISEUR_CENTRE - demiLargeur + insetCadre,
         hauteurZoneDessin / DIVISEUR_CENTRE - demiHauteur + insetCadre,
         largeurZoneDessin / DIVISEUR_CENTRE + demiLargeur - insetCadre,
         hauteurZoneDessin / DIVISEUR_CENTRE + demiHauteur - insetCadre,
         makecol(COULEUR_PANNEAU_MENU_INTERNE_R, COULEUR_PANNEAU_MENU_INTERNE_G, COULEUR_PANNEAU_MENU_INTERNE_B));
}

static void dessiner_texte_centre_aggrandi(BITMAP *buffer,
                                           const char *texte,
                                           int centreX,
                                           int centreY,
                                           int couleurTexte,
                                           int couleurFond,
                                           int hauteurCible) {
    BITMAP *texteBitmap;
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);
    int largeurTexte;
    int hauteurTexte;
    int largeurFinale;
    int hauteurFinale;

    if (!buffer || !texte || hauteurCible <= VALEUR_NULLE) {
        return;
    }

    largeurTexte = text_length(font, texte);
    hauteurTexte = text_height(font);
    if (largeurTexte <= VALEUR_NULLE || hauteurTexte <= VALEUR_NULLE) {
        return;
    }

    texteBitmap = create_bitmap(largeurTexte, hauteurTexte);
    if (!texteBitmap) {
        return;
    }

    clear_to_color(texteBitmap, makecol(COULEUR_MAGENTA_R, COULEUR_MAGENTA_G, COULEUR_MAGENTA_B));
    textout_centre_ex(texteBitmap, font, texte, largeurTexte / DIVISEUR_CENTRE, VALEUR_NULLE, couleurTexte, COULEUR_TRANSPARENTE_TEXTE);

    hauteurFinale = hauteurCible;
    largeurFinale = largeurTexte * hauteurFinale / hauteurTexte;

    rectfill(buffer,
             centreX - largeurFinale / DIVISEUR_CENTRE - largeurZoneDessin / DIVISEUR_QUARANTIEME,
             centreY - hauteurFinale / DIVISEUR_CENTRE - hauteurZoneDessin / DIVISEUR_TRENTE_SIXIEME,
             centreX + largeurFinale / DIVISEUR_CENTRE + largeurZoneDessin / DIVISEUR_QUARANTIEME,
             centreY + hauteurFinale / DIVISEUR_CENTRE + hauteurZoneDessin / DIVISEUR_TRENTE_SIXIEME,
             couleurFond);
    rect(buffer,
         centreX - largeurFinale / DIVISEUR_CENTRE - largeurZoneDessin / DIVISEUR_QUARANTIEME,
         centreY - hauteurFinale / DIVISEUR_CENTRE - hauteurZoneDessin / DIVISEUR_TRENTE_SIXIEME,
         centreX + largeurFinale / DIVISEUR_CENTRE + largeurZoneDessin / DIVISEUR_QUARANTIEME,
         centreY + hauteurFinale / DIVISEUR_CENTRE + hauteurZoneDessin / DIVISEUR_TRENTE_SIXIEME,
         makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B));

    stretch_sprite(buffer,
                   texteBitmap,
                   centreX - largeurFinale / DIVISEUR_CENTRE,
                   centreY - hauteurFinale / DIVISEUR_CENTRE,
                   largeurFinale,
                   hauteurFinale);

    destroy_bitmap(texteBitmap);
}

static void dessiner_scene_jeu(BITMAP *buffer, const RessourcesJeu *ressources, const EtatJeu *etatJeu) {
    int indexElement;
    BITMAP *fond;
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);

    if (!buffer || !ressources || !etatJeu || !ressources->player) {
        return;
    }

    fond = obtenir_fond_niveau(ressources, etatJeu->niveauActuel);
    clear_to_color(buffer, makecol(COULEUR_NOIR_R, COULEUR_NOIR_G, COULEUR_NOIR_B));
    if (fond) {
        stretch_blit(fond,
                     buffer,
                     VALEUR_NULLE,
                     VALEUR_NULLE,
                     fond->w,
                     fond->h,
                     VALEUR_NULLE,
                     VALEUR_NULLE,
                     largeurZoneDessin,
                     hauteurZoneDessin);
    }

    rectfill(buffer,
             VALEUR_NULLE,
             etatJeu->positionSolY,
             largeurZoneDessin,
             hauteurZoneDessin,
             makecol(COULEUR_SOL_R, COULEUR_SOL_G, COULEUR_SOL_B));

    for (indexElement = INDEX_PREMIER; indexElement < etatJeu->nombreBulles; indexElement++) {
        BITMAP *sprite;

        if (etatJeu->bullesEnJeu[indexElement].typeEntite == ENTITE_VIF_DOR) {
            sprite = ressources->spritesVifDor[(int) etatJeu->bullesEnJeu[indexElement].tailleBulle];
        } else if (etatJeu->bullesEnJeu[indexElement].typeEntite == ENTITE_VOL_DE_MORT) {
            sprite = ressources->spritesVolDeMort[(int) etatJeu->bullesEnJeu[indexElement].tailleBulle];
        } else {
            sprite = ressources->sprites[(int) etatJeu->bullesEnJeu[indexElement].tailleBulle];
        }

        if (!sprite) {
            continue;
        }

        masked_blit(sprite,
                    buffer,
                    VALEUR_NULLE,
                    VALEUR_NULLE,
                    (int) etatJeu->bullesEnJeu[indexElement].positionBulleX,
                    (int) etatJeu->bullesEnJeu[indexElement].positionBulleY,
                    sprite->w,
                    sprite->h);
    }

    masked_blit(ressources->player,
                buffer,
                VALEUR_NULLE,
                VALEUR_NULLE,
                etatJeu->positionJoueurX,
                etatJeu->positionJoueurY,
                ressources->player->w,
                ressources->player->h);

    if (etatJeu->projectileEstActif && ressources->tir) {
        BITMAP *projectileSprite = ressources->tir;

        masked_blit(projectileSprite,
                    buffer,
                    VALEUR_NULLE,
                    VALEUR_NULLE,
                    etatJeu->positionProjectileX - projectileSprite->w / DIVISEUR_CENTRE + etatJeu->largeurProjectile / DIVISEUR_CENTRE,
                    etatJeu->positionProjectileY - projectileSprite->h + etatJeu->hauteurProjectile,
                    projectileSprite->w,
                    projectileSprite->h);
    }

    if (etatJeu->auraArdenteEstActive && ressources->feu) {
        int effetX = etatJeu->positionJoueurX - ressources->player->w / DIVISEUR_QUART;
        int effetY = etatJeu->positionJoueurY - ressources->player->h / DIVISEUR_TIERS;
        int effetLargeur = ressources->player->w + ressources->player->w / DIVISEUR_CENTRE;
        int effetHauteur = ressources->player->h + ressources->player->h / DIVISEUR_CENTRE;

        stretch_sprite(buffer,
                       ressources->feu,
                       effetX,
                       effetY,
                       effetLargeur,
                       effetHauteur);
    }

    if (etatJeu->chapeauEstVisible && ressources->chapeau) {
        masked_blit(ressources->chapeau,
                    buffer,
                    VALEUR_NULLE,
                    VALEUR_NULLE,
                    etatJeu->positionChapeauX,
                    etatJeu->positionChapeauY,
                    ressources->chapeau->w,
                    ressources->chapeau->h);
    }

    if (etatJeu->explosionEstActive && ressources->explosion) {
        masked_blit(ressources->explosion,
                    buffer,
                    VALEUR_NULLE,
                    VALEUR_NULLE,
                    etatJeu->positionExplosionX,
                    etatJeu->positionExplosionY,
                    ressources->explosion->w,
                    ressources->explosion->h);
    }
}

static void dessiner_option_menu(BITMAP *buffer,
                                 const char *texte,
                                 int positionOptionY,
                                 int selectionnee,
                                 int active) {
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);
    int demiLargeur = largeurZoneDessin / DIVISEUR_SIXIEME;
    int demiHauteur = hauteurZoneDessin / DIVISEUR_QUARANTE_CINQUIEME;
    int positionGauche = largeurZoneDessin / DIVISEUR_CENTRE - demiLargeur;
    int positionDroite = largeurZoneDessin / DIVISEUR_CENTRE + demiLargeur;
    int couleurFond;
    int couleurTexte;
    int couleurCadre;

    if (!active) {
        couleurFond = makecol(COULEUR_OPTION_INACTIVE_FOND_R, COULEUR_OPTION_INACTIVE_FOND_G, COULEUR_OPTION_INACTIVE_FOND_B);
        couleurTexte = makecol(COULEUR_OPTION_INACTIVE_TEXTE_R, COULEUR_OPTION_INACTIVE_TEXTE_G, COULEUR_OPTION_INACTIVE_TEXTE_B);
        couleurCadre = makecol(COULEUR_OPTION_INACTIVE_CADRE_R, COULEUR_OPTION_INACTIVE_CADRE_G, COULEUR_OPTION_INACTIVE_CADRE_B);
    } else if (selectionnee) {
        couleurFond = makecol(COULEUR_OPTION_SELECTION_FOND_R, COULEUR_OPTION_SELECTION_FOND_G, COULEUR_OPTION_SELECTION_FOND_B);
        couleurTexte = makecol(COULEUR_OPTION_SELECTION_TEXTE_R, COULEUR_OPTION_SELECTION_TEXTE_G, COULEUR_OPTION_SELECTION_TEXTE_B);
        couleurCadre = makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B);
    } else {
        couleurFond = makecol(COULEUR_OPTION_ACTIVE_FOND_R, COULEUR_OPTION_ACTIVE_FOND_G, COULEUR_OPTION_ACTIVE_FOND_B);
        couleurTexte = makecol(COULEUR_OPTION_ACTIVE_TEXTE_R, COULEUR_OPTION_ACTIVE_TEXTE_G, COULEUR_OPTION_ACTIVE_TEXTE_B);
        couleurCadre = makecol(COULEUR_OPTION_ACTIVE_CADRE_R, COULEUR_OPTION_ACTIVE_CADRE_G, COULEUR_OPTION_ACTIVE_CADRE_B);
    }

    rectfill(buffer, positionGauche, positionOptionY - demiHauteur, positionDroite, positionOptionY + demiHauteur, couleurFond);
    rect(buffer, positionGauche, positionOptionY - demiHauteur, positionDroite, positionOptionY + demiHauteur, couleurCadre);
    textout_centre_ex(buffer,
                      font,
                      texte,
                      largeurZoneDessin / DIVISEUR_CENTRE,
                      positionOptionY - text_height(font) / DIVISEUR_CENTRE,
                      couleurTexte,
                      COULEUR_TRANSPARENTE_TEXTE);
}

static void tronquer_texte_pour_largeur(const char *texteSource,
                                        char *texteDestination,
                                        size_t tailleTexteDestination,
                                        int largeurMax) {
    size_t longueur;

    if (!texteDestination || tailleTexteDestination == VALEUR_NULLE) {
        return;
    }

    texteDestination[CHAINE_DEBUT] = CARACTERE_FIN_CHAINE;
    if (!texteSource) {
        return;
    }

    strncpy(texteDestination, texteSource, tailleTexteDestination - INDEX_SUIVANT);
    texteDestination[tailleTexteDestination - INDEX_SUIVANT] = CARACTERE_FIN_CHAINE;

    if (text_length(font, texteDestination) <= largeurMax) {
        return;
    }

    longueur = strlen(texteDestination);
    while (longueur > VALEUR_TRIPLE) {
        longueur--;
        texteDestination[longueur] = CARACTERE_FIN_CHAINE;
        if (snprintf(texteDestination + longueur, tailleTexteDestination - longueur, "...") >=
            (int) (tailleTexteDestination - longueur)) {
            break;
        }
        if (text_length(font, texteDestination) <= largeurMax) {
            return;
        }
        texteDestination[longueur] = CARACTERE_FIN_CHAINE;
    }

    strncpy(texteDestination, "...", tailleTexteDestination - INDEX_SUIVANT);
    texteDestination[tailleTexteDestination - INDEX_SUIVANT] = CARACTERE_FIN_CHAINE;
}

static void dessiner_panneau_hud(BITMAP *buffer, const EtatJeu *etatJeu) {
    char ligne[TAILLE_LIGNE_HUD];
    char pseudoJoueur[TAILLE_PSEUDO_MAX + TAILLE_PSEUDO_AFFICHE];
    int largeurZoneDessin;
    int hauteurZoneDessin;
    int margeX;
    int margeY;
    int largeur;
    int hauteur;
    int positionGauche;
    int positionHaut;
    int positionDroite;
    int positionBas;
    int positionPixelY;
    int interligne;
    int largeurTexte;
    int insetCadre;
    float tempsSecondes;

    if (!buffer || !etatJeu) {
        return;
    }

    largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);
    margeX = largeurZoneDessin / DIVISEUR_QUARANTIEME;
    margeY = hauteurZoneDessin / DIVISEUR_TRENTE_SIXIEME;
    largeur = largeurZoneDessin / DIVISEUR_QUART;
    hauteur = hauteurZoneDessin / DIVISEUR_SIXIEME;
    positionGauche = margeX;
    positionHaut = margeY;
    positionDroite = positionGauche + largeur;
    positionBas = positionHaut + hauteur;
    interligne = text_height(font) + hauteurZoneDessin / DIVISEUR_QUATRE_VINGT_DIXIEME;
    largeurTexte = largeur - largeurZoneDessin / DIVISEUR_QUARANTE_CINQUIEME;
    insetCadre = dimension_relative_zone(buffer,
                                         NUMERATEUR_REDIMENSION_RELATIVE,
                                         DIVISEUR_TROIS_CENT_QUARANTE_ET_UNIEME);

    if (etatJeu->niveauActuel != VALEUR_UNITAIRE) {
        rectfill(buffer, positionGauche, positionHaut, positionDroite, positionBas, makecol(COULEUR_HUD_FOND_R, COULEUR_HUD_FOND_G, COULEUR_HUD_FOND_B));
        rect(buffer, positionGauche, positionHaut, positionDroite, positionBas, makecol(COULEUR_HUD_CONTOUR_R, COULEUR_HUD_CONTOUR_G, COULEUR_HUD_CONTOUR_B));
        rect(buffer,
             positionGauche + insetCadre,
             positionHaut + insetCadre,
             positionDroite - insetCadre,
             positionBas - insetCadre,
             makecol(COULEUR_HUD_CONTOUR_INTERNE_R, COULEUR_HUD_CONTOUR_INTERNE_G, COULEUR_HUD_CONTOUR_INTERNE_B));
    }

    positionPixelY = positionHaut + hauteurZoneDessin / DIVISEUR_SOIXANTE_DIXIEME;
    textout_ex(buffer, font, "HUD JOUEUR", positionGauche + largeurZoneDessin / DIVISEUR_QUATRE_VINGT_DIXIEME, positionPixelY, makecol(COULEUR_HUD_TITRE_R, COULEUR_HUD_TITRE_G, COULEUR_HUD_TITRE_B), COULEUR_TRANSPARENTE_TEXTE);

    positionPixelY += interligne;
    snprintf(ligne, sizeof(ligne), FORMAT_CHRONOMETRE_HUD, (etatJeu->tempsRestantNiveauMs + ARRONDI_CHRONO_MS) / VALEUR_MILLISECONDES_SECONDE);
    textout_ex(buffer, font, ligne, positionGauche + largeurZoneDessin / DIVISEUR_QUATRE_VINGT_DIXIEME, positionPixelY, makecol(COULEUR_HUD_TITRE_R, COULEUR_HUD_TITRE_G, COULEUR_HUD_TITRE_B), COULEUR_TRANSPARENTE_TEXTE);

    positionPixelY += interligne;
    snprintf(ligne, sizeof(ligne), "Niveau : %d/%d", etatJeu->niveauActuel, etatJeu->niveauMaximum);
    textout_ex(buffer, font, ligne, positionGauche + largeurZoneDessin / DIVISEUR_QUATRE_VINGT_DIXIEME, positionPixelY, makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B), COULEUR_TRANSPARENTE_TEXTE);

    positionPixelY += interligne;
    snprintf(ligne, sizeof(ligne), "Score : %d", etatJeu->scoreJoueur);
    textout_ex(buffer, font, ligne, positionGauche + largeurZoneDessin / DIVISEUR_QUATRE_VINGT_DIXIEME, positionPixelY, makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B), COULEUR_TRANSPARENTE_TEXTE);

    positionPixelY += interligne;
    snprintf(pseudoJoueur, sizeof(pseudoJoueur), "Pseudo : %s", etatJeu->pseudoJoueur[CHAINE_DEBUT] != CARACTERE_FIN_CHAINE ? etatJeu->pseudoJoueur : "ANONYME");
    tronquer_texte_pour_largeur(pseudoJoueur, ligne, sizeof(ligne), largeurTexte);
    textout_ex(buffer, font, ligne, positionGauche + largeurZoneDessin / DIVISEUR_QUATRE_VINGT_DIXIEME, positionPixelY, makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B), COULEUR_TRANSPARENTE_TEXTE);

    positionPixelY += interligne;
    if (etatJeu->auraArdenteEstActive) {
        tempsSecondes = (float) etatJeu->dureeRestanteAuraArdenteMs / (float) VALEUR_MILLISECONDES_SECONDE;
        snprintf(ligne, sizeof(ligne), FORMAT_AURA_ARDENTE_HUD, tempsSecondes);
        textout_ex(buffer, font, ligne, positionGauche + largeurZoneDessin / DIVISEUR_QUATRE_VINGT_DIXIEME, positionPixelY, makecol(COULEUR_HUD_AURA_ACTIVE_R, COULEUR_HUD_AURA_ACTIVE_G, COULEUR_HUD_AURA_ACTIVE_B), COULEUR_TRANSPARENTE_TEXTE);
    } else {
        textout_ex(buffer, font, "Aura ardente : INACTIVE", positionGauche + largeurZoneDessin / DIVISEUR_QUATRE_VINGT_DIXIEME, positionPixelY, makecol(COULEUR_HUD_AURA_INACTIVE_R, COULEUR_HUD_AURA_INACTIVE_G, COULEUR_HUD_AURA_INACTIVE_B), COULEUR_TRANSPARENTE_TEXTE);
    }
}

static int obtenir_vie_boss(const EtatJeu *etatJeu) {
    int indexElement;
    int vieBoss = VALEUR_NULLE;

    if (!etatJeu) {
        return FAUX;
    }

    for (indexElement = INDEX_PREMIER; indexElement < etatJeu->nombreBulles; indexElement++) {
        if (etatJeu->bullesEnJeu[indexElement].typeEntite == ENTITE_VOL_DE_MORT &&
            etatJeu->bullesEnJeu[indexElement].nombreCoupsAvantDivision > vieBoss) {
            vieBoss = etatJeu->bullesEnJeu[indexElement].nombreCoupsAvantDivision;
        }
    }

    return vieBoss;
}

static int boss_present(const EtatJeu *etatJeu) {
    int indexElement;

    if (!etatJeu) {
        return FAUX;
    }

    for (indexElement = INDEX_PREMIER; indexElement < etatJeu->nombreBulles; indexElement++) {
        if (etatJeu->bullesEnJeu[indexElement].typeEntite == ENTITE_VOL_DE_MORT &&
            etatJeu->bullesEnJeu[indexElement].nombreCoupsAvantDivision > VALEUR_NULLE) {
            return VRAI;
        }
    }

    return FAUX;
}

static void dessiner_barre_vie_boss(BITMAP *buffer, const EtatJeu *etatJeu) {
    int largeurZoneDessin;
    int hauteurZoneDessin;
    int largeurBarre;
    int hauteurBarre;
    int positionGauche;
    int positionHaut;
    int positionDroite;
    int positionBas;
    int vieBoss;
    int largeurVie;

    if (!buffer || !boss_present(etatJeu)) {
        return;
    }

    largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);
    largeurBarre = largeurZoneDessin / DIVISEUR_TIERS;
    hauteurBarre = hauteurZoneDessin / DIVISEUR_TRENTE_CINQUIEME;
    positionGauche = largeurZoneDessin / DIVISEUR_CENTRE - largeurBarre / DIVISEUR_CENTRE;
    positionHaut = hauteurZoneDessin / DIVISEUR_VINGT_QUATRIEME;
    positionDroite = positionGauche + largeurBarre;
    positionBas = positionHaut + hauteurBarre;
    vieBoss = obtenir_vie_boss(etatJeu);
    if (vieBoss < VALEUR_NULLE) {
        vieBoss = VALEUR_NULLE;
    }
    if (vieBoss > VIE_MAX_BOSS_VOL_DE_MORT) {
        vieBoss = VIE_MAX_BOSS_VOL_DE_MORT;
    }

    largeurVie = largeurBarre * vieBoss / VIE_MAX_BOSS_VOL_DE_MORT;

    rectfill(buffer, positionGauche, positionHaut, positionDroite, positionBas, makecol(COULEUR_BOSS_FOND_R, COULEUR_BOSS_FOND_G, COULEUR_BOSS_FOND_B));
    rectfill(buffer, positionGauche, positionHaut, positionGauche + largeurVie, positionBas, makecol(COULEUR_BOSS_VIE_R, COULEUR_BOSS_VIE_G, COULEUR_BOSS_VIE_B));
    rect(buffer, positionGauche, positionHaut, positionDroite, positionBas, makecol(COULEUR_BOSS_CONTOUR_R, COULEUR_BOSS_CONTOUR_G, COULEUR_BOSS_CONTOUR_B));
    textout_centre_ex(buffer,
                      font,
                      "BOSS",
                      largeurZoneDessin / DIVISEUR_CENTRE,
                      positionHaut + hauteurBarre / DIVISEUR_CENTRE - text_height(font) / DIVISEUR_CENTRE,
                      makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B),
                      COULEUR_TRANSPARENTE_TEXTE);
}

static void normaliser_transparence_magenta(BITMAP *bitmap) {
    int positionPixelX;
    int positionPixelY;
    int couleurMasque;

    if (!bitmap) {
        return;
    }

    couleurMasque = bitmap_mask_color(bitmap);
    for (positionPixelY = INDEX_PREMIER; positionPixelY < bitmap->h; positionPixelY++) {
        for (positionPixelX = INDEX_PREMIER; positionPixelX < bitmap->w; positionPixelX++) {
            int couleur = getpixel(bitmap, positionPixelX, positionPixelY);
            int r = getr(couleur);
            int g = getg(couleur);
            int b = getb(couleur);

            if (r >= SEUIL_TRANSPARENCE_MAGENTA_R &&
                g <= SEUIL_TRANSPARENCE_MAGENTA_G &&
                b >= SEUIL_TRANSPARENCE_MAGENTA_B) {
                putpixel(bitmap, positionPixelX, positionPixelY, couleurMasque);
            }
        }
    }
}

static int fichier_existe_simple(const char *chemin) {
    FILE *fichier;

    if (!chemin || chemin[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE) {
        return FAUX;
    }

    fichier = fopen(chemin, "rb");
    if (!fichier) {
        return FAUX;
    }

    fclose(fichier);
    return VRAI;
}

static int resoudre_chemin_ressource(const char *chemin, char *cheminResolu, size_t tailleCheminResolu) {
    static const char *prefixes[] = {
        "",
        "../",
        "../../",
        PREFIXE_RESSOURCE_PROJET,
        PREFIXE_RESSOURCE_PARENT_PROJET
    };
    int indexElement;

    if (!chemin || !cheminResolu || tailleCheminResolu == VALEUR_NULLE || chemin[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE) {
        return FAUX;
    }

    if (chemin[CHAINE_DEBUT] == '/') {
        if (strlen(chemin) + INDEX_SUIVANT > tailleCheminResolu) {
            return FAUX;
        }
        strcpy(cheminResolu, chemin);
        return fichier_existe_simple(cheminResolu);
    }

    for (indexElement = INDEX_PREMIER; indexElement < (int) (sizeof(prefixes) / sizeof(prefixes[CHAINE_DEBUT])); indexElement++) {
        if (snprintf(cheminResolu, tailleCheminResolu, "%s%s", prefixes[indexElement], chemin) >=
            (int) tailleCheminResolu) {
            continue;
        }
        if (fichier_existe_simple(cheminResolu)) {
            return VRAI;
        }
    }

    cheminResolu[CHAINE_DEBUT] = CARACTERE_FIN_CHAINE;
    return FAUX;
}

static int obtenir_dimensions_bitmap(const char *chemin, int *largeur, int *hauteur) {
    BITMAP *bitmap;
    char cheminResolu[TAILLE_CHEMIN_RESSOURCE];

    if (!largeur || !hauteur) {
        return FAUX;
    }

    *largeur = VALEUR_NULLE;
    *hauteur = VALEUR_NULLE;
    if (!resoudre_chemin_ressource(chemin, cheminResolu, sizeof(cheminResolu))) {
        return FAUX;
    }

    bitmap = load_bitmap(cheminResolu, NULL);
    if (!bitmap) {
        return FAUX;
    }

    *largeur = bitmap->w;
    *hauteur = bitmap->h;
    destroy_bitmap(bitmap);
    return *largeur > VALEUR_NULLE && *hauteur > VALEUR_NULLE;
}

int initialiser_affichage(const char *fond_path,
                          int profondeur_couleur) {
    int largeur;
    int hauteur;
    int largeurReference;
    int hauteurReference;

    allegro_init();

    if (install_keyboard() != VALEUR_NULLE) {
        allegro_message("Erreur d'initialisation du clavier");
        return FAUX;
    }

    set_color_depth(profondeur_couleur);
    largeurReference = VALEUR_NULLE;
    hauteurReference = VALEUR_NULLE;

    if (get_desktop_resolution(&largeurReference, &hauteurReference) != VALEUR_NULLE ||
        largeurReference <= VALEUR_NULLE ||
        hauteurReference <= VALEUR_NULLE) {
        if (!obtenir_dimensions_bitmap(fond_path, &largeurReference, &hauteurReference)) {
            allegro_message("Impossible de calculer une taille de fenetre relative");
            return FAUX;
        }
    }

    largeur = dimension_relative_proportionnelle(largeurReference,
                                                 JEU_FENETRE_LARGEUR_NUMERATEUR,
                                                 JEU_FENETRE_RATIO_DENOMINATEUR);
    hauteur = dimension_relative_proportionnelle(hauteurReference,
                                                 JEU_FENETRE_HAUTEUR_NUMERATEUR,
                                                 JEU_FENETRE_RATIO_DENOMINATEUR);

    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, largeur, hauteur, VALEUR_NULLE, VALEUR_NULLE) != VALEUR_NULLE) {
        allegro_message("Erreur mode graphique");
        return FAUX;
    }

    return VRAI;
}

int ressource_existe(const char *chemin) {
    char cheminResolu[TAILLE_CHEMIN_RESSOURCE];

    return resoudre_chemin_ressource(chemin, cheminResolu, sizeof(cheminResolu));
}

BITMAP *charger_bitmap_ou_erreur(const char *chemin) {
    BITMAP *bitmap;
    char cheminResolu[TAILLE_CHEMIN_RESSOURCE];

    if (!chemin || chemin[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE) {
        allegro_message("Chemin de bitmap invalide");
        return NULL;
    }

    if (!resoudre_chemin_ressource(chemin, cheminResolu, sizeof(cheminResolu))) {
        allegro_message("Impossible de trouver %s", chemin);
        return NULL;
    }

    bitmap = load_bitmap(cheminResolu, NULL);
    if (!bitmap) {
        allegro_message("Impossible de charger %s", cheminResolu);
    }

    return bitmap;
}

static BITMAP *resize_bitmap_dimensions(BITMAP *bitmapSource, int largeur, int hauteur) {
    BITMAP *bitmapRedimensionne;

    if (!bitmapSource || largeur <= VALEUR_NULLE || hauteur <= VALEUR_NULLE) {
        return NULL;
    }

    bitmapRedimensionne = create_bitmap(largeur, hauteur);
    if (!bitmapRedimensionne) {
        return NULL;
    }

    clear_to_color(bitmapRedimensionne, makecol(COULEUR_MAGENTA_R, COULEUR_MAGENTA_G, COULEUR_MAGENTA_B));
    stretch_blit(bitmapSource, bitmapRedimensionne, VALEUR_NULLE, VALEUR_NULLE, bitmapSource->w, bitmapSource->h, VALEUR_NULLE, VALEUR_NULLE, largeur, hauteur);
    return bitmapRedimensionne;
}

static BITMAP *resize_bitmap_hauteur_relative(BITMAP *bitmapSource, int hauteurReference, int numerateur, int denominateur) {
    int hauteur;
    int largeur;

    if (!bitmapSource || bitmapSource->w <= VALEUR_NULLE || bitmapSource->h <= VALEUR_NULLE) {
        return NULL;
    }

    hauteur = dimension_relative_proportionnelle(hauteurReference, numerateur, denominateur);
    largeur = bitmapSource->w * hauteur / bitmapSource->h;
    if (largeur <= VALEUR_NULLE) {
        largeur = hauteur > VALEUR_NULLE ? hauteur : VALEUR_NULLE;
    }

    return resize_bitmap_dimensions(bitmapSource, largeur, hauteur);
}

int charger_ressources_jeu(RessourcesJeu *ressources,
                           const char *fond_path,
                           const char *player_path,
                           const char *mangemort_path,
                           const char *vifdor_path,
                           const char *tir_path,
                           const char *feu_path,
                           const char *chapeau_path,
                           const char *explosion_path) {
    BITMAP *player_orig;
    BITMAP *bulle_orig;
    BITMAP *vifdor_orig;
    BITMAP *tir_orig;
    BITMAP *feu_orig;
    BITMAP *chapeau_orig;
    BITMAP *explosion_orig;
    BITMAP *volDeMortOrig;
    int largeurZoneDessin;
    int hauteurZoneDessin;
    int referenceSprites;

    if (!ressources) {
        return FAUX;
    }

    initialiser_ressources_vides(ressources);
    largeurZoneDessin = screen ? SCREEN_W : VALEUR_NULLE;
    hauteurZoneDessin = screen ? SCREEN_H : VALEUR_NULLE;
    referenceSprites = plus_petite_dimension(largeurZoneDessin, hauteurZoneDessin);

    ressources->fond = charger_bitmap_ou_erreur(fond_path);
    if (!ressources->fond) {
        liberer_ressources_jeu(ressources);
        return FAUX;
    }
    ressources->fondsNiveaux[INDEX_FOND_NIVEAU_1] = ressources->fond;
    ressources->fondsNiveaux[INDEX_FOND_NIVEAU_2] = charger_bitmap_ou_erreur(CHEMIN_FOND_NIVEAU_2);
    ressources->fondsNiveaux[INDEX_FOND_NIVEAU_3] = charger_bitmap_ou_erreur(CHEMIN_FOND_NIVEAU_3);
    ressources->fondsNiveaux[INDEX_FOND_NIVEAU_4] = charger_bitmap_ou_erreur(CHEMIN_FOND_NIVEAU_4);
    ressources->fondsNiveaux[INDEX_FOND_NIVEAU_5] = charger_bitmap_ou_erreur(CHEMIN_FOND_NIVEAU_5);
    if (!ressources->fondsNiveaux[INDEX_FOND_NIVEAU_2] ||
        !ressources->fondsNiveaux[INDEX_FOND_NIVEAU_3] ||
        !ressources->fondsNiveaux[INDEX_FOND_NIVEAU_4] ||
        !ressources->fondsNiveaux[INDEX_FOND_NIVEAU_5]) {
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    player_orig = charger_bitmap_ou_erreur(player_path);
    if (!player_orig) {
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    ressources->player = resize_bitmap_hauteur_relative(player_orig,
                                                        hauteurZoneDessin,
                                                        NUMERATEUR_REDIMENSION_RELATIVE,
                                                        DIVISEUR_REDIMENSION_JOUEUR);
    destroy_bitmap(player_orig);
    if (!ressources->player) {
        allegro_message("Erreur resize personnage");
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    tir_orig = charger_bitmap_ou_erreur(tir_path);
    if (!tir_orig) {
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    ressources->tir = resize_bitmap_hauteur_relative(tir_orig,
                                                     hauteurZoneDessin,
                                                     NUMERATEUR_REDIMENSION_RELATIVE,
                                                     DIVISEUR_REDIMENSION_TIR);
    destroy_bitmap(tir_orig);
    if (!ressources->tir) {
        allegro_message("Erreur resize tir");
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    feu_orig = charger_bitmap_ou_erreur(feu_path);
    if (!feu_orig) {
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    ressources->feu = resize_bitmap_hauteur_relative(feu_orig,
                                                     hauteurZoneDessin,
                                                     NUMERATEUR_REDIMENSION_RELATIVE,
                                                     DIVISEUR_REDIMENSION_FEU);
    destroy_bitmap(feu_orig);
    if (!ressources->feu) {
        allegro_message("Erreur resize feu");
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    chapeau_orig = charger_bitmap_ou_erreur(chapeau_path);
    if (!chapeau_orig) {
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    ressources->chapeau = resize_bitmap_hauteur_relative(chapeau_orig,
                                                         hauteurZoneDessin,
                                                         NUMERATEUR_REDIMENSION_RELATIVE,
                                                         DIVISEUR_REDIMENSION_CHAPEAU);
    destroy_bitmap(chapeau_orig);
    if (!ressources->chapeau) {
        allegro_message("Erreur resize chapeau");
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    explosion_orig = charger_bitmap_ou_erreur(explosion_path);
    if (!explosion_orig) {
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    ressources->explosion = resize_bitmap_hauteur_relative(explosion_orig,
                                                           hauteurZoneDessin,
                                                           NUMERATEUR_REDIMENSION_RELATIVE,
                                                           DIVISEUR_REDIMENSION_EXPLOSION);
    destroy_bitmap(explosion_orig);
    if (!ressources->explosion) {
        allegro_message("Erreur resize explosion");
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    bulle_orig = charger_bitmap_ou_erreur(mangemort_path);
    if (!bulle_orig) {
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    ressources->sprites[BULLE_TRES_GRANDE] = resize_bitmap_hauteur_relative(bulle_orig,
                                                                            referenceSprites,
                                                                            NUMERATEUR_REDIMENSION_RELATIVE,
                                                                            DIVISEUR_REDIMENSION_BULLE_TRES_GRANDE);
    ressources->sprites[BULLE_GRANDE] = resize_bitmap_hauteur_relative(bulle_orig,
                                                                       referenceSprites,
                                                                       NUMERATEUR_REDIMENSION_RELATIVE,
                                                                       DIVISEUR_REDIMENSION_BULLE_GRANDE);
    ressources->sprites[BULLE_MOYENNE] = resize_bitmap_hauteur_relative(bulle_orig,
                                                                        referenceSprites,
                                                                        NUMERATEUR_REDIMENSION_RELATIVE,
                                                                        DIVISEUR_REDIMENSION_BULLE_MOYENNE);
    ressources->sprites[BULLE_PETITE] = resize_bitmap_hauteur_relative(bulle_orig,
                                                                       referenceSprites,
                                                                       NUMERATEUR_REDIMENSION_RELATIVE,
                                                                       DIVISEUR_REDIMENSION_BULLE_PETITE);
    destroy_bitmap(bulle_orig);

    normaliser_transparence_magenta(ressources->sprites[BULLE_TRES_GRANDE]);
    normaliser_transparence_magenta(ressources->sprites[BULLE_GRANDE]);
    normaliser_transparence_magenta(ressources->sprites[BULLE_MOYENNE]);
    normaliser_transparence_magenta(ressources->sprites[BULLE_PETITE]);

    if (!ressources->sprites[BULLE_TRES_GRANDE] ||
        !ressources->sprites[BULLE_GRANDE] ||
        !ressources->sprites[BULLE_MOYENNE] ||
        !ressources->sprites[BULLE_PETITE]) {
        allegro_message("Erreur resize des bulles");
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    vifdor_orig = charger_bitmap_ou_erreur(vifdor_path);
    if (!vifdor_orig) {
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    ressources->spritesVifDor[BULLE_TRES_GRANDE] = resize_bitmap_dimensions(vifdor_orig,
                                                                            ressources->sprites[BULLE_TRES_GRANDE]->w,
                                                                            ressources->sprites[BULLE_TRES_GRANDE]->h);
    ressources->spritesVifDor[BULLE_GRANDE] = resize_bitmap_dimensions(vifdor_orig,
                                                                       ressources->sprites[BULLE_GRANDE]->w,
                                                                       ressources->sprites[BULLE_GRANDE]->h);
    ressources->spritesVifDor[BULLE_MOYENNE] = resize_bitmap_dimensions(vifdor_orig,
                                                                        ressources->sprites[BULLE_MOYENNE]->w,
                                                                        ressources->sprites[BULLE_MOYENNE]->h);
    ressources->spritesVifDor[BULLE_PETITE] = resize_bitmap_dimensions(vifdor_orig,
                                                                       ressources->sprites[BULLE_PETITE]->w,
                                                                       ressources->sprites[BULLE_PETITE]->h);
    destroy_bitmap(vifdor_orig);

    if (!ressources->spritesVifDor[BULLE_TRES_GRANDE] ||
        !ressources->spritesVifDor[BULLE_GRANDE] ||
        !ressources->spritesVifDor[BULLE_MOYENNE] ||
        !ressources->spritesVifDor[BULLE_PETITE]) {
        allegro_message("Erreur resize vif d'or");
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    volDeMortOrig = charger_bitmap_ou_erreur(CHEMIN_SPRITE_VOL_DE_MORT);
    if (!volDeMortOrig) {
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    ressources->spritesVolDeMort[BULLE_TRES_GRANDE] = resize_bitmap_dimensions(volDeMortOrig,
                                                                                ressources->sprites[BULLE_TRES_GRANDE]->w,
                                                                                ressources->sprites[BULLE_TRES_GRANDE]->h);
    ressources->spritesVolDeMort[BULLE_GRANDE] = resize_bitmap_dimensions(volDeMortOrig,
                                                                           ressources->sprites[BULLE_GRANDE]->w,
                                                                           ressources->sprites[BULLE_GRANDE]->h);
    ressources->spritesVolDeMort[BULLE_MOYENNE] = resize_bitmap_dimensions(volDeMortOrig,
                                                                            ressources->sprites[BULLE_MOYENNE]->w,
                                                                            ressources->sprites[BULLE_MOYENNE]->h);
    ressources->spritesVolDeMort[BULLE_PETITE] = resize_bitmap_dimensions(volDeMortOrig,
                                                                           ressources->sprites[BULLE_PETITE]->w,
                                                                           ressources->sprites[BULLE_PETITE]->h);
    destroy_bitmap(volDeMortOrig);

    normaliser_transparence_magenta(ressources->spritesVolDeMort[BULLE_TRES_GRANDE]);
    normaliser_transparence_magenta(ressources->spritesVolDeMort[BULLE_GRANDE]);
    normaliser_transparence_magenta(ressources->spritesVolDeMort[BULLE_MOYENNE]);
    normaliser_transparence_magenta(ressources->spritesVolDeMort[BULLE_PETITE]);

    if (!ressources->spritesVolDeMort[BULLE_TRES_GRANDE] ||
        !ressources->spritesVolDeMort[BULLE_GRANDE] ||
        !ressources->spritesVolDeMort[BULLE_MOYENNE] ||
        !ressources->spritesVolDeMort[BULLE_PETITE]) {
        allegro_message("Erreur resize vol de mort");
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    ressources->buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!ressources->buffer) {
        allegro_message("Impossible de creer le buffer");
        liberer_ressources_jeu(ressources);
        return FAUX;
    }

    return VRAI;
}

void dessiner_menu_depart(const RessourcesJeu *ressources, int selection, int repriseDisponible) {
    static const char *options[] = {
        "Nouvelle partie",
        "Reprendre partie",
        "Quitter",
        "Parametres",
        "Regles"
    };
    int indexElement;
    int yBase;
    int pasVertical;

    if (!ressources || !ressources->buffer) {
        return;
    }

    dessiner_fond_scene(ressources->buffer, ressources);
    dessiner_panneau_menu(ressources->buffer, SCREEN_W / DIVISEUR_QUART, SCREEN_H / DIVISEUR_QUART);

    textout_centre_ex(ressources->buffer,
                      font,
                      "MENU DE DEPART",
                      SCREEN_W / DIVISEUR_CENTRE,
                      SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_CINQUIEME,
                      makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B),
                      COULEUR_TRANSPARENTE_TEXTE);
    textout_centre_ex(ressources->buffer,
                      font,
                      "Utilisez Haut/Bas puis Entree",
                      SCREEN_W / DIVISEUR_CENTRE,
                      SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_SIXIEME,
                      makecol(COULEUR_TEXTE_AIDE_R, COULEUR_TEXTE_AIDE_G, COULEUR_TEXTE_AIDE_B),
                      COULEUR_TRANSPARENTE_TEXTE);

    yBase = SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_TREIZIEME;
    pasVertical = SCREEN_H / DIVISEUR_DIX_HUITIEME;
    for (indexElement = INDEX_PREMIER; indexElement < NOMBRE_OPTIONS_MENU_DEPART; indexElement++) {
        dessiner_option_menu(ressources->buffer,
                             options[indexElement],
                             yBase + indexElement * pasVertical,
                             selection == indexElement,
                             indexElement != MENU_REPRENDRE_PARTIE || repriseDisponible);
    }

    if (!repriseDisponible) {
        textout_centre_ex(ressources->buffer,
                          font,
                          "Aucune sauvegarde disponible",
                          SCREEN_W / DIVISEUR_CENTRE,
                          SCREEN_H / DIVISEUR_CENTRE + SCREEN_H / DIVISEUR_CINQUIEME,
                          makecol(COULEUR_TEXTE_SAUVEGARDE_R, COULEUR_TEXTE_SAUVEGARDE_G, COULEUR_TEXTE_SAUVEGARDE_B),
                          COULEUR_TRANSPARENTE_TEXTE);
    }

    afficher_buffer_a_l_ecran(ressources->buffer);
}

void dessiner_menu_parametres(const RessourcesJeu *ressources,
                              int selection,
                              int modeDemonstrationActif) {
    int yBase;
    int pasVertical;

    if (!ressources || !ressources->buffer) {
        return;
    }

    dessiner_fond_scene(ressources->buffer, ressources);
    dessiner_panneau_menu(ressources->buffer, SCREEN_W / DIVISEUR_QUART, SCREEN_H / DIVISEUR_QUART);

    textout_centre_ex(ressources->buffer,
                      font,
                      "PARAMETRES",
                      SCREEN_W / DIVISEUR_CENTRE,
                      SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_CINQUIEME,
                      makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B),
                      COULEUR_TRANSPARENTE_TEXTE);

    yBase = SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_DIXIEME;
    pasVertical = SCREEN_H / DIVISEUR_DIX_HUITIEME;

    dessiner_option_menu(ressources->buffer,
                         modeDemonstrationActif ? "Mode demonstration : ON" : "Mode demonstration : OFF",
                         yBase,
                         selection == VALEUR_NULLE,
                         VRAI);

    if (modeDemonstrationActif) {
        int niveauDemo;
        char texte[TAILLE_TEXTE_MENU];

        for (niveauDemo = PREMIER_NIVEAU_JEU; niveauDemo <= NIVEAU_MAXIMUM_JEU; niveauDemo++) {
            uszprintf(texte, sizeof(texte), "Demo niveau %d", niveauDemo);
            dessiner_option_menu(ressources->buffer,
                                 texte,
                                 yBase + pasVertical * niveauDemo,
                                 selection == niveauDemo,
                                 VRAI);
        }
        dessiner_option_menu(ressources->buffer,
                             "Retour",
                             yBase + pasVertical * (NIVEAU_MAXIMUM_JEU + INDEX_SUIVANT),
                             selection == NIVEAU_MAXIMUM_JEU + INDEX_SUIVANT,
                             VRAI);
    } else {
        dessiner_option_menu(ressources->buffer,
                             "Retour",
                             yBase + pasVertical,
                             selection == PARAM_SELECTION_RETOUR_SIMPLE,
                             VRAI);
    }

    textout_centre_ex(ressources->buffer,
                      font,
                      "Entree pour valider, ESC pour revenir",
                      SCREEN_W / DIVISEUR_CENTRE,
                      SCREEN_H / DIVISEUR_CENTRE + SCREEN_H / DIVISEUR_CINQUIEME,
                      makecol(COULEUR_TEXTE_INSTRUCTION_R, COULEUR_TEXTE_INSTRUCTION_G, COULEUR_TEXTE_INSTRUCTION_B),
                      COULEUR_TRANSPARENTE_TEXTE);

    afficher_buffer_a_l_ecran(ressources->buffer);
}

void dessiner_selection_niveau(const RessourcesJeu *ressources,
                               int selection,
                               int niveauMaximumDebloque,
                               int niveauMaximumTotal) {
    int indexElement;
    int yBase;
    int pasVertical;
    char texte[TAILLE_TEXTE_MENU];

    if (!ressources || !ressources->buffer) {
        return;
    }

    if (niveauMaximumDebloque < PREMIER_NIVEAU_JEU) {
        niveauMaximumDebloque = PREMIER_NIVEAU_JEU;
    }
    if (niveauMaximumDebloque > niveauMaximumTotal) {
        niveauMaximumDebloque = niveauMaximumTotal;
    }

    dessiner_fond_scene(ressources->buffer, ressources);
    dessiner_panneau_menu(ressources->buffer, SCREEN_W / DIVISEUR_QUART, SCREEN_H / DIVISEUR_QUART);

    textout_centre_ex(ressources->buffer,
                      font,
                      "CHOIX DU NIVEAU",
                      SCREEN_W / DIVISEUR_CENTRE,
                      SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_CINQUIEME,
                      makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B),
                      COULEUR_TRANSPARENTE_TEXTE);

    yBase = SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_DIXIEME;
    pasVertical = SCREEN_H / DIVISEUR_DIX_HUITIEME;

    for (indexElement = INDEX_PREMIER; indexElement < niveauMaximumDebloque; indexElement++) {
        uszprintf(texte, sizeof(texte), "Niveau %d", indexElement + INDEX_SUIVANT);
        dessiner_option_menu(ressources->buffer,
                             texte,
                             yBase + indexElement * pasVertical,
                             selection == indexElement,
                             VRAI);
    }

    dessiner_option_menu(ressources->buffer,
                         "Retour menu",
                         yBase + niveauMaximumDebloque * pasVertical,
                         selection == niveauMaximumDebloque,
                         VRAI);

    afficher_buffer_a_l_ecran(ressources->buffer);
}

void dessiner_selection_sauvegarde(const RessourcesJeu *ressources,
                                   int selection,
                                   const char pseudos[NOMBRE_SLOTS_SAUVEGARDE][TAILLE_PSEUDO_MAX],
                                   const int sauvegardesDisponibles[NOMBRE_SLOTS_SAUVEGARDE]) {
    int indexElement;
    int yBase;
    int pasVertical;
    char texte[TAILLE_TEXTE_MENU];

    if (!ressources || !ressources->buffer || !pseudos || !sauvegardesDisponibles) {
        return;
    }

    dessiner_fond_scene(ressources->buffer, ressources);
    dessiner_panneau_menu(ressources->buffer, SCREEN_W / DIVISEUR_QUART, SCREEN_H / DIVISEUR_QUART);

    textout_centre_ex(ressources->buffer,
                      font,
                      "CHOIX DE SAUVEGARDE",
                      SCREEN_W / DIVISEUR_CENTRE,
                      SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_CINQUIEME,
                      makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B),
                      COULEUR_TRANSPARENTE_TEXTE);

    yBase = SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_DIXIEME;
    pasVertical = SCREEN_H / DIVISEUR_DIX_HUITIEME;

    for (indexElement = INDEX_PREMIER; indexElement < NOMBRE_SLOTS_SAUVEGARDE; indexElement++) {
        if (sauvegardesDisponibles[indexElement]) {
            uszprintf(texte, sizeof(texte), "Pseudo : %s", pseudos[indexElement]);
        } else {
            uszprintf(texte, sizeof(texte), "Emplacement libre");
        }

        dessiner_option_menu(ressources->buffer,
                             texte,
                             yBase + indexElement * pasVertical,
                             selection == indexElement,
                             sauvegardesDisponibles[indexElement]);
    }

    dessiner_option_menu(ressources->buffer,
                         "Retour menu",
                         yBase + NOMBRE_SLOTS_SAUVEGARDE * pasVertical,
                         selection == NOMBRE_SLOTS_SAUVEGARDE,
                         VRAI);

    afficher_buffer_a_l_ecran(ressources->buffer);
}

void dessiner_ecran_information(const RessourcesJeu *ressources,
                                const char *titre,
                                const char *ligne1,
                                const char *ligne2,
                                const char *ligne3) {
    if (!ressources || !ressources->buffer) {
        return;
    }

    dessiner_fond_scene(ressources->buffer, ressources);
    dessiner_panneau_menu(ressources->buffer, SCREEN_W / DIVISEUR_QUART, SCREEN_H / DIVISEUR_QUART);

    textout_centre_ex(ressources->buffer, font, titre, SCREEN_W / DIVISEUR_CENTRE, SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_SIXIEME, makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B), COULEUR_TRANSPARENTE_TEXTE);
    textout_centre_ex(ressources->buffer, font, ligne1, SCREEN_W / DIVISEUR_CENTRE, SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_DIX_HUITIEME, makecol(COULEUR_TEXTE_INFO_R, COULEUR_TEXTE_INFO_G, COULEUR_TEXTE_INFO_B), COULEUR_TRANSPARENTE_TEXTE);
    textout_centre_ex(ressources->buffer, font, ligne2, SCREEN_W / DIVISEUR_CENTRE, SCREEN_H / DIVISEUR_CENTRE, makecol(COULEUR_TEXTE_INFO_R, COULEUR_TEXTE_INFO_G, COULEUR_TEXTE_INFO_B), COULEUR_TRANSPARENTE_TEXTE);
    textout_centre_ex(ressources->buffer, font, ligne3, SCREEN_W / DIVISEUR_CENTRE, SCREEN_H / DIVISEUR_CENTRE + SCREEN_H / DIVISEUR_DIX_HUITIEME, makecol(COULEUR_TEXTE_INFO_R, COULEUR_TEXTE_INFO_G, COULEUR_TEXTE_INFO_B), COULEUR_TRANSPARENTE_TEXTE);
    textout_centre_ex(ressources->buffer,
                      font,
                      "ESC ou Retour arriere pour revenir au menu",
                      SCREEN_W / DIVISEUR_CENTRE,
                      SCREEN_H / DIVISEUR_CENTRE + SCREEN_H / DIVISEUR_SEPTIEME,
                      makecol(COULEUR_TEXTE_INSTRUCTION_R, COULEUR_TEXTE_INSTRUCTION_G, COULEUR_TEXTE_INSTRUCTION_B),
                      COULEUR_TRANSPARENTE_TEXTE);

    afficher_buffer_a_l_ecran(ressources->buffer);
}

void dessiner_saisie_pseudo(const RessourcesJeu *ressources, const char *pseudoJoueur) {
    int demiLargeur;
    int demiHauteur;
    int champDemiLargeur;
    int champHauteur;

    if (!ressources || !ressources->buffer) {
        return;
    }

    demiLargeur = SCREEN_W / DIVISEUR_QUART;
    demiHauteur = SCREEN_H / DIVISEUR_CINQUIEME;
    champDemiLargeur = SCREEN_W / DIVISEUR_SIXIEME;
    champHauteur = SCREEN_H / DIVISEUR_DIX_HUITIEME;

    dessiner_fond_scene(ressources->buffer, ressources);
    dessiner_panneau_menu(ressources->buffer, demiLargeur, demiHauteur);

    textout_centre_ex(ressources->buffer, font, "NOUVELLE PARTIE", SCREEN_W / DIVISEUR_CENTRE, SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_SEPTIEME, makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B), COULEUR_TRANSPARENTE_TEXTE);
    textout_centre_ex(ressources->buffer,
                      font,
                      "Entrez votre pseudo puis appuyez sur Entree",
                      SCREEN_W / DIVISEUR_CENTRE,
                      SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_DIXIEME,
                      makecol(COULEUR_TEXTE_SAISIE_R, COULEUR_TEXTE_SAISIE_G, COULEUR_TEXTE_SAISIE_B),
                      COULEUR_TRANSPARENTE_TEXTE);

    rectfill(ressources->buffer,
             SCREEN_W / DIVISEUR_CENTRE - champDemiLargeur,
             SCREEN_H / DIVISEUR_CENTRE - champHauteur / DIVISEUR_CENTRE,
             SCREEN_W / DIVISEUR_CENTRE + champDemiLargeur,
             SCREEN_H / DIVISEUR_CENTRE + champHauteur / DIVISEUR_CENTRE,
             makecol(COULEUR_OPTION_SELECTION_TEXTE_R, COULEUR_OPTION_SELECTION_TEXTE_G, COULEUR_OPTION_SELECTION_TEXTE_B));
    rect(ressources->buffer,
         SCREEN_W / DIVISEUR_CENTRE - champDemiLargeur,
         SCREEN_H / DIVISEUR_CENTRE - champHauteur / DIVISEUR_CENTRE,
         SCREEN_W / DIVISEUR_CENTRE + champDemiLargeur,
         SCREEN_H / DIVISEUR_CENTRE + champHauteur / DIVISEUR_CENTRE,
         makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B));
    textprintf_centre_ex(ressources->buffer,
                         font,
                         SCREEN_W / DIVISEUR_CENTRE,
                         SCREEN_H / DIVISEUR_CENTRE - text_height(font) / DIVISEUR_CENTRE,
                         makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B),
                         COULEUR_TRANSPARENTE_TEXTE,
                         "%s_",
                         pseudoJoueur);
    textout_centre_ex(ressources->buffer,
                      font,
                      "ESC pour revenir au menu",
                      SCREEN_W / DIVISEUR_CENTRE,
                      SCREEN_H / DIVISEUR_CENTRE + SCREEN_H / DIVISEUR_DIXIEME,
                      makecol(COULEUR_TEXTE_INSTRUCTION_R, COULEUR_TEXTE_INSTRUCTION_G, COULEUR_TEXTE_INSTRUCTION_B),
                      COULEUR_TRANSPARENTE_TEXTE);

    afficher_buffer_a_l_ecran(ressources->buffer);
}

void dessiner_decompte_depart(const RessourcesJeu *ressources, const EtatJeu *etatJeu, int valeur) {
    char texte[TAILLE_TEXTE_DECOMPTE];

    if (!ressources || !etatJeu || !ressources->buffer) {
        return;
    }

    dessiner_scene_jeu(ressources->buffer, ressources, etatJeu);
    uszprintf(texte, sizeof(texte), "%d", valeur);
    dessiner_texte_centre_aggrandi(ressources->buffer,
                                   texte,
                                   SCREEN_W / DIVISEUR_CENTRE,
                                   SCREEN_H / DIVISEUR_CENTRE,
                                   makecol(COULEUR_BLANC_R, COULEUR_BLANC_G, COULEUR_BLANC_B),
                                   makecol(COULEUR_NOIR_R, COULEUR_NOIR_G, COULEUR_NOIR_B),
                                   SCREEN_H / DIVISEUR_CINQUIEME);

    afficher_buffer_a_l_ecran(ressources->buffer);
}

void dessiner_jeu(const RessourcesJeu *ressources, const EtatJeu *etatJeu) {
    if (!ressources || !etatJeu || !ressources->buffer || !ressources->player) {
        return;
    }

    dessiner_scene_jeu(ressources->buffer, ressources, etatJeu);
    dessiner_panneau_hud(ressources->buffer, etatJeu);
    dessiner_barre_vie_boss(ressources->buffer, etatJeu);

    if (etatJeu->partiePerdue) {
        afficher_message_centre(ressources->buffer, SCREEN_W / DIVISEUR_SEPTIEME, makecol(COULEUR_ROUGE_R, COULEUR_ROUGE_G, COULEUR_ROUGE_B), makecol(COULEUR_ROUGE_R, COULEUR_ROUGE_G, COULEUR_ROUGE_B), "PERDU");
    }
    if (etatJeu->partieGagnee) {
        if (etatJeu->niveauActuel < etatJeu->niveauMaximum) {
            afficher_message_centre(ressources->buffer, SCREEN_W / DIVISEUR_CINQUIEME, makecol(COULEUR_VERT_R, COULEUR_VERT_G, COULEUR_VERT_B), makecol(COULEUR_VERT_R, COULEUR_VERT_G, COULEUR_VERT_B), "GAGNE - ENTREE POUR CHOIX NIVEAU");
        } else {
            afficher_message_centre(ressources->buffer, SCREEN_W / DIVISEUR_CINQUIEME, makecol(COULEUR_VERT_R, COULEUR_VERT_G, COULEUR_VERT_B), makecol(COULEUR_VERT_R, COULEUR_VERT_G, COULEUR_VERT_B), "VICTOIRE FINALE - ENTREE POUR MENU");
        }
    }

    afficher_buffer_a_l_ecran(ressources->buffer);
}

void liberer_bitmap(BITMAP **bitmap) {
    if (bitmap && *bitmap) {
        destroy_bitmap(*bitmap);
        *bitmap = NULL;
    }
}

void liberer_ressources_jeu(RessourcesJeu *ressources) {
    int indexElement;

    if (!ressources) {
        return;
    }

    liberer_bitmap(&ressources->buffer);
    for (indexElement = INDEX_SUIVANT; indexElement < NOMBRE_FONDS_NIVEAUX; indexElement++) {
        liberer_bitmap(&ressources->fondsNiveaux[indexElement]);
    }
    ressources->fondsNiveaux[CHAINE_DEBUT] = NULL;
    for (indexElement = INDEX_PREMIER; indexElement < BULLE_TAILLES_TOTAL; indexElement++) {
        liberer_bitmap(&ressources->sprites[indexElement]);
        liberer_bitmap(&ressources->spritesVifDor[indexElement]);
        liberer_bitmap(&ressources->spritesVolDeMort[indexElement]);
    }
    liberer_bitmap(&ressources->player);
    liberer_bitmap(&ressources->tir);
    liberer_bitmap(&ressources->feu);
    liberer_bitmap(&ressources->chapeau);
    liberer_bitmap(&ressources->explosion);
    liberer_bitmap(&ressources->fond);
}

void fermer_affichage(void) {
    allegro_exit();
}
