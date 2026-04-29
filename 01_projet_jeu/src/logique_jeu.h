#ifndef LOGIQUE_JEU_H
#define LOGIQUE_JEU_H

#include "config_jeu.h"

typedef enum {
    BULLE_TRES_GRANDE,
    BULLE_GRANDE,
    BULLE_MOYENNE,
    BULLE_PETITE,
    BULLE_TAILLES_TOTAL
} TailleBulle;

typedef enum {
    ENTITE_MANGE_MORT,
    ENTITE_VIF_DOR,
    ENTITE_VOL_DE_MORT
} TypeEntite;

typedef struct {
    float positionBulleX;
    float positionBulleY;
    float vitesseBulleX;
    float vitesseBulleY;
    TypeEntite typeEntite;
    TailleBulle tailleBulle;
    float gravite;
    float rebondSol;
    float attenuationX;
    int largeur;
    int hauteur;
    int nombreCoupsAvantDivision;
} Bulle;

typedef struct {
    int largeurFenetre;
    int hauteurFenetre;
    int positionSolY;
    int limiteGaucheTerrain;
    int limiteDroiteTerrain;
    int vitesseJoueur;
    int joueurLargeur;
    int joueurHauteur;
    int projectileLargeur;
    int projectileHauteur;
    int projectileVitesse;
    int chapeauLargeur;
    int chapeauHauteur;
    int explosionLargeur;
    int explosionHauteur;
    int largeurBulles[BULLE_TAILLES_TOTAL];
    int hauteurBulles[BULLE_TAILLES_TOTAL];
} ConfigurationJeu;

typedef struct {
    int deplacementHorizontal;
    int tirer;
} CommandesJeu;

typedef struct {
    Bulle *bullesEnJeu;
    int nombreBulles;
    int capaciteMaxBullesEnJeu;
    int niveauActuel;
    int niveauMaximum;
    int positionSolY;
    int limiteGaucheTerrain;
    int limiteDroiteTerrain;
    int positionJoueurX;
    int positionJoueurY;
    int vitesseJoueur;
    int projectileEstActif;
    int positionProjectileX;
    int positionProjectileY;
    int largeurProjectile;
    int hauteurProjectile;
    int vitesseProjectile;
    int chapeauEstVisible;
    int positionChapeauX;
    int positionChapeauY;
    int largeurChapeau;
    int hauteurChapeau;
    float vitesseChapeauX;
    float vitesseChapeauY;
    int explosionEstActive;
    int positionExplosionX;
    int positionExplosionY;
    int largeurExplosion;
    int hauteurExplosion;
    int dureeExplosionRestanteImages;
    int auraArdenteEstActive;
    int dureeRestanteAuraArdenteMs;
    int partiePerdue;
    int partieGagnee;
    int scoreJoueur;
    int tempsRestantNiveauMs;
    char pseudoJoueur[TAILLE_PSEUDO_MAX];
} EtatJeu;

int initialiser_logique_jeu(EtatJeu *etatJeu, const ConfigurationJeu *configuration);
int reinitialiser_partie(EtatJeu *etatJeu, const ConfigurationJeu *configuration, int niveauActuel);
void definir_pseudo_joueur(EtatJeu *etatJeu, const char *pseudoJoueur);
void initialiser_commandes_jeu(CommandesJeu *commandes);
void mettre_a_jour_logique_jeu(EtatJeu *etatJeu,
                               const ConfigurationJeu *configuration,
                               const CommandesJeu *commandes);
void fermer_logique_jeu(EtatJeu *etatJeu);

#endif
