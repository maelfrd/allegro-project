#ifndef ALLEGRO_TEST_IHM_H
#define ALLEGRO_TEST_IHM_H

#include "logique_jeu.h"

typedef struct {
    int quitter;
    int sauvegarder;
    int charger;
    int nouvellePartie;
    int nouvellePartieJeu;
    int reprendrePartie;
    int ouvrirParametres;
    int ouvrirRegles;
    int retourMenu;
    int recommencerNiveau;
    int valider;
    int menuSelection;
    int parametresSelection;
    int defaiteSelection;
    int basculerModeDemonstration;
    int lancerDemoNiveau;
    int oldMenuUpState;
    int oldMenuDownState;
    int oldMenuEnterState;
    int oldGameEnterState;
    int oldBackState;
    int oldSaveState;
    int oldLoadState;
    int oldShootState;
} ActionsIHM;

void initialiser_actions_ihm(ActionsIHM *actions);
void traiter_ihm_menu(ActionsIHM *actions, int repriseDisponible);
void traiter_ihm_ecran_secondaire(ActionsIHM *actions);
void traiter_ihm_parametres(ActionsIHM *actions, int modeDemonstrationActif);
void traiter_ihm_jeu(ActionsIHM *actions,
                     CommandesJeu *commandes,
                     int partiePerdue,
                     int partieGagnee,
                     int tirContinuActif);
void traiter_ihm_saisie_pseudo(ActionsIHM *actions, char *pseudo, int taillePseudo);

#endif
