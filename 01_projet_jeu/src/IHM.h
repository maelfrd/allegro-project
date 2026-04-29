#ifndef ALLEGRO_TEST_IHM_H
#define ALLEGRO_TEST_IHM_H

#include "logique_jeu.h"

typedef struct {
    int quitter;
    int sauvegarder;
    int charger;
    int nouvellePartie;
    int reprendrePartie;
    int ouvrirParametres;
    int ouvrirRegles;
    int retourMenu;
    int valider;
    int menuSelection;
    int parametresSelection;
    int niveauSelection;
    int niveauChoisi;
    int sauvegardeSelection;
    int sauvegardeChoisie;
    int basculerModeDemonstration;
    int lancerDemoNiveau;
    int toucheHautMenuPrecedente;
    int toucheBasMenuPrecedente;
    int toucheEntreeMenuPrecedente;
    int toucheEntreeJeuPrecedente;
    int toucheRetourPrecedente;
    int toucheSauvegardePrecedente;
    int toucheChargementPrecedente;
    int toucheTirPrecedente;
} ActionsIHM;

void initialiser_actions_ihm(ActionsIHM *actions);
void traiter_ihm_menu(ActionsIHM *actions, int repriseDisponible);
void traiter_ihm_ecran_secondaire(ActionsIHM *actions);
void traiter_ihm_parametres(ActionsIHM *actions, int modeDemonstrationActif);
void traiter_ihm_selection_niveau(ActionsIHM *actions, int niveauMaximumDebloque);
void traiter_ihm_selection_sauvegarde(ActionsIHM *actions, const int sauvegardesDisponibles[], int nombreSauvegardes);
void traiter_ihm_jeu(ActionsIHM *actions, CommandesJeu *commandes, int partieBloquee);
void traiter_ihm_saisie_pseudo(ActionsIHM *actions, char *pseudoJoueur, int taillePseudo);

#endif
