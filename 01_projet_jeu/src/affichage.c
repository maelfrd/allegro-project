#include "affichage.h"

#include <stdio.h>
#include <string.h>

enum {
    MENU_NOUVELLE_PARTIE = 0,
    MENU_REPRENDRE_PARTIE,
    MENU_QUITTER,
    MENU_PARAMETRES,
    MENU_REGLES
};

static int obtenir_largeur_zone_dessin(const BITMAP *bitmap) {
    return bitmap ? bitmap->w : 0;
}

static int obtenir_hauteur_zone_dessin(const BITMAP *bitmap) {
    return bitmap ? bitmap->h : 0;
}

static void afficher_buffer_a_l_ecran(BITMAP *buffer) {
    if (!buffer || !screen) {
        return;
    }

    blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
}

static void initialiser_ressources_vides(RessourcesJeu *ressources) {
    int i;

    ressources->fond = NULL;
    for (i = 0; i < NOMBRE_FONDS_NIVEAUX; i++) {
        ressources->fondsNiveaux[i] = NULL;
    }
    ressources->player = NULL;
    ressources->tir = NULL;
    ressources->feu = NULL;
    ressources->chapeau = NULL;
    ressources->explosion = NULL;
    ressources->buffer = NULL;
    for (i = 0; i < BULLE_TAILLES_TOTAL; i++) {
        ressources->sprites[i] = NULL;
        ressources->spritesVifDor[i] = NULL;
        ressources->spritesVolDeMort[i] = NULL;
    }
}

static void dessiner_boite_centrale(BITMAP *buffer,
                                    int demiLargeur,
                                    int demiHauteur,
                                    int couleurFond,
                                    int couleurContour) {
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);
    int centreX = largeurZoneDessin / 2;
    int centreY = hauteurZoneDessin / 2;

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
                            hauteurZoneDessin / 14,
                            makecol(0, 0, 0),
                            couleurBord);

    textout_centre_ex(buffer,
                      font,
                      message,
                      largeurZoneDessin / 2,
                      hauteurZoneDessin / 2 - text_height(font) / 2,
                      couleurTexte,
                      -1);
}

static BITMAP *obtenir_fond_niveau(const RessourcesJeu *ressources, int niveau) {
    if (!ressources) {
        return NULL;
    }

    if (niveau >= 1 && niveau <= NOMBRE_FONDS_NIVEAUX && ressources->fondsNiveaux[niveau - 1]) {
        return ressources->fondsNiveaux[niveau - 1];
    }

    return ressources->fond;
}

static void dessiner_fond_scene(BITMAP *buffer, const RessourcesJeu *ressources) {
    BITMAP *fond = obtenir_fond_niveau(ressources, 1);
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);

    clear_to_color(buffer, makecol(0, 0, 0));

    if (fond) {
        stretch_blit(fond,
                     buffer,
                     0,
                     0,
                     fond->w,
                     fond->h,
                     0,
                     0,
                     largeurZoneDessin,
                     hauteurZoneDessin);
    }
}

static void dessiner_panneau_menu(BITMAP *buffer, int demiLargeur, int demiHauteur) {
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);

    dessiner_boite_centrale(buffer,
                            demiLargeur,
                            demiHauteur,
                            makecol(10, 10, 10),
                            makecol(170, 170, 170));
    rect(buffer,
         largeurZoneDessin / 2 - demiLargeur + 4,
         hauteurZoneDessin / 2 - demiHauteur + 4,
         largeurZoneDessin / 2 + demiLargeur - 4,
         hauteurZoneDessin / 2 + demiHauteur - 4,
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
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);
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
             centreX - largeurFinale / 2 - largeurZoneDessin / 40,
             centreY - hauteurFinale / 2 - hauteurZoneDessin / 36,
             centreX + largeurFinale / 2 + largeurZoneDessin / 40,
             centreY + hauteurFinale / 2 + hauteurZoneDessin / 36,
             couleurFond);
    rect(buffer,
         centreX - largeurFinale / 2 - largeurZoneDessin / 40,
         centreY - hauteurFinale / 2 - hauteurZoneDessin / 36,
         centreX + largeurFinale / 2 + largeurZoneDessin / 40,
         centreY + hauteurFinale / 2 + hauteurZoneDessin / 36,
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
    BITMAP *fond;
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);

    if (!buffer || !ressources || !etat || !ressources->player) {
        return;
    }

    fond = obtenir_fond_niveau(ressources, etat->niveau);
    clear_to_color(buffer, makecol(0, 0, 0));
    if (fond) {
        stretch_blit(fond,
                     buffer,
                     0,
                     0,
                     fond->w,
                     fond->h,
                     0,
                     0,
                     largeurZoneDessin,
                     hauteurZoneDessin);
    }

    rectfill(buffer,
             0,
             etat->groundY,
             largeurZoneDessin,
             hauteurZoneDessin,
             makecol(70, 120, 55));

    for (i = 0; i < etat->nbBulles; i++) {
        BITMAP *sprite;

        if (etat->bulles[i].type == ENTITE_VIF_DOR) {
            sprite = ressources->spritesVifDor[(int) etat->bulles[i].taille];
        } else if (etat->bulles[i].type == ENTITE_VOL_DE_MORT) {
            sprite = ressources->spritesVolDeMort[(int) etat->bulles[i].taille];
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

    if (etat->projectileActive && ressources->tir) {
        BITMAP *projectileSprite = ressources->tir;

        masked_blit(projectileSprite,
                    buffer,
                    0,
                    0,
                    etat->projectileX - projectileSprite->w / 2 + etat->projectileW / 2,
                    etat->projectileY - projectileSprite->h + etat->projectileH,
                    projectileSprite->w,
                    projectileSprite->h);
    }

    if (etat->auraArdenteActive && ressources->feu) {
        int effetX = etat->x - ressources->player->w / 4;
        int effetY = etat->y - ressources->player->h / 3;
        int effetLargeur = ressources->player->w + ressources->player->w / 2;
        int effetHauteur = ressources->player->h + ressources->player->h / 2;

        stretch_sprite(buffer,
                       ressources->feu,
                       effetX,
                       effetY,
                       effetLargeur,
                       effetHauteur);
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
    int largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    int hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);
    int demiLargeur = largeurZoneDessin / 6;
    int demiHauteur = hauteurZoneDessin / 45;
    int x1 = largeurZoneDessin / 2 - demiLargeur;
    int x2 = largeurZoneDessin / 2 + demiLargeur;
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
    textout_centre_ex(buffer, font, texte, largeurZoneDessin / 2, y - text_height(font) / 2, couleurTexte, -1);
}

static void tronquer_texte_pour_largeur(const char *source, char *destination, size_t tailleDestination, int largeurMax) {
    size_t longueur;

    if (!destination || tailleDestination == 0) {
        return;
    }

    destination[0] = '\0';
    if (!source) {
        return;
    }

    strncpy(destination, source, tailleDestination - 1);
    destination[tailleDestination - 1] = '\0';

    if (text_length(font, destination) <= largeurMax) {
        return;
    }

    longueur = strlen(destination);
    while (longueur > 3) {
        longueur--;
        destination[longueur] = '\0';
        if (snprintf(destination + longueur, tailleDestination - longueur, "...") >= (int) (tailleDestination - longueur)) {
            break;
        }
        if (text_length(font, destination) <= largeurMax) {
            return;
        }
        destination[longueur] = '\0';
    }

    strncpy(destination, "...", tailleDestination - 1);
    destination[tailleDestination - 1] = '\0';
}

static void dessiner_panneau_hud(BITMAP *buffer, const EtatJeu *etat) {
    char ligne[128];
    char pseudo[TAILLE_PSEUDO_MAX + 8];
    int largeurZoneDessin;
    int hauteurZoneDessin;
    int margeX;
    int margeY;
    int largeur;
    int hauteur;
    int x1;
    int y1;
    int x2;
    int y2;
    int y;
    int interligne;
    int largeurTexte;
    float tempsSecondes;

    if (!buffer || !etat) {
        return;
    }

    largeurZoneDessin = obtenir_largeur_zone_dessin(buffer);
    hauteurZoneDessin = obtenir_hauteur_zone_dessin(buffer);
    margeX = largeurZoneDessin / 40;
    margeY = hauteurZoneDessin / 36;
    largeur = largeurZoneDessin / 4;
    hauteur = hauteurZoneDessin / 6;
    x1 = margeX;
    y1 = margeY;
    x2 = x1 + largeur;
    y2 = y1 + hauteur;
    interligne = text_height(font) + hauteurZoneDessin / 90;
    largeurTexte = largeur - largeurZoneDessin / 45;

    if (etat->niveau != 1) {
        rectfill(buffer, x1, y1, x2, y2, makecol(150, 20, 20));
        rect(buffer, x1, y1, x2, y2, makecol(255, 235, 235));
        rect(buffer, x1 + 3, y1 + 3, x2 - 3, y2 - 3, makecol(90, 0, 0));
    }

    y = y1 + hauteurZoneDessin / 70;
    textout_ex(buffer, font, "HUD JOUEUR", x1 + largeurZoneDessin / 90, y, makecol(255, 245, 245), -1);

    y += interligne;
    snprintf(ligne, sizeof(ligne), "Niveau : %d/%d", etat->niveau, etat->niveauMaximum);
    textout_ex(buffer, font, ligne, x1 + largeurZoneDessin / 90, y, makecol(255, 255, 255), -1);

    y += interligne;
    snprintf(ligne, sizeof(ligne), "Score : %d", etat->score);
    textout_ex(buffer, font, ligne, x1 + largeurZoneDessin / 90, y, makecol(255, 255, 255), -1);

    y += interligne;
    snprintf(pseudo, sizeof(pseudo), "Pseudo : %s", etat->pseudo[0] != '\0' ? etat->pseudo : "ANONYME");
    tronquer_texte_pour_largeur(pseudo, ligne, sizeof(ligne), largeurTexte);
    textout_ex(buffer, font, ligne, x1 + largeurZoneDessin / 90, y, makecol(255, 255, 255), -1);

    y += interligne;
    if (etat->auraArdenteActive) {
        tempsSecondes = (float) etat->dureeRestanteAuraArdenteMs / 1000.0f;
        snprintf(ligne, sizeof(ligne), "Aura ardente : %.1f s", tempsSecondes);
        textout_ex(buffer, font, ligne, x1 + largeurZoneDessin / 90, y, makecol(255, 240, 120), -1);
    } else {
        textout_ex(buffer, font, "Aura ardente : INACTIVE", x1 + largeurZoneDessin / 90, y, makecol(255, 220, 220), -1);
    }
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

static int fichier_existe_simple(const char *chemin) {
    FILE *fichier;

    if (!chemin || chemin[0] == '\0') {
        return 0;
    }

    fichier = fopen(chemin, "rb");
    if (!fichier) {
        return 0;
    }

    fclose(fichier);
    return 1;
}

static int resoudre_chemin_ressource(const char *chemin, char *destination, size_t taille) {
    static const char *prefixes[] = {
        "",
        "../",
        "../../",
        "01_projet_jeu/",
        "../01_projet_jeu/"
    };
    int i;

    if (!chemin || !destination || taille == 0 || chemin[0] == '\0') {
        return 0;
    }

    if (chemin[0] == '/') {
        if (strlen(chemin) + 1 > taille) {
            return 0;
        }
        strcpy(destination, chemin);
        return fichier_existe_simple(destination);
    }

    for (i = 0; i < (int) (sizeof(prefixes) / sizeof(prefixes[0])); i++) {
        if (snprintf(destination, taille, "%s%s", prefixes[i], chemin) >= (int) taille) {
            continue;
        }
        if (fichier_existe_simple(destination)) {
            return 1;
        }
    }

    destination[0] = '\0';
    return 0;
}

int initialiser_affichage(const char *fond_path,
                          int largeur_defaut,
                          int hauteur_defaut,
                          int profondeur_couleur) {
    int largeur;
    int hauteur;

    (void) fond_path;

    allegro_init();

    if (install_keyboard() != 0) {
        allegro_message("Erreur d'initialisation du clavier");
        return 0;
    }

    set_color_depth(profondeur_couleur);
    largeur = largeur_defaut;
    hauteur = hauteur_defaut;

    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, largeur, hauteur, 0, 0) != 0) {
        allegro_message("Erreur mode graphique");
        return 0;
    }

    return 1;
}

int ressource_existe(const char *chemin) {
    char cheminResolu[512];

    return resoudre_chemin_ressource(chemin, cheminResolu, sizeof(cheminResolu));
}

BITMAP *charger_bitmap_ou_erreur(const char *chemin) {
    BITMAP *bitmap;
    char cheminResolu[512];

    if (!chemin || chemin[0] == '\0') {
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
    BITMAP *volDeMortOrig;

    if (!ressources) {
        return 0;
    }

    initialiser_ressources_vides(ressources);

    ressources->fond = charger_bitmap_ou_erreur(fond_path);
    if (!ressources->fond) {
        liberer_ressources_jeu(ressources);
        return 0;
    }
    ressources->fondsNiveaux[0] = ressources->fond;
    ressources->fondsNiveaux[1] = charger_bitmap_ou_erreur("assets/room_24bit.bmp");
    ressources->fondsNiveaux[2] = charger_bitmap_ou_erreur("assets/library_24bit.bmp");
    ressources->fondsNiveaux[3] = charger_bitmap_ou_erreur("assets/hall_24bit.bmp");
    ressources->fondsNiveaux[4] = charger_bitmap_ou_erreur("assets/duel_24bit.bmp");
    if (!ressources->fondsNiveaux[1] ||
        !ressources->fondsNiveaux[2] ||
        !ressources->fondsNiveaux[3] ||
        !ressources->fondsNiveaux[4]) {
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

    volDeMortOrig = charger_bitmap_ou_erreur("assets/vol_de_mort.bmp");
    if (!volDeMortOrig) {
        liberer_ressources_jeu(ressources);
        return 0;
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
        return 0;
    }

    ressources->buffer = create_bitmap(screen->w, screen->h);
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
    dessiner_panneau_menu(ressources->buffer, ressources->buffer->w / 4, ressources->buffer->h / 4);

    textout_centre_ex(ressources->buffer,
                      font,
                      "MENU DE DEPART",
                      ressources->buffer->w / 2,
                      ressources->buffer->h / 2 - ressources->buffer->h / 5,
                      makecol(255, 255, 255),
                      -1);
    textout_centre_ex(ressources->buffer,
                      font,
                      "Utilisez Haut/Bas puis Entree",
                      ressources->buffer->w / 2,
                      ressources->buffer->h / 2 - ressources->buffer->h / 6,
                      makecol(150, 150, 150),
                      -1);

    yBase = ressources->buffer->h / 2 - ressources->buffer->h / 13;
    pasVertical = ressources->buffer->h / 18;
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
                          ressources->buffer->w / 2,
                          ressources->buffer->h / 2 + ressources->buffer->h / 5,
                          makecol(120, 120, 120),
                          -1);
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
    dessiner_panneau_menu(ressources->buffer, ressources->buffer->w / 4, ressources->buffer->h / 4);

    textout_centre_ex(ressources->buffer,
                      font,
                      "PARAMETRES",
                      ressources->buffer->w / 2,
                      ressources->buffer->h / 2 - ressources->buffer->h / 5,
                      makecol(255, 255, 255),
                      -1);

    yBase = ressources->buffer->h / 2 - ressources->buffer->h / 10;
    pasVertical = ressources->buffer->h / 18;

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
        dessiner_option_menu(ressources->buffer, "Demo niveau 5", yBase + pasVertical * 5, selection == 5, 1);
        dessiner_option_menu(ressources->buffer, "Retour", yBase + pasVertical * 6, selection == 6, 1);
    } else {
        dessiner_option_menu(ressources->buffer, "Retour", yBase + pasVertical, selection == 1, 1);
    }

    textout_centre_ex(ressources->buffer,
                      font,
                      "Entree pour valider, ESC pour revenir",
                      ressources->buffer->w / 2,
                      ressources->buffer->h / 2 + ressources->buffer->h / 5,
                      makecol(140, 140, 140),
                      -1);

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
    dessiner_panneau_menu(ressources->buffer, ressources->buffer->w / 4, ressources->buffer->h / 4);

    textout_centre_ex(ressources->buffer, font, titre, ressources->buffer->w / 2, ressources->buffer->h / 2 - ressources->buffer->h / 6, makecol(255, 255, 255), -1);
    textout_centre_ex(ressources->buffer, font, ligne1, ressources->buffer->w / 2, ressources->buffer->h / 2 - ressources->buffer->h / 18, makecol(210, 210, 210), -1);
    textout_centre_ex(ressources->buffer, font, ligne2, ressources->buffer->w / 2, ressources->buffer->h / 2, makecol(210, 210, 210), -1);
    textout_centre_ex(ressources->buffer, font, ligne3, ressources->buffer->w / 2, ressources->buffer->h / 2 + ressources->buffer->h / 18, makecol(210, 210, 210), -1);
    textout_centre_ex(ressources->buffer,
                      font,
                      "ESC ou Retour arriere pour revenir au menu",
                      ressources->buffer->w / 2,
                      ressources->buffer->h / 2 + ressources->buffer->h / 7,
                      makecol(140, 140, 140),
                      -1);

    afficher_buffer_a_l_ecran(ressources->buffer);
}

void dessiner_saisie_pseudo(const RessourcesJeu *ressources, const char *pseudo) {
    int demiLargeur;
    int demiHauteur;
    int champDemiLargeur;
    int champHauteur;

    if (!ressources || !ressources->buffer) {
        return;
    }

    demiLargeur = ressources->buffer->w / 4;
    demiHauteur = ressources->buffer->h / 5;
    champDemiLargeur = ressources->buffer->w / 6;
    champHauteur = ressources->buffer->h / 18;

    dessiner_fond_scene(ressources->buffer, ressources);
    dessiner_panneau_menu(ressources->buffer, demiLargeur, demiHauteur);

    textout_centre_ex(ressources->buffer, font, "NOUVELLE PARTIE", ressources->buffer->w / 2, ressources->buffer->h / 2 - ressources->buffer->h / 7, makecol(255, 255, 255), -1);
    textout_centre_ex(ressources->buffer,
                      font,
                      "Entrez votre pseudo puis appuyez sur Entree",
                      ressources->buffer->w / 2,
                      ressources->buffer->h / 2 - ressources->buffer->h / 10,
                      makecol(200, 200, 200),
                      -1);

    rectfill(ressources->buffer,
             ressources->buffer->w / 2 - champDemiLargeur,
             ressources->buffer->h / 2 - champHauteur / 2,
             ressources->buffer->w / 2 + champDemiLargeur,
             ressources->buffer->h / 2 + champHauteur / 2,
             makecol(20, 20, 20));
    rect(ressources->buffer,
         ressources->buffer->w / 2 - champDemiLargeur,
         ressources->buffer->h / 2 - champHauteur / 2,
         ressources->buffer->w / 2 + champDemiLargeur,
         ressources->buffer->h / 2 + champHauteur / 2,
         makecol(255, 255, 255));
    textprintf_centre_ex(ressources->buffer,
                         font,
                         ressources->buffer->w / 2,
                         ressources->buffer->h / 2 - text_height(font) / 2,
                         makecol(255, 255, 255),
                         -1,
                         "%s_",
                         pseudo);
    textout_centre_ex(ressources->buffer,
                      font,
                      "ESC pour revenir au menu",
                      ressources->buffer->w / 2,
                      ressources->buffer->h / 2 + ressources->buffer->h / 10,
                      makecol(140, 140, 140),
                      -1);

    afficher_buffer_a_l_ecran(ressources->buffer);
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
                                   ressources->buffer->w / 2,
                                   ressources->buffer->h / 2,
                                   makecol(255, 255, 255),
                                   makecol(0, 0, 0),
                                   10);

    afficher_buffer_a_l_ecran(ressources->buffer);
}

void dessiner_jeu(const RessourcesJeu *ressources, const EtatJeu *etat) {
    if (!ressources || !etat || !ressources->buffer || !ressources->player) {
        return;
    }

    dessiner_scene_jeu(ressources->buffer, ressources, etat);
    dessiner_panneau_hud(ressources->buffer, etat);

    if (etat->perdu) {
        afficher_message_centre(ressources->buffer, ressources->buffer->w / 7, makecol(255, 0, 0), makecol(255, 0, 0), "PERDU");
    }
    if (etat->gagne) {
        if (etat->niveau < etat->niveauMaximum) {
            afficher_message_centre(ressources->buffer, ressources->buffer->w / 5, makecol(0, 255, 0), makecol(0, 255, 0), "GAGNE - ENTREE POUR NIVEAU SUIVANT");
        } else {
            afficher_message_centre(ressources->buffer, ressources->buffer->w / 5, makecol(0, 255, 0), makecol(0, 255, 0), "VICTOIRE FINALE - ENTREE POUR MENU");
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
    int i;

    if (!ressources) {
        return;
    }

    liberer_bitmap(&ressources->buffer);
    for (i = 1; i < NOMBRE_FONDS_NIVEAUX; i++) {
        liberer_bitmap(&ressources->fondsNiveaux[i]);
    }
    ressources->fondsNiveaux[0] = NULL;
    for (i = 0; i < BULLE_TAILLES_TOTAL; i++) {
        liberer_bitmap(&ressources->sprites[i]);
        liberer_bitmap(&ressources->spritesVifDor[i]);
        liberer_bitmap(&ressources->spritesVolDeMort[i]);
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
