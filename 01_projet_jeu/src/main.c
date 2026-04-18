#include "affichage.h"
#include "IHM.h"
#include "logique_jeu.h"
#include "sauvegarde.h"

#ifndef JEU_ASSETS_DIR
#define JEU_ASSETS_DIR "01_projet_jeu/assets"
#endif

#ifndef JEU_SAVE_FILE
#define JEU_SAVE_FILE "01_projet_jeu/sauvegardes/savegame.dat"
#endif

int main(void) {
    RessourcesJeu ressources;
    EtatJeu etat;
    ActionsIHM actions;

    if (!initialiser_affichage(1280, 720, 32)) {
        return 1;
    }

    if (!charger_ressources_jeu(&ressources,
                                JEU_ASSETS_DIR "/test2.bmp",
                                JEU_ASSETS_DIR "/hary2.bmp",
                                JEU_ASSETS_DIR "/vifdor3.bmp")) {
        fermer_affichage();
        return 1;
    }

    if (!initialiser_logique_jeu(&etat, ressources.sprites, ressources.player->w, ressources.player->h)) {
        liberer_ressources_jeu(&ressources);
        fermer_affichage();
        return 1;
    }

    if (!initialiser_sauvegarde()) {
        fermer_logique_jeu(&etat);
        liberer_ressources_jeu(&ressources);
        fermer_affichage();
        return 1;
    }

    actions.quitter = 0;
    while (!actions.quitter) {
        traiter_ihm(&etat, ressources.player, &actions);

        if (actions.sauvegarder) {
            sauvegarder_etat_jeu(&etat, JEU_SAVE_FILE);
        }

        if (actions.charger) {
            charger_etat_jeu(&etat, ressources.sprites, JEU_SAVE_FILE);
        }

        if (!actions.quitter) {
            mettre_a_jour_logique_jeu(&etat,
                                      ressources.sprites,
                                      ressources.player->w,
                                      ressources.player->h);
        }

        dessiner_jeu(&ressources, &etat);
        rest(16);
    }

    fermer_sauvegarde();
    fermer_logique_jeu(&etat);
    liberer_ressources_jeu(&ressources);
    fermer_affichage();

    return 0;
}
END_OF_MAIN();
