#include "affichage.h"

static void initialiser_ressources_vides(RessourcesJeu *ressources) {
    int i;

    ressources->fond = NULL;
    ressources->player = NULL;
    ressources->buffer = NULL;
    for (i = 0; i < 4; i++) {
        ressources->sprites[i] = NULL;
    }
}

static void afficher_message_centre(BITMAP *buffer,
                                    int largeur,
                                    int couleur_bord,
                                    int couleur_texte,
                                    const char *message) {
    rectfill(buffer,
             SCREEN_W / 2 - largeur,
             SCREEN_H / 2 - 50,
             SCREEN_W / 2 + largeur,
             SCREEN_H / 2 + 50,
             makecol(0, 0, 0));

    rect(buffer,
         SCREEN_W / 2 - largeur,
         SCREEN_H / 2 - 50,
         SCREEN_W / 2 + largeur,
         SCREEN_H / 2 + 50,
         couleur_bord);

    textout_centre_ex(buffer,
                      font,
                      message,
                      SCREEN_W / 2,
                      SCREEN_H / 2 - 5,
                      couleur_texte,
                      -1);
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

int charger_ressources_jeu(RessourcesJeu *ressources,
                           const char *fond_path,
                           const char *player_path,
                           const char *bulle_path) {
    BITMAP *player_orig;
    BITMAP *bulle_orig;

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

    bulle_orig = charger_bitmap_ou_erreur(bulle_path);
    if (!bulle_orig) {
        liberer_ressources_jeu(ressources);
        return 0;
    }

    ressources->sprites[BULLE_TRES_GRANDE] = resize_bitmap(bulle_orig, 0.22f);
    ressources->sprites[BULLE_GRANDE] = resize_bitmap(bulle_orig, 0.17f);
    ressources->sprites[BULLE_MOYENNE] = resize_bitmap(bulle_orig, 0.12f);
    ressources->sprites[BULLE_PETITE] = resize_bitmap(bulle_orig, 0.08f);
    destroy_bitmap(bulle_orig);

    if (!ressources->sprites[BULLE_TRES_GRANDE] ||
        !ressources->sprites[BULLE_GRANDE] ||
        !ressources->sprites[BULLE_MOYENNE] ||
        !ressources->sprites[BULLE_PETITE]) {
        allegro_message("Erreur resize des bulles");
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

void dessiner_jeu(const RessourcesJeu *ressources, const EtatJeu *etat) {
    int i;

    if (!ressources || !etat || !ressources->buffer || !ressources->fond || !ressources->player) {
        return;
    }

    clear_to_color(ressources->buffer, makecol(0, 0, 0));
    stretch_blit(ressources->fond,
                 ressources->buffer,
                 0,
                 0,
                 ressources->fond->w,
                 ressources->fond->h,
                 0,
                 0,
                 SCREEN_W,
                 SCREEN_H);

    rectfill(ressources->buffer,
             0,
             etat->groundY,
             SCREEN_W,
             SCREEN_H,
             makecol(70, 120, 55));

    for (i = 0; i < MAX_BULLES; i++) {
        if (!etat->bulles[i].active || !etat->bulles[i].sprite) {
            continue;
        }

        masked_blit(etat->bulles[i].sprite,
                    ressources->buffer,
                    0,
                    0,
                    (int) etat->bulles[i].x,
                    (int) etat->bulles[i].y,
                    etat->bulles[i].sprite->w,
                    etat->bulles[i].sprite->h);
    }

    masked_blit(ressources->player,
                ressources->buffer,
                0,
                0,
                etat->x,
                etat->y,
                ressources->player->w,
                ressources->player->h);

    if (etat->projectileActive) {
        rectfill(ressources->buffer,
                 etat->projectileX,
                 etat->projectileY,
                 etat->projectileX + etat->projectileW,
                 etat->projectileY + etat->projectileH,
                 makecol(255, 230, 120));
    }

    if (etat->perdu) {
        afficher_message_centre(ressources->buffer, 180, makecol(255, 0, 0), makecol(255, 0, 0), "PERDU");
    }
    if (etat->gagne) {
        afficher_message_centre(ressources->buffer, 220, makecol(0, 255, 0), makecol(0, 255, 0), "GAGNE - PLUS DE BULLES");
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
    for (i = 0; i < 4; i++) {
        liberer_bitmap(&ressources->sprites[i]);
    }
    liberer_bitmap(&ressources->player);
    liberer_bitmap(&ressources->fond);
}

void fermer_affichage(void) {
    allegro_exit();
}
