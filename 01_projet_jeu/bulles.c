#include "bulles.h"

int initialiser_bulles(BITMAP **sprite_bulle, const char *sprite_path) {
    if (!sprite_bulle) {
        return FAUX;
    }

    *sprite_bulle = charger_bitmap_ou_erreur(sprite_path);
    return (*sprite_bulle != NULL);
}

void fermer_bulles(BITMAP **sprite_bulle) {
    liberer_bitmap(sprite_bulle);
}
