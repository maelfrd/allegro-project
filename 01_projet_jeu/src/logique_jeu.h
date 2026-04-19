#ifndef LOGIQUE_JEU_H
#define LOGIQUE_JEU_H

#define TAILLE_PSEUDO_MAX 32
#define DUREE_NIVEAU_MS 180000
#define INTERVALLE_ECLAIR_MS 30000
#define DUREE_POUVOIR_MS 15000
#define DUREE_DOUBLE_TIR_MS 10000
#define MAX_ECLAIRS 64

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

typedef enum {
    BONUS_AUCUN = 0,
    BONUS_CHAPEAU,
    BONUS_LANCE_FLAMMES,
    BONUS_DOUBLE_TIR
} TypeBonus;

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
} Bulle;

typedef struct {
    float x;
    float y;
    float vitesseY;
    int largeur;
    int hauteur;
} Eclair;

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
    int feuLargeur;
    int feuHauteur;
    int chapeauLargeur;
    int chapeauHauteur;
    int doubleTirLargeur;
    int doubleTirHauteur;
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
    int vitesse;
    int tirActif;
    int tirX;
    int tirY;
    int tirLargeur;
    int tirHauteur;
    int vitesseTir;
    int tirSecondaireActif;
    int tirSecondaireX;
    int tirSecondaireY;
    int bonusVisible;
    int bonusX;
    int bonusY;
    int bonusLargeur;
    int bonusHauteur;
    float bonusVx;
    float bonusVy;
    int explosionActive;
    int explosionX;
    int explosionY;
    int explosionLargeur;
    int explosionHauteur;
    int explosionTimer;
    int typeBonusQuiTombe;
    int typeBonusActif;
    int tempsBonusActifMs;
    int tempsRestantMs;
    int prochainDeclenchementEclairMs;
    Eclair eclairs[MAX_ECLAIRS];
    int nbEclairs;
    int bossTouchesAvantReduction;
    int score;
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
                               const CommandesJeu *commandes,
                               int deltaTempsMs);
void fermer_logique_jeu(EtatJeu *etat);

#endif
