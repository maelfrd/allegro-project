#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <allegro.h>

#include "src/config_jeu.h"

#define MAX_BULLES TAILLE_PSEUDO_MAX

typedef enum {
    BULLE_TRES_GRANDE,
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
static int plus_petite_dimension_test(int largeur, int hauteur) {
    if (largeur <= VALEUR_NULLE) {
        return hauteur;
    }
    if (hauteur <= VALEUR_NULLE) {
        return largeur;
    }
    return largeur < hauteur ? largeur : hauteur;
}

static int dimension_relative_test(int reference, int denominateur) {
    int valeur;

    if (reference <= VALEUR_NULLE || denominateur <= VALEUR_NULLE) {
        return VALEUR_NULLE;
    }

    valeur = reference / denominateur;
    if (valeur <= VALEUR_NULLE) {
        valeur = reference / reference;
    }

    return valeur;
}

BITMAP* resize_bitmap_hauteur_relative_test(BITMAP *src, int hauteurReference, int denominateur) {
    int newH;
    int newW;
    BITMAP *dest = NULL;

    if (!src || src->w <= VALEUR_NULLE || src->h <= VALEUR_NULLE) {
        return NULL;
    }

    newH = dimension_relative_test(hauteurReference, denominateur);
    newW = src->w * newH / src->h;

    if (newW <= VALEUR_NULLE) newW = VALEUR_UNITAIRE;
    if (newH <= VALEUR_NULLE) newH = VALEUR_UNITAIRE;

    dest = create_bitmap(newW, newH);
    if (!dest) return NULL;

    clear_to_color(dest, makecol(COULEUR_MAGENTA_R, COULEUR_MAGENTA_G, COULEUR_MAGENTA_B));
    stretch_blit(src, dest,
                 VALEUR_NULLE, VALEUR_NULLE, src->w, src->h,
                 VALEUR_NULLE, VALEUR_NULLE, newW, newH);

    return dest;
}

static int fichier_existe_simple(const char *chemin) {
    FILE *fichier = NULL;

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

static int resoudre_chemin_ressource(const char *chemin, char *destination, size_t taille) {
    static const char *prefixes[] = {
        "",
        "../",
        "../../",
        "assets/",
        "../assets/"
    };
    int i;

    if (!chemin || !destination || taille == VALEUR_NULLE || chemin[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE) {
        return FAUX;
    }

    for (i = INDEX_PREMIER; i < (int) (sizeof(prefixes) / sizeof(prefixes[CHAINE_DEBUT])); i++) {
        if (snprintf(destination, taille, "%s%s", prefixes[i], chemin) >= (int) taille) {
            continue;
        }
        if (fichier_existe_simple(destination)) {
            return VRAI;
        }
    }

    destination[CHAINE_DEBUT] = CARACTERE_FIN_CHAINE;
    return FAUX;
}

static int lire_entier_le(const unsigned char *octets) {
    return (int) octets[INDEX_PREMIER] |
           ((int) octets[INDEX_SUIVANT] << CODE_BITS_TOUCHE) |
           ((int) octets[VALEUR_DOUBLE] << DUREE_UNE_IMAGE_MS) |
           ((int) octets[VALEUR_TRIPLE] << VALEUR_VINGT_QUATRE);
}

static int determiner_taille_bitmap(const char *chemin, int *largeur, int *hauteur) {
    FILE *fichier = NULL;
    char cheminResolu[TAILLE_CHEMIN_RESSOURCE];
    unsigned char entete[DIVISEUR_VINGT_SIXIEME];
    int hauteurBitmap;

    if (!chemin || !largeur || !hauteur || chemin[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE) {
        return FAUX;
    }

    if (!resoudre_chemin_ressource(chemin, cheminResolu, sizeof(cheminResolu))) {
        return FAUX;
    }

    fichier = fopen(cheminResolu, "rb");
    if (!fichier) {
        return FAUX;
    }

    if (fread(entete, INDEX_SUIVANT, sizeof(entete), fichier) != sizeof(entete)) {
        fclose(fichier);
        return FAUX;
    }
    fclose(fichier);

    if (entete[INDEX_PREMIER] != 'B' || entete[INDEX_SUIVANT] != 'M') {
        return FAUX;
    }

    *largeur = lire_entier_le(&entete[VALEUR_DIX_HUIT]);
    hauteurBitmap = lire_entier_le(&entete[DIVISEUR_VINGT_DEUXIEME]);
    if (*largeur <= VALEUR_NULLE || hauteurBitmap == VALEUR_NULLE) {
        return FAUX;
    }

    *hauteur = hauteurBitmap < VALEUR_NULLE ? -hauteurBitmap : hauteurBitmap;
    return VRAI;
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
            b->gravite = (float) SCREEN_H / (float) DIVISEUR_GRAVITE_TRES_GRANDE;
            b->rebondSol = -(float) SCREEN_H / (float) DIVISEUR_REBOND_TRES_GRANDE;   // très haut
            b->attenuationX = ATTENUATION_TRES_GRANDE;
            break;

        case BULLE_GRANDE:
            b->gravite = (float) SCREEN_H / (float) DIVISEUR_GRAVITE_GRANDE;
            b->rebondSol = -(float) SCREEN_H / (float) DIVISEUR_REBOND_GRANDE;   // haut
            b->attenuationX = ATTENUATION_GRANDE;
            break;

        case BULLE_MOYENNE:
            b->gravite = (float) SCREEN_H / (float) DIVISEUR_GRAVITE_MOYENNE;
            b->rebondSol = -(float) SCREEN_H / (float) DIVISEUR_REBOND_MOYENNE;   // moyen
            b->attenuationX = ATTENUATION_MOYENNE;
            break;

        case BULLE_PETITE:
            b->gravite = (float) SCREEN_H / (float) DIVISEUR_GRAVITE_PETITE;
            b->rebondSol = -(float) SCREEN_H / (float) DIVISEUR_REBOND_PETITE;    // faible
            b->attenuationX = ATTENUATION_PETITE;
            break;
    }
}

/* --------------------------------------------------
   Trouver une case libre
-------------------------------------------------- */
int trouver_case_libre(Bulle bulles[]) {
    int i;
    for (i = INDEX_PREMIER; i < MAX_BULLES; i++) {
        if (!bulles[i].active) return i;
    }
    return INDEX_INVALIDE;
}

/* --------------------------------------------------
   Vitesse horizontale de base selon la taille
-------------------------------------------------- */
float vitesse_horizontale_fille(TailleBulle taille) {
    switch (taille) {
        case BULLE_TRES_GRANDE: return (float) SCREEN_W / (float) DIVISEUR_VITESSE_ENFANT_TRES_GRANDE;
        case BULLE_GRANDE:      return (float) SCREEN_W / (float) DIVISEUR_VITESSE_ENFANT_GRANDE;
        case BULLE_MOYENNE:     return (float) SCREEN_W / (float) DIVISEUR_VITESSE_ENFANT_MOYENNE;
        case BULLE_PETITE:      return (float) SCREEN_W / (float) DIVISEUR_VITESSE_ENFANT_PETITE;
    }
    return (float) SCREEN_W / (float) DIVISEUR_VITESSE_ENFANT_GRANDE;
}

/* --------------------------------------------------
   Ajouter une bulle
-------------------------------------------------- */
int ajouter_bulle(Bulle bulles[], BITMAP *sprites[],
                  float x, float y, float vx, float vy,
                  TailleBulle taille) {
    int idx = trouver_case_libre(bulles);

    if (idx == INDEX_INVALIDE) return INDEX_INVALIDE;

    configurer_bulle(&bulles[idx], taille, sprites);
    bulles[idx].x = x;
    bulles[idx].y = y;
    bulles[idx].vx = vx;
    bulles[idx].vy = vy;
    bulles[idx].active = VRAI;

    return idx;
}

/* --------------------------------------------------
   Séparer une bulle en deux bulles plus petites
   Si elle est déjà petite => destruction
-------------------------------------------------- */
void separer_bulle(Bulle bulles[], BITMAP *sprites[], int index) {
    Bulle source;
    TailleBulle nouvelleTaille;
    float centreX, centreY;
    float vxFille;
    BITMAP *spriteFille;

    if (index < VALEUR_NULLE || index >= MAX_BULLES) return;
    if (!bulles[index].active) return;

    source = bulles[index];

    if (source.taille == BULLE_PETITE) {
        bulles[index].active = FAUX;
        return;
    }

    nouvelleTaille = (TailleBulle)(source.taille + INDEX_SUIVANT);
    spriteFille = sprites[(int)nouvelleTaille];

    centreX = source.x + source.sprite->w / (float) DIVISEUR_CENTRE;
    centreY = source.y + source.sprite->h / (float) DIVISEUR_CENTRE;
    vxFille = vitesse_horizontale_fille(nouvelleTaille);

    bulles[index].active = FAUX;

    ajouter_bulle(bulles, sprites,
                  centreX - spriteFille->w / (float) DIVISEUR_CENTRE - (float) SCREEN_W / (float) DIVISEUR_CENT_VINGT_HUITIEME,
                  centreY - spriteFille->h / (float) DIVISEUR_CENTRE,
                  -vxFille,
                  ZERO_FLOTTANT,
                  nouvelleTaille);

    ajouter_bulle(bulles, sprites,
                  centreX - spriteFille->w / (float) DIVISEUR_CENTRE + (float) SCREEN_W / (float) DIVISEUR_CENT_VINGT_HUITIEME,
                  centreY - spriteFille->h / (float) DIVISEUR_CENTRE,
                  vxFille,
                  ZERO_FLOTTANT,
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
        if (b->vx > ZERO_FLOTTANT && b->vx < (float) SCREEN_W / (float) DIVISEUR_VITESSE_MIN_BULLE) b->vx = (float) SCREEN_W / (float) DIVISEUR_VITESSE_MIN_BULLE;
        if (b->vx < ZERO_FLOTTANT && b->vx > -(float) SCREEN_W / (float) DIVISEUR_VITESSE_MIN_BULLE) b->vx = -(float) SCREEN_W / (float) DIVISEUR_VITESSE_MIN_BULLE;
    }

    /* rebond mur gauche */
    if (b->x < ZERO_FLOTTANT) {
        b->x = ZERO_FLOTTANT;
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
    for (i = INDEX_PREMIER; i < MAX_BULLES; i++) {
        update_bulle(&bulles[i], groundY);
    }
}

/* --------------------------------------------------
   Dessin des bulles
-------------------------------------------------- */
void draw_bulles(Bulle bulles[], BITMAP *buffer) {
    int i;
    for (i = INDEX_PREMIER; i < MAX_BULLES; i++) {
        if (!bulles[i].active) continue;

        masked_blit(bulles[i].sprite, buffer,
                    VALEUR_NULLE, VALEUR_NULLE,
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
    for (i = INDEX_PREMIER; i < MAX_BULLES; i++) {
        if (!bulles[i].active) continue;

        if (collision_rect(projX, projY, projW, projH,
                           (int)bulles[i].x, (int)bulles[i].y,
                           bulles[i].sprite->w, bulles[i].sprite->h)) {
            return i;
        }
    }
    return INDEX_INVALIDE;
}

/* --------------------------------------------------
   Collision joueur / bulles
-------------------------------------------------- */
int check_player_collision(Bulle bulles[], int px, int py, int pw, int ph) {
    int i;
    for (i = INDEX_PREMIER; i < MAX_BULLES; i++) {
        if (!bulles[i].active) continue;

        if (collision_rect(px, py, pw, ph,
                           (int)bulles[i].x, (int)bulles[i].y,
                           bulles[i].sprite->w, bulles[i].sprite->h)) {
            return VRAI;
        }
    }
    return FAUX;
}

/* --------------------------------------------------
   Vérifier s'il reste des bulles
-------------------------------------------------- */
int reste_des_bulles(Bulle bulles[]) {
    int i;
    for (i = INDEX_PREMIER; i < MAX_BULLES; i++) {
        if (bulles[i].active) return VRAI;
    }
    return FAUX;
}

/* --------------------------------------------------
   Affichage perdu
-------------------------------------------------- */
void afficher_perdu(BITMAP *buffer) {
    rectfill(buffer,
             SCREEN_W / DIVISEUR_CENTRE - SCREEN_W / DIVISEUR_HUITIEME,
             SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_VINGT_DEUXIEME,
             SCREEN_W / DIVISEUR_CENTRE + SCREEN_W / DIVISEUR_HUITIEME,
             SCREEN_H / DIVISEUR_CENTRE + SCREEN_H / DIVISEUR_VINGT_DEUXIEME,
             makecol(COULEUR_NOIR_R, COULEUR_NOIR_G, COULEUR_NOIR_B));

    rect(buffer,
         SCREEN_W / DIVISEUR_CENTRE - SCREEN_W / DIVISEUR_HUITIEME,
         SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_VINGT_DEUXIEME,
         SCREEN_W / DIVISEUR_CENTRE + SCREEN_W / DIVISEUR_HUITIEME,
         SCREEN_H / DIVISEUR_CENTRE + SCREEN_H / DIVISEUR_VINGT_DEUXIEME,
         makecol(COULEUR_ROUGE_R, COULEUR_ROUGE_G, COULEUR_ROUGE_B));

    textout_centre_ex(buffer, font,
                      "PERDU",
                      SCREEN_W / DIVISEUR_CENTRE,
                      SCREEN_H / DIVISEUR_CENTRE - text_height(font) / DIVISEUR_CENTRE,
                      makecol(COULEUR_ROUGE_R, COULEUR_ROUGE_G, COULEUR_ROUGE_B),
                      COULEUR_TRANSPARENTE_TEXTE);
}

/* --------------------------------------------------
   Affichage gagne
-------------------------------------------------- */
void afficher_gagne(BITMAP *buffer) {
    rectfill(buffer,
             SCREEN_W / DIVISEUR_CENTRE - SCREEN_W / DIVISEUR_SEPTIEME,
             SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_VINGT_DEUXIEME,
             SCREEN_W / DIVISEUR_CENTRE + SCREEN_W / DIVISEUR_SEPTIEME,
             SCREEN_H / DIVISEUR_CENTRE + SCREEN_H / DIVISEUR_VINGT_DEUXIEME,
             makecol(COULEUR_NOIR_R, COULEUR_NOIR_G, COULEUR_NOIR_B));

    rect(buffer,
         SCREEN_W / DIVISEUR_CENTRE - SCREEN_W / DIVISEUR_SEPTIEME,
         SCREEN_H / DIVISEUR_CENTRE - SCREEN_H / DIVISEUR_VINGT_DEUXIEME,
         SCREEN_W / DIVISEUR_CENTRE + SCREEN_W / DIVISEUR_SEPTIEME,
         SCREEN_H / DIVISEUR_CENTRE + SCREEN_H / DIVISEUR_VINGT_DEUXIEME,
         makecol(COULEUR_VERT_R, COULEUR_VERT_G, COULEUR_VERT_B));

    textout_centre_ex(buffer, font,
                      "GAGNE - PLUS DE BULLES",
                      SCREEN_W / DIVISEUR_CENTRE,
                      SCREEN_H / DIVISEUR_CENTRE - text_height(font) / DIVISEUR_CENTRE,
                      makecol(COULEUR_VERT_R, COULEUR_VERT_G, COULEUR_VERT_B),
                      COULEUR_TRANSPARENTE_TEXTE);
}

int main() {
    BITMAP *fond = NULL;
    BITMAP *player_orig = NULL;
    BITMAP *player = NULL;
    BITMAP *bulle_orig = NULL;
    BITMAP *sprites[VALEUR_QUADRUPLE] = {NULL, NULL, NULL, NULL};
    BITMAP *buffer = NULL;

    int groundY, leftLimit, rightLimit;
    int x, y, speed;

    Bulle bulles[MAX_BULLES];
    int i;

    int projectileActive = FAUX;
    int projectileX = VALEUR_NULLE;
    int projectileY = VALEUR_NULLE;
    int projectileW;
    int projectileH;
    int projectileSpeed;
    int oldUpState = FAUX;

    int perdu = FAUX;
    int gagne = FAUX;
    int idxTouchee = INDEX_INVALIDE;
    int largeurReference = VALEUR_NULLE;
    int hauteurReference = VALEUR_NULLE;
    int largeurFenetre;
    int hauteurFenetre;
    int referenceSprites;
    char fondPath[TAILLE_CHEMIN_RESSOURCE];

    srand((unsigned int)time(NULL));

    /* --------------------------------------------------
       Initialisation
    -------------------------------------------------- */
    allegro_init();
    install_keyboard();
    set_color_depth(PROFONDEUR_COULEUR_JEU);
    resoudre_chemin_ressource("test2.bmp", fondPath, sizeof(fondPath));
    if (get_desktop_resolution(&largeurReference, &hauteurReference) != VALEUR_NULLE ||
        largeurReference <= VALEUR_NULLE ||
        hauteurReference <= VALEUR_NULLE) {
        if (!determiner_taille_bitmap(fondPath, &largeurReference, &hauteurReference)) {
            allegro_message("Impossible de calculer une taille de fenetre relative");
            return RETOUR_PROGRAMME_ERREUR;
        }
    }
    largeurFenetre = largeurReference * JEU_FENETRE_LARGEUR_NUMERATEUR / JEU_FENETRE_RATIO_DENOMINATEUR;
    hauteurFenetre = hauteurReference * JEU_FENETRE_HAUTEUR_NUMERATEUR / JEU_FENETRE_RATIO_DENOMINATEUR;
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, largeurFenetre, hauteurFenetre, VALEUR_NULLE, VALEUR_NULLE) != VALEUR_NULLE) {
        allegro_message("Erreur mode graphique");
        return RETOUR_PROGRAMME_ERREUR;
    }

    /* --------------------------------------------------
       Chargement fond
    -------------------------------------------------- */
    fond = load_bitmap(fondPath, NULL);
    if (!fond) {
        allegro_message("Impossible de charger %s", fondPath);
        return RETOUR_PROGRAMME_ERREUR;
    }

    /* --------------------------------------------------
       Chargement joueur
    -------------------------------------------------- */
    player_orig = load_bitmap("hary2.bmp", NULL);
    if (!player_orig) {
        destroy_bitmap(fond);
        allegro_message("Impossible de charger hary.bmp");
        return RETOUR_PROGRAMME_ERREUR;
    }

    player = resize_bitmap_hauteur_relative_test(player_orig, SCREEN_H, DIVISEUR_REDIMENSION_JOUEUR);
    destroy_bitmap(player_orig);
    player_orig = NULL;

    if (!player) {
        destroy_bitmap(fond);
        allegro_message("Erreur resize personnage");
        return RETOUR_PROGRAMME_ERREUR;
    }

    /* --------------------------------------------------
       Chargement sprite bulle / vif d'or
    -------------------------------------------------- */
    bulle_orig = load_bitmap("vifdor3.bmp", NULL);
    if (!bulle_orig) {
        destroy_bitmap(fond);
        destroy_bitmap(player);
        allegro_message("Impossible de charger vifdor2.bmp");
        return RETOUR_PROGRAMME_ERREUR;
    }

    /* tailles */
    referenceSprites = plus_petite_dimension_test(SCREEN_W, SCREEN_H);
    sprites[BULLE_TRES_GRANDE] = resize_bitmap_hauteur_relative_test(bulle_orig, referenceSprites, DIVISEUR_REDIMENSION_BULLE_TRES_GRANDE);
    sprites[BULLE_GRANDE]      = resize_bitmap_hauteur_relative_test(bulle_orig, referenceSprites, DIVISEUR_REDIMENSION_BULLE_GRANDE);
    sprites[BULLE_MOYENNE]     = resize_bitmap_hauteur_relative_test(bulle_orig, referenceSprites, DIVISEUR_REDIMENSION_BULLE_MOYENNE);
    sprites[BULLE_PETITE]      = resize_bitmap_hauteur_relative_test(bulle_orig, referenceSprites, DIVISEUR_REDIMENSION_BULLE_PETITE);

    destroy_bitmap(bulle_orig);
    bulle_orig = NULL;

    for (i = INDEX_PREMIER; i < VALEUR_QUADRUPLE; i++) {
        if (!sprites[i]) {
            int j;
            for (j = INDEX_PREMIER; j < VALEUR_QUADRUPLE; j++) {
                if (sprites[j]) destroy_bitmap(sprites[j]);
            }
            destroy_bitmap(fond);
            destroy_bitmap(player);
            allegro_message("Erreur resize des bulles");
            return RETOUR_PROGRAMME_ERREUR;
        }
    }

    /* --------------------------------------------------
       Buffer
    -------------------------------------------------- */
    buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        for (i = INDEX_PREMIER; i < VALEUR_QUADRUPLE; i++) destroy_bitmap(sprites[i]);
        destroy_bitmap(player);
        destroy_bitmap(fond);
        allegro_message("Impossible de creer le buffer");
        return RETOUR_PROGRAMME_ERREUR;
    }

    /* --------------------------------------------------
       Terrain
    -------------------------------------------------- */
    groundY = SCREEN_H - SCREEN_H / DIVISEUR_HAUTEUR_SOL;
    leftLimit = VALEUR_NULLE;
    rightLimit = SCREEN_W;

    /* --------------------------------------------------
       Joueur
    -------------------------------------------------- */
    x = SCREEN_W / DIVISEUR_CENTRE - player->w / DIVISEUR_CENTRE;
    y = groundY - player->h;
    speed = SCREEN_W / DIVISEUR_VITESSE_JOUEUR;
    if (speed < VALEUR_UNITAIRE) {
        speed = VALEUR_UNITAIRE;
    }
    projectileW = SCREEN_W / DIVISEUR_LARGEUR_PROJECTILE;
    if (projectileW < VALEUR_UNITAIRE) {
        projectileW = VALEUR_UNITAIRE;
    }
    projectileH = SCREEN_H / DIVISEUR_HAUTEUR_PROJECTILE;
    if (projectileH < VALEUR_UNITAIRE) {
        projectileH = VALEUR_UNITAIRE;
    }
    projectileSpeed = SCREEN_H / DIVISEUR_VITESSE_PROJECTILE;
    if (projectileSpeed < VALEUR_UNITAIRE) {
        projectileSpeed = VALEUR_UNITAIRE;
    }

    /* --------------------------------------------------
       Initialisation bulles
    -------------------------------------------------- */
    for (i = INDEX_PREMIER; i < MAX_BULLES; i++) {
        bulles[i].active = FAUX;
        bulles[i].x = ZERO_FLOTTANT;
        bulles[i].y = ZERO_FLOTTANT;
        bulles[i].vx = ZERO_FLOTTANT;
        bulles[i].vy = ZERO_FLOTTANT;
        bulles[i].taille = BULLE_PETITE;
        bulles[i].gravite = ZERO_FLOTTANT;
        bulles[i].rebondSol = ZERO_FLOTTANT;
        bulles[i].attenuationX = (float) VALEUR_UNITAIRE;
        bulles[i].sprite = NULL;
    }

    /* une bulle de départ */
    ajouter_bulle(bulles, sprites,
                  SCREEN_W * POSITION_X_NIVEAU_1,
                  SCREEN_H * POSITION_Y_NIVEAU_1,
                  -(float) SCREEN_W / (float) DIVISEUR_VITESSE_NIVEAU_1,
                  ZERO_FLOTTANT,
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
                projectileActive = VRAI;
                projectileX = x + player->w / DIVISEUR_CENTRE - projectileW / DIVISEUR_CENTRE;
                projectileY = y;
            }
            oldUpState = key[KEY_UP];

            /* projectile */
            if (projectileActive) {
                projectileY -= projectileSpeed;

                if (projectileY + projectileH < VALEUR_NULLE) {
                    projectileActive = FAUX;
                }
            }

            /* update bulles */
            update_bulles(bulles, groundY);

            /* collision projectile / bulle */
            if (projectileActive) {
                idxTouchee = collision_projectile_bulles(bulles,
                                                        projectileX, projectileY,
                                                        projectileW, projectileH);
                if (idxTouchee != INDEX_INVALIDE) {
                    projectileActive = FAUX;
                    separer_bulle(bulles, sprites, idxTouchee);
                }
            }

            /* collision joueur / bulles */
            if (check_player_collision(bulles, x, y, player->w, player->h)) {
                perdu = VRAI;
            }

            /* victoire */
            if (!reste_des_bulles(bulles)) {
                gagne = VRAI;
            }
        }

        /* --------------------------------------------------
           Dessin
        -------------------------------------------------- */
        clear_to_color(buffer, makecol(COULEUR_NOIR_R, COULEUR_NOIR_G, COULEUR_NOIR_B));

        stretch_blit(fond, buffer,
                     VALEUR_NULLE, VALEUR_NULLE, fond->w, fond->h,
                     VALEUR_NULLE, VALEUR_NULLE, SCREEN_W, SCREEN_H);

        /* sol */
        rectfill(buffer,
                 VALEUR_NULLE, groundY,
                 SCREEN_W, SCREEN_H,
                 makecol(COULEUR_SOL_R, COULEUR_SOL_G, COULEUR_SOL_B));

        /* bulles */
        draw_bulles(bulles, buffer);

        /* joueur */
        masked_blit(player, buffer,
                    VALEUR_NULLE, VALEUR_NULLE,
                    x, y,
                    player->w, player->h);

        /* projectile */
        if (projectileActive) {
            rectfill(buffer,
                     projectileX,
                     projectileY,
                     projectileX + projectileW,
                     projectileY + projectileH,
                     makecol(COULEUR_HUD_AURA_ACTIVE_R, COULEUR_OPTION_SELECTION_FOND_R, COULEUR_HUD_AURA_ACTIVE_B));
        }

        /* messages */
        if (perdu) afficher_perdu(buffer);
        if (gagne) afficher_gagne(buffer);

        blit(buffer, screen, VALEUR_NULLE, VALEUR_NULLE, VALEUR_NULLE, VALEUR_NULLE, SCREEN_W, SCREEN_H);
        rest(DUREE_UNE_IMAGE_MS);
    }

    /* --------------------------------------------------
       Nettoyage
    -------------------------------------------------- */
    destroy_bitmap(buffer);

    for (i = INDEX_PREMIER; i < VALEUR_QUADRUPLE; i++) {
        destroy_bitmap(sprites[i]);
    }

    destroy_bitmap(player);
    destroy_bitmap(fond);

    return RETOUR_PROGRAMME_OK;
}
END_OF_MAIN();
