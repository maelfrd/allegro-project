#include "IHM.h"

#include <allegro.h>
#include <string.h>

enum {
    MENU_NOUVELLE_PARTIE,
    MENU_REPRENDRE_PARTIE,
    MENU_QUITTER,
    MENU_PARAMETRES,
    MENU_REGLES,
    MENU_TOTAL
};

enum {
    PARAM_MODE_DEMO,
    PARAM_DEMO_NIVEAU_1,
    PARAM_DEMO_NIVEAU_2,
    PARAM_DEMO_NIVEAU_3,
    PARAM_DEMO_NIVEAU_4,
    PARAM_DEMO_NIVEAU_5,
    PARAM_RETOUR
};

static int caractere_pseudo_valide(int caractere) {
    return ((caractere >= 'a' && caractere <= 'z') ||
            (caractere >= 'A' && caractere <= 'Z') ||
            (caractere >= CARACTERE_CHIFFRE_MIN && caractere <= CARACTERE_CHIFFRE_MAX) ||
            caractere == '_' ||
            caractere == '-');
}

static void reinitialiser_actions_navigation(ActionsIHM *actions) {
    actions->nouvellePartie = FAUX;
    actions->reprendrePartie = FAUX;
    actions->ouvrirParametres = FAUX;
    actions->ouvrirRegles = FAUX;
    actions->retourMenu = FAUX;
    actions->valider = FAUX;
    actions->niveauChoisi = VALEUR_NULLE;
    actions->sauvegardeChoisie = INDEX_INVALIDE;
    actions->basculerModeDemonstration = FAUX;
    actions->lancerDemoNiveau = VALEUR_NULLE;
}

void initialiser_actions_ihm(ActionsIHM *actions) {
    if (!actions) {
        return;
    }

    memset(actions, VALEUR_NULLE, sizeof(*actions));
    actions->menuSelection = MENU_NOUVELLE_PARTIE;
    actions->parametresSelection = PARAM_MODE_DEMO;
    actions->niveauSelection = INDEX_PREMIER;
    actions->sauvegardeSelection = INDEX_PREMIER;
}

void traiter_ihm_menu(ActionsIHM *actions, int repriseDisponible) {
    if (!actions) {
        return;
    }

    actions->quitter = FAUX;
    actions->sauvegarder = FAUX;
    actions->charger = FAUX;
    reinitialiser_actions_navigation(actions);

    if (key[KEY_UP] && !actions->toucheHautMenuPrecedente) {
        actions->menuSelection--;
        if (actions->menuSelection < VALEUR_NULLE) {
            actions->menuSelection = MENU_TOTAL - INDEX_SUIVANT;
        }
    }

    if (key[KEY_DOWN] && !actions->toucheBasMenuPrecedente) {
        actions->menuSelection++;
        if (actions->menuSelection >= MENU_TOTAL) {
            actions->menuSelection = MENU_NOUVELLE_PARTIE;
        }
    }

    if (key[KEY_ENTER] && !actions->toucheEntreeMenuPrecedente) {
        switch (actions->menuSelection) {
            case MENU_NOUVELLE_PARTIE:
                actions->nouvellePartie = VRAI;
                break;
            case MENU_REPRENDRE_PARTIE:
                if (repriseDisponible) {
                    actions->reprendrePartie = VRAI;
                }
                break;
            case MENU_QUITTER:
                actions->quitter = VRAI;
                break;
            case MENU_PARAMETRES:
                actions->ouvrirParametres = VRAI;
                break;
            case MENU_REGLES:
                actions->ouvrirRegles = VRAI;
                break;
        }
    }

    if (key[KEY_ESC]) {
        actions->quitter = VRAI;
    }

    actions->toucheHautMenuPrecedente = key[KEY_UP];
    actions->toucheBasMenuPrecedente = key[KEY_DOWN];
    actions->toucheEntreeMenuPrecedente = key[KEY_ENTER];
    actions->toucheRetourPrecedente = key[KEY_ESC];
}

void traiter_ihm_ecran_secondaire(ActionsIHM *actions) {
    if (!actions) {
        return;
    }

    actions->quitter = FAUX;
    actions->sauvegarder = FAUX;
    actions->charger = FAUX;
    reinitialiser_actions_navigation(actions);

    if ((key[KEY_ESC] || key[KEY_BACKSPACE]) && !actions->toucheRetourPrecedente) {
        actions->retourMenu = VRAI;
    }

    actions->toucheRetourPrecedente = key[KEY_ESC] || key[KEY_BACKSPACE];
    actions->toucheHautMenuPrecedente = key[KEY_UP];
    actions->toucheBasMenuPrecedente = key[KEY_DOWN];
    actions->toucheEntreeMenuPrecedente = key[KEY_ENTER];
}

void traiter_ihm_parametres(ActionsIHM *actions, int modeDemonstrationActif) {
    int selectionMax = modeDemonstrationActif ? PARAM_RETOUR : PARAM_MODE_DEMO + INDEX_SUIVANT;

    if (!actions) {
        return;
    }

    actions->quitter = FAUX;
    actions->sauvegarder = FAUX;
    actions->charger = FAUX;
    reinitialiser_actions_navigation(actions);

    if (key[KEY_UP] && !actions->toucheHautMenuPrecedente) {
        actions->parametresSelection--;
        if (actions->parametresSelection < VALEUR_NULLE) {
            actions->parametresSelection = selectionMax;
        }
    }

    if (key[KEY_DOWN] && !actions->toucheBasMenuPrecedente) {
        actions->parametresSelection++;
        if (actions->parametresSelection > selectionMax) {
            actions->parametresSelection = PARAM_MODE_DEMO;
        }
    }

    if (!modeDemonstrationActif && actions->parametresSelection > PARAM_MODE_DEMO + INDEX_SUIVANT) {
        actions->parametresSelection = PARAM_MODE_DEMO;
    }

    if (key[KEY_ENTER] && !actions->toucheEntreeMenuPrecedente) {
        switch (actions->parametresSelection) {
            case PARAM_MODE_DEMO:
                actions->basculerModeDemonstration = VRAI;
                break;
            case PARAM_DEMO_NIVEAU_1:
                actions->lancerDemoNiveau = PREMIER_NIVEAU_JEU;
                break;
            case PARAM_DEMO_NIVEAU_2:
                actions->lancerDemoNiveau = VALEUR_DOUBLE;
                break;
            case PARAM_DEMO_NIVEAU_3:
                actions->lancerDemoNiveau = VALEUR_TRIPLE;
                break;
            case PARAM_DEMO_NIVEAU_4:
                actions->lancerDemoNiveau = VALEUR_QUADRUPLE;
                break;
            case PARAM_DEMO_NIVEAU_5:
                actions->lancerDemoNiveau = VALEUR_QUINTUPLE;
                break;
            case PARAM_RETOUR:
                actions->retourMenu = VRAI;
                break;
        }
    }

    if ((key[KEY_ESC] || key[KEY_BACKSPACE]) && !actions->toucheRetourPrecedente) {
        actions->retourMenu = VRAI;
    }

    actions->toucheRetourPrecedente = key[KEY_ESC] || key[KEY_BACKSPACE];
    actions->toucheHautMenuPrecedente = key[KEY_UP];
    actions->toucheBasMenuPrecedente = key[KEY_DOWN];
    actions->toucheEntreeMenuPrecedente = key[KEY_ENTER];
}

void traiter_ihm_selection_niveau(ActionsIHM *actions, int niveauMaximumDebloque) {
    int selectionMax;

    if (!actions) {
        return;
    }

    if (niveauMaximumDebloque < PREMIER_NIVEAU_JEU) {
        niveauMaximumDebloque = PREMIER_NIVEAU_JEU;
    }

    actions->quitter = FAUX;
    actions->sauvegarder = FAUX;
    actions->charger = FAUX;
    reinitialiser_actions_navigation(actions);

    selectionMax = niveauMaximumDebloque;
    if (actions->niveauSelection > selectionMax) {
        actions->niveauSelection = selectionMax;
    }

    if (key[KEY_UP] && !actions->toucheHautMenuPrecedente) {
        actions->niveauSelection--;
        if (actions->niveauSelection < VALEUR_NULLE) {
            actions->niveauSelection = selectionMax;
        }
    }

    if (key[KEY_DOWN] && !actions->toucheBasMenuPrecedente) {
        actions->niveauSelection++;
        if (actions->niveauSelection > selectionMax) {
            actions->niveauSelection = INDEX_PREMIER;
        }
    }

    if (key[KEY_ENTER] && !actions->toucheEntreeMenuPrecedente) {
        if (actions->niveauSelection < niveauMaximumDebloque) {
            actions->niveauChoisi = actions->niveauSelection + INDEX_SUIVANT;
        } else {
            actions->retourMenu = VRAI;
        }
    }

    if ((key[KEY_ESC] || key[KEY_BACKSPACE]) && !actions->toucheRetourPrecedente) {
        actions->retourMenu = VRAI;
    }

    actions->toucheRetourPrecedente = key[KEY_ESC] || key[KEY_BACKSPACE];
    actions->toucheHautMenuPrecedente = key[KEY_UP];
    actions->toucheBasMenuPrecedente = key[KEY_DOWN];
    actions->toucheEntreeMenuPrecedente = key[KEY_ENTER];
}

void traiter_ihm_selection_sauvegarde(ActionsIHM *actions, const int sauvegardesDisponibles[], int nombreSauvegardes) {
    int selectionMax;

    if (!actions || !sauvegardesDisponibles || nombreSauvegardes <= VALEUR_NULLE) {
        return;
    }

    actions->quitter = FAUX;
    actions->sauvegarder = FAUX;
    actions->charger = FAUX;
    reinitialiser_actions_navigation(actions);

    selectionMax = nombreSauvegardes;
    if (actions->sauvegardeSelection > selectionMax) {
        actions->sauvegardeSelection = selectionMax;
    }

    if (key[KEY_UP] && !actions->toucheHautMenuPrecedente) {
        actions->sauvegardeSelection--;
        if (actions->sauvegardeSelection < VALEUR_NULLE) {
            actions->sauvegardeSelection = selectionMax;
        }
    }

    if (key[KEY_DOWN] && !actions->toucheBasMenuPrecedente) {
        actions->sauvegardeSelection++;
        if (actions->sauvegardeSelection > selectionMax) {
            actions->sauvegardeSelection = INDEX_PREMIER;
        }
    }

    if (key[KEY_ENTER] && !actions->toucheEntreeMenuPrecedente) {
        if (actions->sauvegardeSelection < nombreSauvegardes &&
            sauvegardesDisponibles[actions->sauvegardeSelection]) {
            actions->sauvegardeChoisie = actions->sauvegardeSelection;
        } else if (actions->sauvegardeSelection == nombreSauvegardes) {
            actions->retourMenu = VRAI;
        }
    }

    if ((key[KEY_ESC] || key[KEY_BACKSPACE]) && !actions->toucheRetourPrecedente) {
        actions->retourMenu = VRAI;
    }

    actions->toucheRetourPrecedente = key[KEY_ESC] || key[KEY_BACKSPACE];
    actions->toucheHautMenuPrecedente = key[KEY_UP];
    actions->toucheBasMenuPrecedente = key[KEY_DOWN];
    actions->toucheEntreeMenuPrecedente = key[KEY_ENTER];
}

void traiter_ihm_jeu(ActionsIHM *actions, CommandesJeu *commandes, int partieBloquee) {
    if (!actions || !commandes) {
        return;
    }

    initialiser_commandes_jeu(commandes);
    actions->quitter = key[KEY_ESC];
    actions->sauvegarder = FAUX;
    actions->charger = FAUX;
    actions->retourMenu = FAUX;
    actions->valider = FAUX;

    if (!partieBloquee) {
        if (key[KEY_LEFT]) {
            commandes->deplacementHorizontal = -VALEUR_UNITAIRE;
        } else if (key[KEY_RIGHT]) {
            commandes->deplacementHorizontal = VALEUR_UNITAIRE;
        }

        if (key[KEY_UP] && !actions->toucheTirPrecedente) {
            commandes->tirer = VRAI;
        }
    }

    if (key[KEY_S] && !actions->toucheSauvegardePrecedente) {
        actions->sauvegarder = VRAI;
    }
    if (key[KEY_L] && !actions->toucheChargementPrecedente) {
        actions->charger = VRAI;
    }
    if (partieBloquee && key[KEY_ENTER] && !actions->toucheEntreeJeuPrecedente) {
        actions->retourMenu = VRAI;
    }

    actions->toucheSauvegardePrecedente = key[KEY_S];
    actions->toucheChargementPrecedente = key[KEY_L];
    actions->toucheTirPrecedente = key[KEY_UP];
    actions->toucheEntreeJeuPrecedente = key[KEY_ENTER];
    actions->toucheRetourPrecedente = key[KEY_ESC];
}

void traiter_ihm_saisie_pseudo(ActionsIHM *actions, char *pseudoJoueur, int taillePseudo) {
    int longueur;

    if (!actions || !pseudoJoueur || taillePseudo <= VALEUR_UNITAIRE) {
        return;
    }

    actions->quitter = FAUX;
    actions->sauvegarder = FAUX;
    actions->charger = FAUX;
    reinitialiser_actions_navigation(actions);

    longueur = (int) strlen(pseudoJoueur);

    while (keypressed()) {
        int touche = readkey();
        int code = touche >> CODE_BITS_TOUCHE;
        int ascii = touche & MASQUE_ASCII_TOUCHE;

        if (code == KEY_ESC) {
            actions->retourMenu = VRAI;
            return;
        }

        if (code == KEY_ENTER) {
            if (longueur > VALEUR_NULLE) {
                actions->valider = VRAI;
            }
            continue;
        }

        if (code == KEY_BACKSPACE) {
            if (longueur > VALEUR_NULLE) {
                longueur--;
                pseudoJoueur[longueur] = CARACTERE_FIN_CHAINE;
            }
            continue;
        }

        if (caractere_pseudo_valide(ascii) && longueur < taillePseudo - INDEX_SUIVANT) {
            pseudoJoueur[longueur] = (char) ascii;
            longueur++;
            pseudoJoueur[longueur] = CARACTERE_FIN_CHAINE;
        }
    }
}
