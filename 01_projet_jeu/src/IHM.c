#include "IHM.h"

#include <allegro.h>
#include <string.h>

enum {
    MENU_NOUVELLE_PARTIE = 0,
    MENU_REPRENDRE_PARTIE,
    MENU_QUITTER,
    MENU_PARAMETRES,
    MENU_REGLES,
    MENU_TOTAL
};

enum {
    PARAM_MODE_DEMO = 0,
    PARAM_DEMO_NIVEAU_1,
    PARAM_DEMO_NIVEAU_2,
    PARAM_DEMO_NIVEAU_3,
    PARAM_DEMO_NIVEAU_4,
    PARAM_DEMO_NIVEAU_5,
    PARAM_RETOUR
};

enum {
    MENU_DEFAITE_RECOMMENCER = 0,
    MENU_DEFAITE_NOUVELLE_PARTIE,
    MENU_DEFAITE_RETOUR_MENU,
    MENU_DEFAITE_SAUVEGARDER,
    MENU_DEFAITE_TOTAL
};

static int caractere_pseudo_valide(int caractere) {
    return ((caractere >= 'a' && caractere <= 'z') ||
            (caractere >= 'A' && caractere <= 'Z') ||
            (caractere >= '0' && caractere <= '9') ||
            caractere == '_' ||
            caractere == '-');
}

static void reinitialiser_actions_navigation(ActionsIHM *actions) {
    actions->nouvellePartie = 0;
    actions->nouvellePartieJeu = 0;
    actions->reprendrePartie = 0;
    actions->ouvrirParametres = 0;
    actions->ouvrirRegles = 0;
    actions->retourMenu = 0;
    actions->recommencerNiveau = 0;
    actions->valider = 0;
    actions->basculerModeDemonstration = 0;
    actions->lancerDemoNiveau = 0;
}

void initialiser_actions_ihm(ActionsIHM *actions) {
    if (!actions) {
        return;
    }

    memset(actions, 0, sizeof(*actions));
    actions->menuSelection = MENU_NOUVELLE_PARTIE;
    actions->parametresSelection = PARAM_MODE_DEMO;
    actions->defaiteSelection = MENU_DEFAITE_RECOMMENCER;
}

void traiter_ihm_menu(ActionsIHM *actions, int repriseDisponible) {
    if (!actions) {
        return;
    }

    actions->quitter = 0;
    actions->sauvegarder = 0;
    actions->charger = 0;
    reinitialiser_actions_navigation(actions);

    if (key[KEY_UP] && !actions->oldMenuUpState) {
        actions->menuSelection--;
        if (actions->menuSelection < 0) {
            actions->menuSelection = MENU_TOTAL - 1;
        }
    }

    if (key[KEY_DOWN] && !actions->oldMenuDownState) {
        actions->menuSelection++;
        if (actions->menuSelection >= MENU_TOTAL) {
            actions->menuSelection = 0;
        }
    }

    if (key[KEY_ENTER] && !actions->oldMenuEnterState) {
        switch (actions->menuSelection) {
            case MENU_NOUVELLE_PARTIE:
                actions->nouvellePartie = 1;
                break;
            case MENU_REPRENDRE_PARTIE:
                if (repriseDisponible) {
                    actions->reprendrePartie = 1;
                }
                break;
            case MENU_QUITTER:
                actions->quitter = 1;
                break;
            case MENU_PARAMETRES:
                actions->ouvrirParametres = 1;
                break;
            case MENU_REGLES:
                actions->ouvrirRegles = 1;
                break;
        }
    }

    if (key[KEY_ESC]) {
        actions->quitter = 1;
    }

    actions->oldMenuUpState = key[KEY_UP];
    actions->oldMenuDownState = key[KEY_DOWN];
    actions->oldMenuEnterState = key[KEY_ENTER];
    actions->oldBackState = key[KEY_ESC];
}

void traiter_ihm_ecran_secondaire(ActionsIHM *actions) {
    if (!actions) {
        return;
    }

    actions->quitter = 0;
    actions->sauvegarder = 0;
    actions->charger = 0;
    reinitialiser_actions_navigation(actions);

    if ((key[KEY_ESC] || key[KEY_BACKSPACE]) && !actions->oldBackState) {
        actions->retourMenu = 1;
    }

    actions->oldBackState = key[KEY_ESC] || key[KEY_BACKSPACE];
    actions->oldMenuUpState = key[KEY_UP];
    actions->oldMenuDownState = key[KEY_DOWN];
    actions->oldMenuEnterState = key[KEY_ENTER];
}

void traiter_ihm_parametres(ActionsIHM *actions, int modeDemonstrationActif) {
    int selectionMax = modeDemonstrationActif ? PARAM_RETOUR : PARAM_MODE_DEMO + 1;

    if (!actions) {
        return;
    }

    actions->quitter = 0;
    actions->sauvegarder = 0;
    actions->charger = 0;
    reinitialiser_actions_navigation(actions);

    if (key[KEY_UP] && !actions->oldMenuUpState) {
        actions->parametresSelection--;
        if (actions->parametresSelection < 0) {
            actions->parametresSelection = selectionMax;
        }
    }

    if (key[KEY_DOWN] && !actions->oldMenuDownState) {
        actions->parametresSelection++;
        if (actions->parametresSelection > selectionMax) {
            actions->parametresSelection = 0;
        }
    }

    if (!modeDemonstrationActif && actions->parametresSelection > PARAM_MODE_DEMO + 1) {
        actions->parametresSelection = PARAM_MODE_DEMO;
    }

    if (key[KEY_ENTER] && !actions->oldMenuEnterState) {
        switch (actions->parametresSelection) {
            case PARAM_MODE_DEMO:
                actions->basculerModeDemonstration = 1;
                break;
            case PARAM_DEMO_NIVEAU_1:
                actions->lancerDemoNiveau = 1;
                break;
            case PARAM_DEMO_NIVEAU_2:
                actions->lancerDemoNiveau = 2;
                break;
            case PARAM_DEMO_NIVEAU_3:
                actions->lancerDemoNiveau = 3;
                break;
            case PARAM_DEMO_NIVEAU_4:
                actions->lancerDemoNiveau = 4;
                break;
            case PARAM_DEMO_NIVEAU_5:
                actions->lancerDemoNiveau = 5;
                break;
            case PARAM_RETOUR:
                actions->retourMenu = 1;
                break;
        }
    }

    if ((key[KEY_ESC] || key[KEY_BACKSPACE]) && !actions->oldBackState) {
        actions->retourMenu = 1;
    }

    actions->oldBackState = key[KEY_ESC] || key[KEY_BACKSPACE];
    actions->oldMenuUpState = key[KEY_UP];
    actions->oldMenuDownState = key[KEY_DOWN];
    actions->oldMenuEnterState = key[KEY_ENTER];
}

void traiter_ihm_jeu(ActionsIHM *actions,
                     CommandesJeu *commandes,
                     int partiePerdue,
                     int partieGagnee,
                     int tirContinuActif) {
    if (!actions || !commandes) {
        return;
    }

    initialiser_commandes_jeu(commandes);
    actions->quitter = (!partiePerdue && !partieGagnee) ? key[KEY_ESC] : 0;
    actions->sauvegarder = 0;
    actions->charger = 0;
    reinitialiser_actions_navigation(actions);

    if (partiePerdue) {
        if (key[KEY_UP] && !actions->oldMenuUpState) {
            actions->defaiteSelection--;
            if (actions->defaiteSelection < 0) {
                actions->defaiteSelection = MENU_DEFAITE_TOTAL - 1;
            }
        }

        if (key[KEY_DOWN] && !actions->oldMenuDownState) {
            actions->defaiteSelection++;
            if (actions->defaiteSelection >= MENU_DEFAITE_TOTAL) {
                actions->defaiteSelection = 0;
            }
        }

        if (key[KEY_ENTER] && !actions->oldGameEnterState) {
            switch (actions->defaiteSelection) {
                case MENU_DEFAITE_RECOMMENCER:
                    actions->recommencerNiveau = 1;
                    break;
                case MENU_DEFAITE_NOUVELLE_PARTIE:
                    actions->nouvellePartieJeu = 1;
                    break;
                case MENU_DEFAITE_RETOUR_MENU:
                    actions->retourMenu = 1;
                    break;
                case MENU_DEFAITE_SAUVEGARDER:
                    actions->sauvegarder = 1;
                    break;
            }
        }
    } else if (!partieGagnee) {
        if (key[KEY_LEFT]) {
            commandes->deplacementHorizontal = -1;
        } else if (key[KEY_RIGHT]) {
            commandes->deplacementHorizontal = 1;
        }

        if (tirContinuActif && key[KEY_UP]) {
            commandes->tirer = 1;
        } else if (key[KEY_UP] && !actions->oldShootState) {
            commandes->tirer = 1;
        }
    }

    if (key[KEY_S] && !actions->oldSaveState) {
        actions->sauvegarder = 1;
    }
    if (key[KEY_L] && !actions->oldLoadState) {
        actions->charger = 1;
    }
    if (partieGagnee && key[KEY_ENTER] && !actions->oldGameEnterState) {
        actions->retourMenu = 1;
    }

    actions->oldSaveState = key[KEY_S];
    actions->oldLoadState = key[KEY_L];
    actions->oldShootState = key[KEY_UP];
    actions->oldGameEnterState = key[KEY_ENTER];
    actions->oldMenuUpState = key[KEY_UP];
    actions->oldMenuDownState = key[KEY_DOWN];
    actions->oldBackState = key[KEY_ESC];
}

void traiter_ihm_saisie_pseudo(ActionsIHM *actions, char *pseudo, int taillePseudo) {
    int longueur;

    if (!actions || !pseudo || taillePseudo <= 1) {
        return;
    }

    actions->quitter = 0;
    actions->sauvegarder = 0;
    actions->charger = 0;
    reinitialiser_actions_navigation(actions);

    longueur = (int) strlen(pseudo);

    while (keypressed()) {
        int touche = readkey();
        int code = touche >> 8;
        int ascii = touche & 0xff;

        if (code == KEY_ESC) {
            actions->retourMenu = 1;
            return;
        }

        if (code == KEY_ENTER) {
            if (longueur > 0) {
                actions->valider = 1;
            }
            continue;
        }

        if (code == KEY_BACKSPACE) {
            if (longueur > 0) {
                longueur--;
                pseudo[longueur] = '\0';
            }
            continue;
        }

        if (caractere_pseudo_valide(ascii) && longueur < taillePseudo - 1) {
            pseudo[longueur] = (char) ascii;
            longueur++;
            pseudo[longueur] = '\0';
        }
    }
}
