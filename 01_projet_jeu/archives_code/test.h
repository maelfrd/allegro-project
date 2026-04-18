#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <allegro.h>

#define MAX_BULLES 32

typedef enum {
    BULLE_TRES_GRANDE = 0,
    BULLE_GRANDE,
    BULLE_MOYENNE,
    BULLE_PETITE
} TailleBulle;

typedef struct {
    float x, y;
    float vx, vy;

    int active;
    TailleBulle taille;

    float gravite;
    float rebondSol;     // vitesse verticale imposée au sol
    float attenuationX;  // atténuation horizontale au rebond

    BITMAP *sprite;
} Bulle;

/* --------------------------------------------------
   Redimensionnement bitmap
-------------------------------------------------- */
BITMAP* resize_bitmap(BITMAP *src, float scale) {
    int newW = (int)(src->w * scale);
    int newH = (int)(src->h * scale);
    BITMAP *dest = NULL;

    if (newW <= 0) newW = 1;
    if (newH <= 0) newH = 1;

    dest = create_bitmap(newW, newH);
    if (!dest) return NULL;

    clear_to_color(dest, makecol(255, 0, 255));
    stretch_blit(src, dest,
                 0, 0, src->w, src->h,
                 0, 0, newW, newH);

    return dest;
}

/* --------------------------------------------------
   Collision rectangle / rectangle
-------------------------------------------------- */
int collision_rect(int x1, int y1, int w1, int h1,
                   int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}

/* --------------------------------------------------
   Paramètres selon la taille
   => ici tu règles la hauteur des rebonds
-------------------------------------------------- */
void configurer_bulle(Bulle *b, TailleBulle taille, BITMAP *sprites[]) {
    b->taille = taille;
    b->sprite = sprites[(int)taille];

    switch (taille) {
        case BULLE_TRES_GRANDE:
            b->gravite = 0.28f;
            b->rebondSol = -16.0f;   // très haut
            b->attenuationX = 0.995f;
            break;

        case BULLE_GRANDE:
            b->gravite = 0.26f;
            b->rebondSol = -13.0f;   // haut
            b->attenuationX = 0.992f;
            break;

        case BULLE_MOYENNE:
            b->gravite = 0.24f;
            b->rebondSol = -10.0f;   // moyen
            b->attenuationX = 0.989f;
            break;

        case BULLE_PETITE:
            b->gravite = 0.22f;
            b->rebondSol = -7.0f;    // faible
            b->attenuationX = 0.985f;
            break;
    }
}

/* --------------------------------------------------
   Trouver une case libre
-------------------------------------------------- */
int trouver_case_libre(Bulle bulles[]) {
    int i;
    for (i = 0; i < MAX_BULLES; i++) {
        if (!bulles[i].active) return i;
    }
    return -1;
}

/* --------------------------------------------------
   Vitesse horizontale de base selon la taille
-------------------------------------------------- */
float vitesse_horizontale_fille(TailleBulle taille) {
    switch (taille) {
        case BULLE_TRES_GRANDE: return 1.8f;
        case BULLE_GRANDE:      return 2.4f;
        case BULLE_MOYENNE:     return 3.0f;
        case BULLE_PETITE:      return 3.8f;
    }
    return 3.0f;
}

/* --------------------------------------------------
   Ajouter une bulle
-------------------------------------------------- */
int ajouter_bulle(Bulle bulles[], BITMAP *sprites[],
                  float x, float y, float vx, float vy,
                  TailleBulle taille) {
    int idx = trouver_case_libre(bulles);

    if (idx == -1) return -1;

    configurer_bulle(&bulles[idx], taille, sprites);
    bulles[idx].x = x;
    bulles[idx].y = y;
    bulles[idx].vx = vx;
    bulles[idx].vy = vy;
    bulles[idx].active = 1;

    return idx;
}

/* --------------------------------------------------
   Séparer une bulle en 2 bulles plus petites
   Si elle est déjà petite => destruction
-------------------------------------------------- */
void separer_bulle(Bulle bulles[], BITMAP *sprites[], int index) {
    Bulle source;
    TailleBulle nouvelleTaille;
    float centreX, centreY;
    float vxFille;
    BITMAP *spriteFille;

    if (index < 0 || index >= MAX_BULLES) return;
    if (!bulles[index].active) return;

    source = bulles[index];

    if (source.taille == BULLE_PETITE) {
        bulles[index].active = 0;
        return;
    }

    nouvelleTaille = (TailleBulle)(source.taille + 1);
    spriteFille = sprites[(int)nouvelleTaille];

    centreX = source.x + source.sprite->w / 2.0f;
    centreY = source.y + source.sprite->h / 2.0f;
    vxFille = vitesse_horizontale_fille(nouvelleTaille);

    bulles[index].active = 0;

    ajouter_bulle(bulles, sprites,
                  centreX - spriteFille->w / 2.0f - 8,
                  centreY - spriteFille->h / 2.0f,
                  -vxFille,
                  0.0f,
                  nouvelleTaille);

    ajouter_bulle(bulles, sprites,
                  centreX - spriteFille->w / 2.0f + 8,
                  centreY - spriteFille->h / 2.0f,
                  vxFille,
                  0.0f,
                  nouvelleTaille);
}

/* --------------------------------------------------
   Mise à jour d'une bulle
   Le rebond vertical dépend de la taille
-------------------------------------------------- */
void update_bulle(Bulle *b, int groundY) {
    if (!b->active) return;

    b->vy += b->gravite;
    b->x += b->vx;
    b->y += b->vy;

    /* rebond au sol */
    if (b->y + b->sprite->h >= groundY) {
        b->y = groundY - b->sprite->h;

        /* hauteur de rebond fixée par l'étage/taille */
        b->vy = b->rebondSol;

        /* légère atténuation horizontale */
        b->vx *= b->attenuationX;

        /* vitesse horizontale minimum pour éviter qu'elle s'arrête */
        if (b->vx > 0.0f && b->vx < 1.5f) b->vx = 1.5f;
        if (b->vx < 0.0f && b->vx > -1.5f) b->vx = -1.5f;
    }

    /* rebond mur gauche */
    if (b->x < 0) {
        b->x = 0;
        b->vx = -b->vx;
    }

    /* rebond mur droit */
    if (b->x + b->sprite->w > SCREEN_W) {
        b->x = SCREEN_W - b->sprite->w;
        b->vx = -b->vx;
    }
}

/* --------------------------------------------------
   Mise à jour de toutes les bulles
-------------------------------------------------- */
void update_bulles(Bulle bulles[], int groundY) {
    int i;
    for (i = 0; i < MAX_BULLES; i++) {
        update_bulle(&bulles[i], groundY);
    }
}

/* --------------------------------------------------
   Dessin des bulles
-------------------------------------------------- */
void draw_bulles(Bulle bulles[], BITMAP *buffer) {
    int i;
    for (i = 0; i < MAX_BULLES; i++) {
        if (!bulles[i].active) continue;

        masked_blit(bulles[i].sprite, buffer,
                    0, 0,
                    (int)bulles[i].x, (int)bulles[i].y,
                    bulles[i].sprite->w, bulles[i].sprite->h);
    }
}

/* --------------------------------------------------
   Collision projectile / bulle
-------------------------------------------------- */
int collision_projectile_bulles(Bulle bulles[],
                                int projX, int projY, int projW, int projH) {
    int i;
    for (i = 0; i < MAX_BULLES; i++) {
        if (!bulles[i].active) continue;

        if (collision_rect(projX, projY, projW, projH,
                           (int)bulles[i].x, (int)bulles[i].y,
                           bulles[i].sprite->w, bulles[i].sprite->h)) {
            return i;
        }
    }
    return -1;
}

/* --------------------------------------------------
   Collision joueur / bulles
-------------------------------------------------- */
int check_player_collision(Bulle bulles[], int px, int py, int pw, int ph) {
    int i;
    for (i = 0; i < MAX_BULLES; i++) {
        if (!bulles[i].active) continue;

        if (collision_rect(px, py, pw, ph,
                           (int)bulles[i].x, (int)bulles[i].y,
                           bulles[i].sprite->w, bulles[i].sprite->h)) {
            return 1;
        }
    }
    return 0;
}

/* --------------------------------------------------
   Vérifier s'il reste des bulles
-------------------------------------------------- */
int reste_des_bulles(Bulle bulles[]) {
    int i;
    for (i = 0; i < MAX_BULLES; i++) {
        if (bulles[i].active) return 1;
    }
    return 0;
}

/* --------------------------------------------------
   Affichage perdu
-------------------------------------------------- */
void afficher_perdu(BITMAP *buffer) {
    rectfill(buffer,
             SCREEN_W / 2 - 180, SCREEN_H / 2 - 50,
             SCREEN_W / 2 + 180, SCREEN_H / 2 + 50,
             makecol(0, 0, 0));

    rect(buffer,
         SCREEN_W / 2 - 180, SCREEN_H / 2 - 50,
         SCREEN_W / 2 + 180, SCREEN_H / 2 + 50,
         makecol(255, 0, 0));

    textout_centre_ex(buffer, font,
                      "PERDU",
                      SCREEN_W / 2,
                      SCREEN_H / 2 - 5,
                      makecol(255, 0, 0),
                      -1);
}

/* --------------------------------------------------
   Affichage gagne
-------------------------------------------------- */
void afficher_gagne(BITMAP *buffer) {
    rectfill(buffer,
             SCREEN_W / 2 - 220, SCREEN_H / 2 - 50,
             SCREEN_W / 2 + 220, SCREEN_H / 2 + 50,
             makecol(0, 0, 0));

    rect(buffer,
         SCREEN_W / 2 - 220, SCREEN_H / 2 - 50,
         SCREEN_W / 2 + 220, SCREEN_H / 2 + 50,
         makecol(0, 255, 0));

    textout_centre_ex(buffer, font,
                      "GAGNE - PLUS DE BULLES",
                      SCREEN_W / 2,
                      SCREEN_H / 2 - 5,
                      makecol(0, 255, 0),
                      -1);
}

int main() {
    BITMAP *fond = NULL;
    BITMAP *player_orig = NULL;
    BITMAP *player = NULL;
    BITMAP *bulle_orig = NULL;
    BITMAP *sprites[4] = {NULL, NULL, NULL, NULL};
    BITMAP *buffer = NULL;

    int groundY, leftLimit, rightLimit;
    int x, y, speed;

    Bulle bulles[MAX_BULLES];
    int i;

    int projectileActive = 0;
    int projectileX = 0;
    int projectileY = 0;
    int projectileW = 8;
    int projectileH = 20;
    int projectileSpeed = 12;
    int oldUpState = 0;

    int perdu = 0;
    int gagne = 0;
    int idxTouchee = -1;

    srand((unsigned int)time(NULL));

    /* --------------------------------------------------
       Initialisation
    -------------------------------------------------- */
    allegro_init();
    install_keyboard();
    set_color_depth(32);

    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1280, 720, 0, 0) != 0) {
        allegro_message("Erreur mode graphique");
        return 1;
    }

    /* --------------------------------------------------
       Chargement fond
    -------------------------------------------------- */
    fond = load_bitmap("test2.bmp", NULL);
    if (!fond) {
        allegro_message("Impossible de charger test2.bmp");
        return 1;
    }

    /* --------------------------------------------------
       Chargement joueur
    -------------------------------------------------- */
    player_orig = load_bitmap("hary2.bmp", NULL);
    if (!player_orig) {
        destroy_bitmap(fond);
        allegro_message("Impossible de charger hary.bmp");
        return 1;
    }

    player = resize_bitmap(player_orig, 0.25f);
    destroy_bitmap(player_orig);
    player_orig = NULL;

    if (!player) {
        destroy_bitmap(fond);
        allegro_message("Erreur resize personnage");
        return 1;
    }

    /* --------------------------------------------------
       Chargement sprite bulle / vif d'or
    -------------------------------------------------- */
    bulle_orig = load_bitmap("vifdor3.bmp", NULL);
    if (!bulle_orig) {
        destroy_bitmap(fond);
        destroy_bitmap(player);
        allegro_message("Impossible de charger vifdor2.bmp");
        return 1;
    }

    /* tailles */
    sprites[BULLE_TRES_GRANDE] = resize_bitmap(bulle_orig, 0.22f);
    sprites[BULLE_GRANDE]      = resize_bitmap(bulle_orig, 0.17f);
    sprites[BULLE_MOYENNE]     = resize_bitmap(bulle_orig, 0.12f);
    sprites[BULLE_PETITE]      = resize_bitmap(bulle_orig, 0.08f);

    destroy_bitmap(bulle_orig);
    bulle_orig = NULL;

    for (i = 0; i < 4; i++) {
        if (!sprites[i]) {
            int j;
            for (j = 0; j < 4; j++) {
                if (sprites[j]) destroy_bitmap(sprites[j]);
            }
            destroy_bitmap(fond);
            destroy_bitmap(player);
            allegro_message("Erreur resize des bulles");
            return 1;
        }
    }

    /* --------------------------------------------------
       Buffer
    -------------------------------------------------- */
    buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        for (i = 0; i < 4; i++) destroy_bitmap(sprites[i]);
        destroy_bitmap(player);
        destroy_bitmap(fond);
        allegro_message("Impossible de creer le buffer");
        return 1;
    }

    /* --------------------------------------------------
       Terrain
    -------------------------------------------------- */
    groundY = SCREEN_H - 120;
    leftLimit = 0;
    rightLimit = SCREEN_W;

    /* --------------------------------------------------
       Joueur
    -------------------------------------------------- */
    x = SCREEN_W / 2 - player->w / 2;
    y = groundY - player->h;
    speed = 6;

    /* --------------------------------------------------
       Initialisation bulles
    -------------------------------------------------- */
    for (i = 0; i < MAX_BULLES; i++) {
        bulles[i].active = 0;
        bulles[i].x = 0;
        bulles[i].y = 0;
        bulles[i].vx = 0;
        bulles[i].vy = 0;
        bulles[i].taille = BULLE_PETITE;
        bulles[i].gravite = 0;
        bulles[i].rebondSol = 0;
        bulles[i].attenuationX = 1.0f;
        bulles[i].sprite = NULL;
    }

    /* une bulle de départ */
    ajouter_bulle(bulles, sprites,
                  SCREEN_W - 320.0f,
                  80.0f,
                  -3.2f,
                  0.0f,
                  BULLE_TRES_GRANDE);

    /* --------------------------------------------------
       Boucle principale
    -------------------------------------------------- */
    while (!key[KEY_ESC]) {

        if (!perdu && !gagne) {
            /* déplacement joueur */
            if (key[KEY_LEFT])  x -= speed;
            if (key[KEY_RIGHT]) x += speed;

            if (x < leftLimit) x = leftLimit;
            if (x + player->w > rightLimit) x = rightLimit - player->w;

            y = groundY - player->h;

            /* tir flèche haut */
            if (key[KEY_UP] && !oldUpState && !projectileActive) {
                projectileActive = 1;
                projectileX = x + player->w / 2 - projectileW / 2;
                projectileY = y;
            }
            oldUpState = key[KEY_UP];

            /* projectile */
            if (projectileActive) {
                projectileY -= projectileSpeed;

                if (projectileY + projectileH < 0) {
                    projectileActive = 0;
                }
            }

            /* update bulles */
            update_bulles(bulles, groundY);

            /* collision projectile / bulle */
            if (projectileActive) {
                idxTouchee = collision_projectile_bulles(bulles,
                                                        projectileX, projectileY,
                                                        projectileW, projectileH);
                if (idxTouchee != -1) {
                    projectileActive = 0;
                    separer_bulle(bulles, sprites, idxTouchee);
                }
            }

            /* collision joueur / bulles */
            if (check_player_collision(bulles, x, y, player->w, player->h)) {
                perdu = 1;
            }

            /* victoire */
            if (!reste_des_bulles(bulles)) {
                gagne = 1;
            }
        }

        /* --------------------------------------------------
           Dessin
        -------------------------------------------------- */
        clear_to_color(buffer, makecol(0, 0, 0));

        stretch_blit(fond, buffer,
                     0, 0, fond->w, fond->h,
                     0, 0, SCREEN_W, SCREEN_H);

        /* sol */
        rectfill(buffer,
                 0, groundY,
                 SCREEN_W, SCREEN_H,
                 makecol(70, 120, 55));

        /* bulles */
        draw_bulles(bulles, buffer);

        /* joueur */
        masked_blit(player, buffer,
                    0, 0,
                    x, y,
                    player->w, player->h);

        /* projectile */
        if (projectileActive) {
            rectfill(buffer,
                     projectileX,
                     projectileY,
                     projectileX + projectileW,
                     projectileY + projectileH,
                     makecol(255, 230, 120));
        }

        /* messages */
        if (perdu) afficher_perdu(buffer);
        if (gagne) afficher_gagne(buffer);

        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
        rest(16);
    }

    /* --------------------------------------------------
       Nettoyage
    -------------------------------------------------- */
    destroy_bitmap(buffer);

    for (i = 0; i < 4; i++) {
        destroy_bitmap(sprites[i]);
    }

    destroy_bitmap(player);
    destroy_bitmap(fond);

    return 0;
}
END_OF_MAIN();