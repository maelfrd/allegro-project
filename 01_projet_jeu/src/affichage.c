#include "affichage.h"

enum {
    MENU_NOUVELLE_PARTIE = 0,
    MENU_REPRENDRE_PARTIE,
    MENU_QUITTER,
    MENU_PARAMETRES,
    MENU_REGLES
};

static void initialiser_ressources_vides(RessourcesJeu *ressources) {
    int i;

    ressources->fond = NULL;
    ressources->player = NULL;
    ressources->tir = NULL;
    ressources->feu = NULL;
    ressources->chapeau = NULL;
    ressources->explosion = NULL;
    ressources->buffer = NULL;
    for (i = 0; i < BULLE_TAILLES_TOTAL; i++) {
        ressources->sprites[i] = NULL;
        ressources->spritesVifDor[i] = NULL;
    }
}

static void dessiner_boite_centrale(BITMAP *buffer,
                                    int demiLargeur,
                                    int demiHauteur,
                                    int couleurFond,
                                    int couleurContour) {
    int centreX = SCREEN_W / 2;
    int centreY = SCREEN_H / 2;

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
    dessiner_boite_centrale(buffer,
                            demiLargeur,
                            SCREEN_H / 14,
                            makecol(0, 0, 0),
                            couleurBord);

    textout_centre_ex(buffer,
                      font,
                      message,
                      SCREEN_W / 2,
                      SCREEN_H / 2 - text_height(font) / 2,
                      couleurTexte,
                      -1);
}

static void dessiner_fond_scene(BITMAP *buffer, const RessourcesJeu *ressources) {
    clear_to_color(buffer, makecol(0, 0, 0));

    if (ressources && ressources->fond) {
        stretch_blit(ressources->fond,
                     buffer,
                     0,
                     0,
                     ressources->fond->w,
                     ressources->fond->h,
                     0,
                     0,
                     SCREEN_W,
                     SCREEN_H);
    }
}

static void dessiner_panneau_menu(BITMAP *buffer, int demiLargeur, int demiHauteur) {
    dessiner_boite_centrale(buffer,
                            demiLargeur,
                            demiHauteur,
                            makecol(10, 10, 10),
                            makecol(170, 170, 170));
    rect(buffer,
         SCREEN_W / 2 - demiLargeur + 4,
         SCREEN_H / 2 - demiHauteur + 4,
         SCREEN_W / 2 + demiLargeur - 4,
         SCREEN_H / 2 + demiHauteur - 4,
         makecol(60, 60, 60));
}

static void dessiner_texte_centre_aggrandi(BITMAP *buffer,
                                           const char *texte,
                                           int centreX,
                                           int centreY,
                                           int couleurTexte,
                                           int couleurFond,
                                           int facteur) {
    BITMAP *texteBitmap;
    int largeurTexte;
    int hauteurTexte;
    int largeurFinale;
    int hauteurFinale;

    if (!buffer || !texte || facteur <= 0) {
        return;
    }

    largeurTexte = text_length(font, texte);
    hauteurTexte = text_height(font);
    if (largeurTexte <= 0 || hauteurTexte <= 0) {
        return;
    }

    texteBitmap = create_bitmap(largeurTexte, hauteurTexte);
    if (!texteBitmap) {
        return;
    }

    clear_to_color(texteBitmap, makecol(255, 0, 255));
    textout_centre_ex(texteBitmap, font, texte, largeurTexte / 2, 0, couleurTexte, -1);

    largeurFinale = largeurTexte * facteur;
    hauteurFinale = hauteurTexte * facteur;

    rectfill(buffer,
             centreX - largeurFinale / 2 - SCREEN_W / 40,
             centreY - hauteurFinale / 2 - SCREEN_H / 36,
             centreX + largeurFinale / 2 + SCREEN_W / 40,
             centreY + hauteurFinale / 2 + SCREEN_H / 36,
             couleurFond);
    rect(buffer,
         centreX - largeurFinale / 2 - SCREEN_W / 40,
         centreY - hauteurFinale / 2 - SCREEN_H / 36,
         centreX + largeurFinale / 2 + SCREEN_W / 40,
         centreY + hauteurFinale / 2 + SCREEN_H / 36,
         makecol(255, 255, 255));

    stretch_sprite(buffer,
                   texteBitmap,
                   centreX - largeurFinale / 2,
                   centreY - hauteurFinale / 2,
                   largeurFinale,
                   hauteurFinale);

    destroy_bitmap(texteBitmap);
}

static void dessiner_scene_jeu(BITMAP *buffer, const RessourcesJeu *ressources, const EtatJeu *etat) {
    int i;

    if (!buffer || !ressources || !etat || !ressources->player) {
        return;
    }

    dessiner_fond_scene(buffer, ressources);

    rectfill(buffer,
             0,
             etat->groundY,
             SCREEN_W,
             SCREEN_H,
             makecol(70, 120, 55));

    for (i = 0; i < etat->nbBulles; i++) {
        BITMAP *sprite;

        if (etat->bulles[i].type == ENTITE_VIF_DOR) {
            sprite = ressources->spritesVifDor[(int) etat->bulles[i].taille];
        } else {
            sprite = ressources->sprites[(int) etat->bulles[i].taille];
        }

        if (!sprite) {
            continue;
        }

        masked_blit(sprite,
                    buffer,
                    0,
                    0,
                    (int) etat->bulles[i].x,
                    (int) etat->bulles[i].y,
                    sprite->w,
                    sprite->h);
    }

    masked_blit(ressources->player,
                buffer,
                0,
                0,
                etat->x,
                etat->y,
                ressources->player->w,
                ressources->player->h);

    if (etat->projectileActive && (etat->modeFeuActif ? ressources->feu : ressources->tir)) {
        BITMAP *projectileSprite = etat->modeFeuActif ? ressources->feu : ressources->tir;

        masked_blit(projectileSprite,
                    buffer,
                    0,
                    0,
                    etat->projectileX - projectileSprite->w / 2 + etat->projectileW / 2,
                    etat->projectileY - projectileSprite->h + etat->projectileH,
                    projectileSprite->w,
                    projectileSprite->h);
    }

    if (etat->chapeauVisible && ressources->chapeau) {
        masked_blit(ressources->chapeau,
                    buffer,
                    0,
                    0,
                    etat->chapeauX,
                    etat->chapeauY,
                    ressources->chapeau->w,
                    ressources->chapeau->h);
    }

    if (etat->explosionActive && ressources->explosion) {
        masked_blit(ressources->explosion,
                    buffer,
                    0,
                    0,
                    etat->explosionX,
                    etat->explosionY,
                    ressources->explosion->w,
                    ressources->explosion->h);
    }
}

static void dessiner_option_menu(BITMAP *buffer,
                                 const char *texte,
                                 int y,
                                 int selectionnee,
                                 int active) {
    int demiLargeur = SCREEN_W / 6;
    int demiHauteur = SCREEN_H / 45;
    int x1 = SCREEN_W / 2 - demiLargeur;
    int x2 = SCREEN_W / 2 + demiLargeur;
    int couleurFond;
    int couleurTexte;
    int couleurCadre;

    if (!active) {
        couleurFond = makecol(18, 18, 18);
        couleurTexte = makecol(90, 90, 90);
        couleurCadre = makecol(40, 40, 40);
    } else if (selectionnee) {
        couleurFond = makecol(230, 230, 230);
        couleurTexte = makecol(20, 20, 20);
        couleurCadre = makecol(255, 255, 255);
    } else {
        couleurFond = makecol(24, 24, 24);
        couleurTexte = makecol(220, 220, 220);
        couleurCadre = makecol(90, 90, 90);
    }

    rectfill(buffer, x1, y - demiHauteur, x2, y + demiHauteur, couleurFond);
    rect(buffer, x1, y - demiHauteur, x2, y + demiHauteur, couleurCadre);
    textout_centre_ex(buffer, font, texte, SCREEN_W / 2, y - text_height(font) / 2, couleurTexte, -1);
}

static void normaliser_transparence_magenta(BITMAP *bitmap) {
    int x;
    int y;
    int couleurMasque;

    if (!bitmap) {
        return;
    }

    couleurMasque = bitmap_mask_color(bitmap);
    for (y = 0; y < bitmap->h; y++) {
        for (x = 0; x < bitmap->w; x++) {
            int couleur = getpixel(bitmap, x, y);
            int r = getr(couleur);
            int g = getg(couleur);
            int b = getb(couleur);

            if (r >= 220 && g <= 80 && b >= 220) {
                putpixel(bitmap, x, y, couleurMasque);
            }
        }
    }
}

int initialiser_affichage(int largeur, int hauteur, int profondeur_couleur) {
    allegro_init();

    if (install_keyboard() != 0) {
        allegro_message("Erreur d'initialisation du clavier");
        return 0;
    }

    set_color_depth(profondeur_couleur);
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, largeur, hauteur, 0, 0) != 0) {
        allegro_message("Erreur mode graphique");
        return 0;
    }

    return 1;
}

BITMAP *charger_bitmap_ou_erreur(const char *chemin) {
    BITMAP *bitmap;

    if (!chemin || chemin[0] == '\0') {
        allegro_message("Chemin de bitmap invalide");
        return NULL;
    }

    bitmap = load_bitmap(chemin, NULL);
    if (!bitmap) {
        allegro_message("Impossible de charger %s", chemin);
    }

    return bitmap;
}

BITMAP *resize_bitmap(BITMAP *src, float scale) {
    int newW;
    int newH;
    BITMAP *dest;

    if (!src) {
        return NULL;
    }

    newW = (int) (src->w * scale);
    newH = (int) (src->h * scale);
    if (newW <= 0) {
        newW = 1;
    }
    if (newH <= 0) {
        newH = 1;
    }

    dest = create_bitmap(newW, newH);
    if (!dest) {
        return NULL;
    }

    clear_to_color(dest, makecol(255, 0, 255));
    stretch_blit(src, dest, 0, 0, src->w, src->h, 0, 0, newW, newH);

    return dest;
}

static BITMAP *resize_bitmap_dimensions(BITMAP *src, int largeur, int hauteur) {
    BITMAP *dest;

    if (!src || largeur <= 0 || hauteur <= 0) {
        return NULL;
    }

    dest = create_bitmap(largeur, hauteur);
    if (!dest) {
        return NULL;
    }

    clear_to_color(dest, makecol(255, 0, 255));
    stretch_blit(src, dest, 0, 0, src->w, src->h, 0, 0, largeur, hauteur);
    return dest;
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

    if (!ressources) {
        return 0;
    }

    initialiser_ressources_vides(ressources);

    ressources->fond = charger_bitmap_ou_erreur(fond_path);
    if (!ressources->fond) {
        liberer_ressources_jeu(ressources);
        return 0;
    }

    player_orig = charger_bitmap_ou_erreur(player_path);
    if (!player_orig) {
        liberer_ressources_jeu(ressources);
        return 0;
    }

    ressources->player = resize_bitmap(player_orig, 0.25f);
    destroy_bitmap(player_orig);
    if (!ressources->player) {
        allegro_message("Erreur resize personnage");
        liberer_ressources_jeu(ressources);
        return 0;
    }

    tir_orig = charger_bitmap_ou_erreur(tir_path);
    if (!tir_orig) {
        liberer_ressources_jeu(ressources);
        return 0;
    }

    ressources->tir = resize_bitmap(tir_orig, 0.18f);
    destroy_bitmap(tir_orig);
    if (!ressources->tir) {
        allegro_message("Erreur resize tir");
        liberer_ressources_jeu(ressources);
        return 0;
    }

    feu_orig = charger_bitmap_ou_erreur(feu_path);
    if (!feu_orig) {
        liberer_ressources_jeu(ressources);
        return 0;
    }

    ressources->feu = resize_bitmap(feu_orig, 0.18f);
    destroy_bitmap(feu_orig);
    if (!ressources->feu) {
        allegro_message("Erreur resize feu");
        liberer_ressources_jeu(ressources);
        return 0;
    }

    chapeau_orig = charger_bitmap_ou_erreur(chapeau_path);
    if (!chapeau_orig) {
        liberer_ressources_jeu(ressources);
        return 0;
    }

    ressources->chapeau = resize_bitmap(chapeau_orig, 0.20f);
    destroy_bitmap(chapeau_orig);
    if (!ressources->chapeau) {
        allegro_message("Erreur resize chapeau");
        liberer_ressources_jeu(ressources);
        return 0;
    }

    explosion_orig = charger_bitmap_ou_erreur(explosion_path);
    if (!explosion_orig) {
        liberer_ressources_jeu(ressources);
        return 0;
    }

    ressources->explosion = resize_bitmap(explosion_orig, 1.10f);
    destroy_bitmap(explosion_orig);
    if (!ressources->explosion) {
        allegro_message("Erreur resize explosion");
        liberer_ressources_jeu(ressources);
        return 0;
    }

    bulle_orig = charger_bitmap_ou_erreur(mangemort_path);
    if (!bulle_orig) {
        liberer_ressources_jeu(ressources);
        return 0;
    }

    ressources->sprites[BULLE_TRES_GRANDE] = resize_bitmap(bulle_orig, 0.22f);
    ressources->sprites[BULLE_GRANDE] = resize_bitmap(bulle_orig, 0.17f);
    ressources->sprites[BULLE_MOYENNE] = resize_bitmap(bulle_orig, 0.12f);
    ressources->sprites[BULLE_PETITE] = resize_bitmap(bulle_orig, 0.08f);
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
        return 0;
    }

    vifdor_orig = charger_bitmap_ou_erreur(vifdor_path);
    if (!vifdor_orig) {
        liberer_ressources_jeu(ressources);
        return 0;
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
        return 0;
    }

    ressources->buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!ressources->buffer) {
        allegro_message("Impossible de creer le buffer");
        liberer_ressources_jeu(ressources);
        return 0;
    }

    return 1;
}

void dessiner_menu_depart(const RessourcesJeu *ressources, int selection, int repriseDisponible) {
    static const char *options[] = {
        "Nouvelle partie",
        "Reprendre partie",
        "Quitter",
        "Parametres",
        "Regles"
    };
    int i;
    int yBase;
    int pasVertical;

    if (!ressources || !ressources->buffer) {
        return;
    }

    dessiner_fond_scene(ressources->buffer, ressources);
    dessiner_panneau_menu(ressources->buffer, SCREEN_W / 4, SCREEN_H / 4);

    textout_centre_ex(ressources->buffer,
                      font,
                      "MENU DE DEPART",
                      SCREEN_W / 2,
                      SCREEN_H / 2 - SCREEN_H / 5,
                      makecol(255, 255, 255),
                      -1);
    textout_centre_ex(ressources->buffer,
                      font,
                      "Utilisez Haut/Bas puis Entree",
                      SCREEN_W / 2,
                      SCREEN_H / 2 - SCREEN_H / 6,
                      makecol(150, 150, 150),
                      -1);

    yBase = SCREEN_H / 2 - SCREEN_H / 13;
    pasVertical = SCREEN_H / 18;
    for (i = 0; i < 5; i++) {
        dessiner_option_menu(ressources->buffer,
                             options[i],
                             yBase + i * pasVertical,
                             selection == i,
                             i != MENU_REPRENDRE_PARTIE || repriseDisponible);
    }

    if (!repriseDisponible) {
        textout_centre_ex(ressources->buffer,
                          font,
                          "Aucune sauvegarde disponible",
                          SCREEN_W / 2,
                          SCREEN_H / 2 + SCREEN_H / 5,
                          makecol(120, 120, 120),
                          -1);
    }

    blit(ressources->buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
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
    dessiner_panneau_menu(ressources->buffer, SCREEN_W / 4, SCREEN_H / 4);

    textout_centre_ex(ressources->buffer,
                      font,
                      "PARAMETRES",
                      SCREEN_W / 2,
                      SCREEN_H / 2 - SCREEN_H / 5,
                      makecol(255, 255, 255),
                      -1);

    yBase = SCREEN_H / 2 - SCREEN_H / 10;
    pasVertical = SCREEN_H / 18;

    dessiner_option_menu(ressources->buffer,
                         modeDemonstrationActif ? "Mode demonstration : ON" : "Mode demonstration : OFF",
                         yBase,
                         selection == 0,
                         1);

    if (modeDemonstrationActif) {
        dessiner_option_menu(ressources->buffer, "Demo niveau 1", yBase + pasVertical, selection == 1, 1);
        dessiner_option_menu(ressources->buffer, "Demo niveau 2", yBase + pasVertical * 2, selection == 2, 1);
        dessiner_option_menu(ressources->buffer, "Demo niveau 3", yBase + pasVertical * 3, selection == 3, 1);
        dessiner_option_menu(ressources->buffer, "Demo niveau 4", yBase + pasVertical * 4, selection == 4, 1);
        dessiner_option_menu(ressources->buffer, "Retour", yBase + pasVertical * 5, selection == 5, 1);
    } else {
        dessiner_option_menu(ressources->buffer, "Retour", yBase + pasVertical, selection == 1, 1);
    }

    textout_centre_ex(ressources->buffer,
                      font,
                      "Entree pour valider, ESC pour revenir",
                      SCREEN_W / 2,
                      SCREEN_H / 2 + SCREEN_H / 5,
                      makecol(140, 140, 140),
                      -1);

    blit(ressources->buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
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
    dessiner_panneau_menu(ressources->buffer, SCREEN_W / 4, SCREEN_H / 4);

    textout_centre_ex(ressources->buffer, font, titre, SCREEN_W / 2, SCREEN_H / 2 - SCREEN_H / 6, makecol(255, 255, 255), -1);
    textout_centre_ex(ressources->buffer, font, ligne1, SCREEN_W / 2, SCREEN_H / 2 - SCREEN_H / 18, makecol(210, 210, 210), -1);
    textout_centre_ex(ressources->buffer, font, ligne2, SCREEN_W / 2, SCREEN_H / 2, makecol(210, 210, 210), -1);
    textout_centre_ex(ressources->buffer, font, ligne3, SCREEN_W / 2, SCREEN_H / 2 + SCREEN_H / 18, makecol(210, 210, 210), -1);
    textout_centre_ex(ressources->buffer,
                      font,
                      "ESC ou Retour arriere pour revenir au menu",
                      SCREEN_W / 2,
                      SCREEN_H / 2 + SCREEN_H / 7,
                      makecol(140, 140, 140),
                      -1);

    blit(ressources->buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

void dessiner_saisie_pseudo(const RessourcesJeu *ressources, const char *pseudo) {
    int demiLargeur = SCREEN_W / 4;
    int demiHauteur = SCREEN_H / 5;
    int champDemiLargeur = SCREEN_W / 6;
    int champHauteur = SCREEN_H / 18;

    if (!ressources || !ressources->buffer) {
        return;
    }

    dessiner_fond_scene(ressources->buffer, ressources);
    dessiner_panneau_menu(ressources->buffer, demiLargeur, demiHauteur);

    textout_centre_ex(ressources->buffer, font, "NOUVELLE PARTIE", SCREEN_W / 2, SCREEN_H / 2 - SCREEN_H / 7, makecol(255, 255, 255), -1);
    textout_centre_ex(ressources->buffer,
                      font,
                      "Entrez votre pseudo puis appuyez sur Entree",
                      SCREEN_W / 2,
                      SCREEN_H / 2 - SCREEN_H / 10,
                      makecol(200, 200, 200),
                      -1);

    rectfill(ressources->buffer,
             SCREEN_W / 2 - champDemiLargeur,
             SCREEN_H / 2 - champHauteur / 2,
             SCREEN_W / 2 + champDemiLargeur,
             SCREEN_H / 2 + champHauteur / 2,
             makecol(20, 20, 20));
    rect(ressources->buffer,
         SCREEN_W / 2 - champDemiLargeur,
         SCREEN_H / 2 - champHauteur / 2,
         SCREEN_W / 2 + champDemiLargeur,
         SCREEN_H / 2 + champHauteur / 2,
         makecol(255, 255, 255));
    textprintf_centre_ex(ressources->buffer,
                         font,
                         SCREEN_W / 2,
                         SCREEN_H / 2 - text_height(font) / 2,
                         makecol(255, 255, 255),
                         -1,
                         "%s_",
                         pseudo);
    textout_centre_ex(ressources->buffer,
                      font,
                      "ESC pour revenir au menu",
                      SCREEN_W / 2,
                      SCREEN_H / 2 + SCREEN_H / 10,
                      makecol(140, 140, 140),
                      -1);

    blit(ressources->buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

void dessiner_decompte_depart(const RessourcesJeu *ressources, const EtatJeu *etat, int valeur) {
    char texte[8];

    if (!ressources || !etat || !ressources->buffer) {
        return;
    }

    dessiner_scene_jeu(ressources->buffer, ressources, etat);
    uszprintf(texte, sizeof(texte), "%d", valeur);
    dessiner_texte_centre_aggrandi(ressources->buffer,
                                   texte,
                                   SCREEN_W / 2,
                                   SCREEN_H / 2,
                                   makecol(255, 255, 255),
                                   makecol(0, 0, 0),
                                   10);

    blit(ressources->buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

void dessiner_jeu(const RessourcesJeu *ressources, const EtatJeu *etat) {
    if (!ressources || !etat || !ressources->buffer || !ressources->player) {
        return;
    }

    dessiner_scene_jeu(ressources->buffer, ressources, etat);
    textprintf_ex(ressources->buffer,
                  font,
                  SCREEN_W / 40,
                  SCREEN_H / 36,
                  makecol(255, 255, 255),
                  -1,
                  "Niveau %d/%d",
                  etat->niveau,
                  etat->niveauMaximum);
    if (etat->modeFeuActif) {
        textout_ex(ressources->buffer,
                   font,
                   "Pouvoir feu actif",
                   SCREEN_W / 40,
                   SCREEN_H / 18,
                   makecol(255, 120, 60),
                   -1);
    }

    if (etat->perdu) {
        afficher_message_centre(ressources->buffer, SCREEN_W / 7, makecol(255, 0, 0), makecol(255, 0, 0), "PERDU");
    }
    if (etat->gagne) {
        if (etat->niveau < etat->niveauMaximum) {
            afficher_message_centre(ressources->buffer, SCREEN_W / 5, makecol(0, 255, 0), makecol(0, 255, 0), "GAGNE - ENTREE POUR NIVEAU SUIVANT");
        } else {
            afficher_message_centre(ressources->buffer, SCREEN_W / 5, makecol(0, 255, 0), makecol(0, 255, 0), "VICTOIRE FINALE - ENTREE POUR MENU");
        }
    }

    blit(ressources->buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

void liberer_bitmap(BITMAP **bitmap) {
    if (bitmap && *bitmap) {
        destroy_bitmap(*bitmap);
        *bitmap = NULL;
    }
}

void liberer_ressources_jeu(RessourcesJeu *ressources) {
    int i;

    if (!ressources) {
        return;
    }

    liberer_bitmap(&ressources->buffer);
    for (i = 0; i < BULLE_TAILLES_TOTAL; i++) {
        liberer_bitmap(&ressources->sprites[i]);
        liberer_bitmap(&ressources->spritesVifDor[i]);
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
