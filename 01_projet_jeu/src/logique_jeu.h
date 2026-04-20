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

typedef enum {
    ENTITE_MANGE_MORT = 0,
    ENTITE_VIF_DOR,
    ENTITE_VOL_DE_MORT
} TypeEntite;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    TypeEntite type;
    TailleBulle taille;
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
    int groundY;
    int leftLimit;
    int rightLimit;
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
    int chapeauVisible;
    int chapeauX;
    int chapeauY;
    int chapeauW;
    int chapeauH;
    float chapeauVx;
    float chapeauVy;
    int explosionActive;
    int explosionX;
    int explosionY;
    int explosionW;
    int explosionH;
    int explosionTimer;
    int auraArdenteActive;
    int dureeRestanteAuraArdenteMs;
    int perdu;
    int gagne;
    int score;
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
