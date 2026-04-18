#include "affichage.h"
#include "IHM.h"
#include "logique_jeu.h"
#include "sauvegarde.h"

#include <stdio.h>

typedef enum {
    ECRAN_MENU = 0,
    ECRAN_SAISIE_PSEUDO,
    ECRAN_DECOMPTE,
    ECRAN_JEU,
    ECRAN_PARAMETRES,
    ECRAN_REGLES
} EcranActif;

static void construire_configuration_jeu(ConfigurationJeu *configuration, const RessourcesJeu *ressources) {
    int i;

    configuration->largeurFenetre = SCREEN_W;
    configuration->hauteurFenetre = SCREEN_H;
    configuration->groundY = SCREEN_H - SCREEN_H / 6;
    configuration->leftLimit = 0;
    configuration->rightLimit = SCREEN_W;
    configuration->vitesseJoueur = SCREEN_W / 213;
    if (configuration->vitesseJoueur < 4) {
        configuration->vitesseJoueur = 4;
    }
    configuration->joueurLargeur = ressources->player->w;
    configuration->joueurHauteur = ressources->player->h;
    configuration->projectileLargeur = SCREEN_W / 160;
    if (configuration->projectileLargeur < 6) {
        configuration->projectileLargeur = 6;
    }
    configuration->projectileHauteur = SCREEN_H / 36;
    if (configuration->projectileHauteur < 16) {
        configuration->projectileHauteur = 16;
    }
    configuration->projectileVitesse = SCREEN_H / 60;
    if (configuration->projectileVitesse < 10) {
        configuration->projectileVitesse = 10;
    }

    for (i = 0; i < BULLE_TAILLES_TOTAL; i++) {
        configuration->largeurBulles[i] = ressources->sprites[i]->w;
        configuration->hauteurBulles[i] = ressources->sprites[i]->h;
    }
}

static int fichier_existe(const char *chemin) {
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

int main(void) {
    RessourcesJeu ressources;
    ConfigurationJeu configuration;
    EtatJeu etat;
    ActionsIHM actions;
    CommandesJeu commandes;
    EcranActif ecran;
    int repriseDisponible;
    int modeDemonstrationActif;
    int valeurDecompte;
    int tempsDecompteMs;
    char pseudoJoueur[TAILLE_PSEUDO_MAX];

    if (!initialiser_affichage(1280, 720, 32)) {
        return 1;
    }

    if (!charger_ressources_jeu(&ressources,
                                "assets/test2.bmp",
                                "assets/hary2.bmp",
                                "assets/vifdor3.bmp")) {
        fermer_affichage();
        return 1;
    }

    construire_configuration_jeu(&configuration, &ressources);

    if (!initialiser_logique_jeu(&etat, &configuration)) {
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

    initialiser_actions_ihm(&actions);
    initialiser_commandes_jeu(&commandes);
    ecran = ECRAN_MENU;
    modeDemonstrationActif = 0;
    valeurDecompte = 0;
    tempsDecompteMs = 0;
    pseudoJoueur[0] = '\0';

    while (!actions.quitter) {
        repriseDisponible = fichier_existe("savegame.dat");

        switch (ecran) {
            case ECRAN_MENU:
                traiter_ihm_menu(&actions, repriseDisponible);

                if (actions.nouvellePartie) {
                    pseudoJoueur[0] = '\0';
                    ecran = ECRAN_SAISIE_PSEUDO;
                } else if (actions.reprendrePartie) {
                    if (charger_etat_jeu(&etat, "savegame.dat")) {
                        valeurDecompte = 3;
                        tempsDecompteMs = 1000;
                        ecran = ECRAN_DECOMPTE;
                    }
                } else if (actions.ouvrirParametres) {
                    ecran = ECRAN_PARAMETRES;
                } else if (actions.ouvrirRegles) {
                    ecran = ECRAN_REGLES;
                }

                dessiner_menu_depart(&ressources, actions.menuSelection, repriseDisponible);
                break;

            case ECRAN_SAISIE_PSEUDO:
                traiter_ihm_saisie_pseudo(&actions, pseudoJoueur, TAILLE_PSEUDO_MAX);
                if (actions.retourMenu) {
                    ecran = ECRAN_MENU;
                } else if (actions.valider) {
                    if (reinitialiser_partie(&etat, &configuration, 1)) {
                        definir_pseudo_joueur(&etat, pseudoJoueur);
                        valeurDecompte = 3;
                        tempsDecompteMs = 1000;
                        ecran = ECRAN_DECOMPTE;
                    }
                }
                dessiner_saisie_pseudo(&ressources, pseudoJoueur);
                break;

            case ECRAN_DECOMPTE:
                dessiner_decompte_depart(&ressources, &etat, valeurDecompte);
                tempsDecompteMs -= 16;
                if (tempsDecompteMs <= 0) {
                    valeurDecompte--;
                    if (valeurDecompte <= 0) {
                        ecran = ECRAN_JEU;
                    } else {
                        tempsDecompteMs = 1000;
                    }
                }
                break;

            case ECRAN_PARAMETRES:
                traiter_ihm_parametres(&actions, modeDemonstrationActif);
                if (actions.basculerModeDemonstration) {
                    modeDemonstrationActif = !modeDemonstrationActif;
                    if (!modeDemonstrationActif && actions.parametresSelection > 1) {
                        actions.parametresSelection = 0;
                    }
                } else if (actions.lancerDemoNiveau > 0 && modeDemonstrationActif) {
                    if (reinitialiser_partie(&etat, &configuration, actions.lancerDemoNiveau)) {
                        valeurDecompte = 3;
                        tempsDecompteMs = 1000;
                        ecran = ECRAN_DECOMPTE;
                    }
                } else if (actions.retourMenu) {
                    ecran = ECRAN_MENU;
                }
                if (ecran != ECRAN_PARAMETRES) {
                    break;
                }
                dessiner_menu_parametres(&ressources,
                                         actions.parametresSelection,
                                         modeDemonstrationActif);
                break;

            case ECRAN_REGLES:
                traiter_ihm_ecran_secondaire(&actions);
                if (actions.retourMenu) {
                    ecran = ECRAN_MENU;
                }
                dessiner_ecran_information(&ressources,
                                           "REGLES",
                                           "Gauche/Droite pour bouger.",
                                           "Haut pour tirer sur les bulles.",
                                           "S pour sauvegarder, L pour charger.");
                break;

            case ECRAN_JEU:
                traiter_ihm_jeu(&actions, &commandes, etat.perdu || etat.gagne);

                if (actions.sauvegarder) {
                    sauvegarder_etat_jeu(&etat, "savegame.dat");
                }

                if (actions.charger) {
                    charger_etat_jeu(&etat, "savegame.dat");
                }

                if (actions.retourMenu) {
                    if (etat.gagne) {
                        if (etat.niveau < etat.niveauMaximum) {
                            if (reinitialiser_partie(&etat, &configuration, etat.niveau + 1)) {
                                valeurDecompte = 3;
                                tempsDecompteMs = 1000;
                                ecran = ECRAN_DECOMPTE;
                            }
                        } else {
                            ecran = ECRAN_MENU;
                        }
                    } else {
                        ecran = ECRAN_MENU;
                    }
                    break;
                }

                if (!actions.quitter) {
                    mettre_a_jour_logique_jeu(&etat, &configuration, &commandes);
                }

                dessiner_jeu(&ressources, &etat);
                break;
        }

        rest(16);
    }

    fermer_sauvegarde();
    fermer_logique_jeu(&etat);
    liberer_ressources_jeu(&ressources);
    fermer_affichage();

    return 0;
}
END_OF_MAIN();
