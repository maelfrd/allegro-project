#ifndef LOGIQUE_JEU_H
#define LOGIQUE_JEU_H

#define TAILLE_PSEUDO_MAX 32

typedef enum {
    BULLE_TRES_GRANDE = 0,
    BULLE_GRANDE,
    BULLE_MOYENNE,
    BULLE_PETITE,
    BULLE_TAILLES_TOTAL
} TailleBulle;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    TailleBulle taille;
    float gravite;
    float rebondSol;
    float attenuationX;
    int largeur;
    int hauteur;
} Bulle;

typedef struct {
    int largeurFenetre;
    int hauteurFenetre;
    int groundY;
    int leftLimit;
    int rightLimit;
    int vitesseJoueur;
    int joueurLargeur;
    int joueurHauteur;
    int projectileLargeur;
    int projectileHauteur;
    int projectileVitesse;
    int largeurBulles[BULLE_TAILLES_TOTAL];
    int hauteurBulles[BULLE_TAILLES_TOTAL];
} ConfigurationJeu;

typedef struct {
    int deplacementHorizontal;
    int tirer;
} CommandesJeu;

typedef struct {
    Bulle *bulles;
    int nbBulles;
    int capaciteBulles;
    int niveau;
    int niveauMaximum;
    int groundY;
    int leftLimit;
    int rightLimit;
    int x;
    int y;
    int speed;
    int projectileActive;
    int projectileX;
    int projectileY;
    int projectileW;
    int projectileH;
    int projectileSpeed;
    int perdu;
    int gagne;
    char pseudo[TAILLE_PSEUDO_MAX];
} EtatJeu;

int initialiser_logique_jeu(EtatJeu *etat, const ConfigurationJeu *configuration);
int reinitialiser_partie(EtatJeu *etat, const ConfigurationJeu *configuration, int niveau);
void definir_pseudo_joueur(EtatJeu *etat, const char *pseudo);
void initialiser_commandes_jeu(CommandesJeu *commandes);
void mettre_a_jour_logique_jeu(EtatJeu *etat,
                               const ConfigurationJeu *configuration,
                               const CommandesJeu *commandes);
void fermer_logique_jeu(EtatJeu *etat);

#endif
