#include "affichage.h"
#include "IHM.h"
#include "logique_jeu.h"
#include "sauvegarde.h"

#include <stdio.h>

#define LARGEUR_FENETRE_JEU 1536
#define HAUTEUR_FENETRE_JEU 1024
#define PROFONDEUR_COULEUR_JEU 32
#define DUREE_UNE_SECONDE_MS 1000
#define DUREE_UNE_IMAGE_MS 16
#define VALEUR_DECOMPTE_DEPART 3
#define PREMIER_NIVEAU_JEU 1
#define DIVISEUR_HAUTEUR_SOL 6
#define DIVISEUR_VITESSE_JOUEUR 640
#define DIVISEUR_LARGEUR_PROJECTILE 160
#define LARGEUR_MIN_PROJECTILE 6
#define DIVISEUR_HAUTEUR_PROJECTILE 36
#define HAUTEUR_MIN_PROJECTILE 16
#define DIVISEUR_VITESSE_PROJECTILE 90
#define VITESSE_MIN_PROJECTILE 6

#define CHEMIN_SAUVEGARDE "savegame.dat"
#define CHEMIN_FOND_PRINCIPAL "assets/fg.bmp"
#define CHEMIN_IMAGE_JOUEUR "assets/hary2.bmp"
#define CHEMIN_IMAGE_MANGE_MORT "assets/mangemort.bmp"
#define CHEMIN_IMAGE_VIF_DOR "assets/vifdor3.bmp"
#define CHEMIN_IMAGE_TIR "assets/tir.bmp"
#define CHEMIN_IMAGE_AURA_ARDENTE "assets/effet feu .bmp"
#define CHEMIN_IMAGE_CHAPEAU "assets/chapeau.bmp"
#define CHEMIN_IMAGE_EXPLOSION "assets/explosion.bmp"

typedef enum {
    ECRAN_MENU = 0,
    ECRAN_SAISIE_PSEUDO,
    ECRAN_DECOMPTE,
    ECRAN_JEU,
    ECRAN_PARAMETRES,
    ECRAN_REGLES
} EcranActif;

typedef struct {
    RessourcesJeu ressources;
    ConfigurationJeu configuration;
    EtatJeu etat;
    ActionsIHM actions;
    CommandesJeu commandes;
    EcranActif ecranActuel;
    int repriseDisponible;
    int modeDemonstrationActif;
    int valeurDecompte;
    int tempsRestantDecompteMs;
    char pseudoJoueur[TAILLE_PSEUDO_MAX];
} ApplicationJeu;

static void construire_configuration_jeu(ConfigurationJeu *configuration,
                                         const RessourcesJeu *ressources);
static int fichier_existe(const char *cheminFichier);
static void initialiser_decompte_depart(ApplicationJeu *application);
static void vider_pseudo_joueur(ApplicationJeu *application);
static int demarrer_systemes_jeu(ApplicationJeu *application);
static void fermer_systemes_jeu(ApplicationJeu *application);
static int lancer_niveau(ApplicationJeu *application, int numeroNiveau);
static int charger_partie_et_lancer_decompte(ApplicationJeu *application);
static void traiter_ecran_menu(ApplicationJeu *application);
static void traiter_ecran_saisie_pseudo(ApplicationJeu *application);
static void traiter_ecran_decompte(ApplicationJeu *application);
static void traiter_ecran_parametres(ApplicationJeu *application);
static void traiter_ecran_regles(ApplicationJeu *application);
static void traiter_retour_depuis_le_jeu(ApplicationJeu *application);
static void traiter_ecran_jeu(ApplicationJeu *application);

int main(void) {
    ApplicationJeu application;

    /* --- initialiser Allegro, les ressources, la logique et la sauvegarde --- */
    if (!demarrer_systemes_jeu(&application)) {
        return 1;
    }

    while (!application.actions.quitter) {
        /* --- vérifier si une sauvegarde peut être reprise --- */
        application.repriseDisponible = fichier_existe(CHEMIN_SAUVEGARDE);

        /* --- traiter l'écran actuellement affiché --- */
        switch (application.ecranActuel) {
            case ECRAN_MENU:
                traiter_ecran_menu(&application);
                break;

            case ECRAN_SAISIE_PSEUDO:
                traiter_ecran_saisie_pseudo(&application);
                break;

            case ECRAN_DECOMPTE:
                traiter_ecran_decompte(&application);
                break;

            case ECRAN_JEU:
                traiter_ecran_jeu(&application);
                break;

            case ECRAN_PARAMETRES:
                traiter_ecran_parametres(&application);
                break;

            case ECRAN_REGLES:
                traiter_ecran_regles(&application);
                break;
        }

        /* --- limiter la boucle de jeu à un rythme stable --- */
        rest(DUREE_UNE_IMAGE_MS);
    }

    /* --- libérer proprement les systèmes du jeu --- */
    fermer_systemes_jeu(&application);

    return 0;
}
END_OF_MAIN();

static void construire_configuration_jeu(ConfigurationJeu *configuration,
                                         const RessourcesJeu *ressources) {
    int indexTailleBulle;

    /* --- calculer les dimensions générales du terrain de jeu --- */
    configuration->largeurFenetre = LARGEUR_FENETRE_JEU;
    configuration->hauteurFenetre = HAUTEUR_FENETRE_JEU;
    configuration->groundY = HAUTEUR_FENETRE_JEU - HAUTEUR_FENETRE_JEU / DIVISEUR_HAUTEUR_SOL;
    configuration->leftLimit = 0;
    configuration->rightLimit = LARGEUR_FENETRE_JEU;

    /* --- adapter la vitesse du joueur à la fenêtre --- */
    configuration->vitesseJoueur = LARGEUR_FENETRE_JEU / DIVISEUR_VITESSE_JOUEUR;
    if (configuration->vitesseJoueur < 1) {
        configuration->vitesseJoueur = 1;
    }

    /* --- recopier les dimensions des sprites du joueur et des bonus --- */
    configuration->joueurLargeur = ressources->player->w;
    configuration->joueurHauteur = ressources->player->h;
    configuration->chapeauLargeur = ressources->chapeau->w;
    configuration->chapeauHauteur = ressources->chapeau->h;
    configuration->explosionLargeur = ressources->explosion->w;
    configuration->explosionHauteur = ressources->explosion->h;

    /* --- dimensionner le tir en gardant une taille minimale lisible --- */
    configuration->projectileLargeur = LARGEUR_FENETRE_JEU / DIVISEUR_LARGEUR_PROJECTILE;
    if (configuration->projectileLargeur < LARGEUR_MIN_PROJECTILE) {
        configuration->projectileLargeur = LARGEUR_MIN_PROJECTILE;
    }

    configuration->projectileHauteur = HAUTEUR_FENETRE_JEU / DIVISEUR_HAUTEUR_PROJECTILE;
    if (configuration->projectileHauteur < HAUTEUR_MIN_PROJECTILE) {
        configuration->projectileHauteur = HAUTEUR_MIN_PROJECTILE;
    }

    configuration->projectileVitesse = HAUTEUR_FENETRE_JEU / DIVISEUR_VITESSE_PROJECTILE;
    if (configuration->projectileVitesse < VITESSE_MIN_PROJECTILE) {
        configuration->projectileVitesse = VITESSE_MIN_PROJECTILE;
    }

    /* --- mémoriser la taille des bulles pour chaque niveau de découpe --- */
    for (indexTailleBulle = 0; indexTailleBulle < BULLE_TAILLES_TOTAL; indexTailleBulle++) {
        configuration->largeurBulles[indexTailleBulle] = ressources->sprites[indexTailleBulle]->w;
        configuration->hauteurBulles[indexTailleBulle] = ressources->sprites[indexTailleBulle]->h;
    }
}

static int fichier_existe(const char *cheminFichier) {
    FILE *fichier;

    /* --- refuser un chemin vide ou nul --- */
    if (!cheminFichier || cheminFichier[0] == '\0') {
        return 0;
    }

    /* --- tenter l'ouverture du fichier en lecture binaire --- */
    fichier = fopen(cheminFichier, "rb");
    if (!fichier) {
        return 0;
    }

    /* --- fermer le fichier immédiatement après le test --- */
    fclose(fichier);
    return 1;
}

static void initialiser_decompte_depart(ApplicationJeu *application) {
    /* --- préparer l'écran de décompte avant d'entrer en jeu --- */
    application->valeurDecompte = VALEUR_DECOMPTE_DEPART;
    application->tempsRestantDecompteMs = DUREE_UNE_SECONDE_MS;
    application->ecranActuel = ECRAN_DECOMPTE;
}

static void vider_pseudo_joueur(ApplicationJeu *application) {
    /* --- repartir d'un pseudo vide pour une nouvelle partie --- */
    application->pseudoJoueur[0] = '\0';
}

static int demarrer_systemes_jeu(ApplicationJeu *application) {
    /* --- ouvrir la fenêtre Allegro --- */
    if (!initialiser_affichage(CHEMIN_FOND_PRINCIPAL,
                               LARGEUR_FENETRE_JEU,
                               HAUTEUR_FENETRE_JEU,
                               PROFONDEUR_COULEUR_JEU)) {
        return 0;
    }

    /* --- charger tous les bitmaps utilisés par le jeu --- */
    if (!charger_ressources_jeu(&application->ressources,
                                CHEMIN_FOND_PRINCIPAL,
                                CHEMIN_IMAGE_JOUEUR,
                                CHEMIN_IMAGE_MANGE_MORT,
                                CHEMIN_IMAGE_VIF_DOR,
                                CHEMIN_IMAGE_TIR,
                                CHEMIN_IMAGE_AURA_ARDENTE,
                                CHEMIN_IMAGE_CHAPEAU,
                                CHEMIN_IMAGE_EXPLOSION)) {
        fermer_affichage();
        return 0;
    }

    /* --- construire la configuration de jeu à partir des sprites chargés --- */
    construire_configuration_jeu(&application->configuration, &application->ressources);

    /* --- initialiser l'état logique de la partie --- */
    if (!initialiser_logique_jeu(&application->etat, &application->configuration)) {
        liberer_ressources_jeu(&application->ressources);
        fermer_affichage();
        return 0;
    }

    /* --- ouvrir le système de sauvegarde --- */
    if (!initialiser_sauvegarde()) {
        fermer_logique_jeu(&application->etat);
        liberer_ressources_jeu(&application->ressources);
        fermer_affichage();
        return 0;
    }

    /* --- préparer les commandes et l'état global de l'application --- */
    initialiser_actions_ihm(&application->actions);
    initialiser_commandes_jeu(&application->commandes);
    application->ecranActuel = ECRAN_MENU;
    application->repriseDisponible = 0;
    application->modeDemonstrationActif = 0;
    application->valeurDecompte = 0;
    application->tempsRestantDecompteMs = 0;
    vider_pseudo_joueur(application);

    return 1;
}

static void fermer_systemes_jeu(ApplicationJeu *application) {
    /* --- fermer les sous-systèmes dans l'ordre inverse de l'initialisation --- */
    fermer_sauvegarde();
    fermer_logique_jeu(&application->etat);
    liberer_ressources_jeu(&application->ressources);
    fermer_affichage();
}

static int lancer_niveau(ApplicationJeu *application, int numeroNiveau) {
    /* --- réinitialiser la partie sur le niveau demandé --- */
    if (!reinitialiser_partie(&application->etat, &application->configuration, numeroNiveau)) {
        return 0;
    }

    /* --- afficher ensuite le décompte de départ --- */
    initialiser_decompte_depart(application);
    return 1;
}

static int charger_partie_et_lancer_decompte(ApplicationJeu *application) {
    /* --- charger la sauvegarde depuis le disque --- */
    if (!charger_etat_jeu(&application->etat, CHEMIN_SAUVEGARDE)) {
        return 0;
    }

    /* --- relancer un décompte avant de rendre la main au joueur --- */
    initialiser_decompte_depart(application);
    return 1;
}

static void traiter_ecran_menu(ApplicationJeu *application) {
    /* --- lire les actions du menu principal --- */
    traiter_ihm_menu(&application->actions, application->repriseDisponible);

    /* --- appliquer le choix du joueur dans le menu --- */
    if (application->actions.nouvellePartie) {
        vider_pseudo_joueur(application);
        application->ecranActuel = ECRAN_SAISIE_PSEUDO;
    } else if (application->actions.reprendrePartie) {
        charger_partie_et_lancer_decompte(application);
    } else if (application->actions.ouvrirParametres) {
        application->ecranActuel = ECRAN_PARAMETRES;
    } else if (application->actions.ouvrirRegles) {
        application->ecranActuel = ECRAN_REGLES;
    }

    /* --- dessiner le menu principal --- */
    dessiner_menu_depart(&application->ressources,
                         application->actions.menuSelection,
                         application->repriseDisponible);
}

static void traiter_ecran_saisie_pseudo(ApplicationJeu *application) {
    /* --- récupérer la saisie clavier du pseudo --- */
    traiter_ihm_saisie_pseudo(&application->actions,
                              application->pseudoJoueur,
                              TAILLE_PSEUDO_MAX);

    /* --- valider le pseudo ou revenir au menu --- */
    if (application->actions.retourMenu) {
        application->ecranActuel = ECRAN_MENU;
    } else if (application->actions.valider) {
        if (lancer_niveau(application, PREMIER_NIVEAU_JEU)) {
            definir_pseudo_joueur(&application->etat, application->pseudoJoueur);
        }
    }

    /* --- dessiner l'écran de saisie du pseudo --- */
    dessiner_saisie_pseudo(&application->ressources, application->pseudoJoueur);
}

static void traiter_ecran_decompte(ApplicationJeu *application) {
    /* --- afficher le chiffre courant du décompte --- */
    dessiner_decompte_depart(&application->ressources,
                             &application->etat,
                             application->valeurDecompte);

    /* --- faire avancer le décompte image par image --- */
    application->tempsRestantDecompteMs -= DUREE_UNE_IMAGE_MS;
    if (application->tempsRestantDecompteMs > 0) {
        return;
    }

    /* --- passer au chiffre suivant ou démarrer le jeu --- */
    application->valeurDecompte--;
    if (application->valeurDecompte <= 0) {
        application->ecranActuel = ECRAN_JEU;
    } else {
        application->tempsRestantDecompteMs = DUREE_UNE_SECONDE_MS;
    }
}

static void traiter_ecran_parametres(ApplicationJeu *application) {
    /* --- lire les actions de l'écran des paramètres --- */
    traiter_ihm_parametres(&application->actions, application->modeDemonstrationActif);

    /* --- activer ou désactiver le mode démonstration --- */
    if (application->actions.basculerModeDemonstration) {
        application->modeDemonstrationActif = !application->modeDemonstrationActif;
        if (!application->modeDemonstrationActif && application->actions.parametresSelection > 1) {
            application->actions.parametresSelection = 0;
        }
    } else if (application->actions.lancerDemoNiveau > 0 && application->modeDemonstrationActif) {
        lancer_niveau(application, application->actions.lancerDemoNiveau);
    } else if (application->actions.retourMenu) {
        application->ecranActuel = ECRAN_MENU;
    }

    /* --- ne rien dessiner si l'écran a changé pendant ce traitement --- */
    if (application->ecranActuel != ECRAN_PARAMETRES) {
        return;
    }

    /* --- dessiner le menu des paramètres --- */
    dessiner_menu_parametres(&application->ressources,
                             application->actions.parametresSelection,
                             application->modeDemonstrationActif);
}

static void traiter_ecran_regles(ApplicationJeu *application) {
    /* --- lire la demande de retour depuis l'écran de règles --- */
    traiter_ihm_ecran_secondaire(&application->actions);
    if (application->actions.retourMenu) {
        application->ecranActuel = ECRAN_MENU;
    }

    /* --- dessiner les règles du jeu --- */
    dessiner_ecran_information(&application->ressources,
                               "REGLES",
                               "Gauche/Droite pour bouger.",
                               "Haut pour tirer sur les bulles.",
                               "S pour sauvegarder, L pour charger.");
}

static void traiter_retour_depuis_le_jeu(ApplicationJeu *application) {
    /* --- passer au niveau suivant si le joueur a gagné --- */
    if (application->etat.gagne) {
        if (application->etat.niveau < application->etat.niveauMaximum) {
            lancer_niveau(application, application->etat.niveau + 1);
        } else {
            application->ecranActuel = ECRAN_MENU;
        }
        return;
    }

    /* --- revenir au menu dans tous les autres cas --- */
    application->ecranActuel = ECRAN_MENU;
}

static void traiter_ecran_jeu(ApplicationJeu *application) {
    /* --- lire les commandes de jeu et geler les entrées si la partie est finie --- */
    traiter_ihm_jeu(&application->actions,
                    &application->commandes,
                    application->etat.perdu || application->etat.gagne);

    /* --- sauvegarder ou charger la partie à la demande --- */
    if (application->actions.sauvegarder) {
        sauvegarder_etat_jeu(&application->etat, CHEMIN_SAUVEGARDE);
    }

    if (application->actions.charger) {
        charger_partie_et_lancer_decompte(application);
    }

    /* --- laisser le décompte reprendre immédiatement après un chargement --- */
    if (application->ecranActuel != ECRAN_JEU) {
        return;
    }

    /* --- gérer le retour au menu ou le passage au niveau suivant --- */
    if (application->actions.retourMenu) {
        traiter_retour_depuis_le_jeu(application);
        return;
    }

    /* --- mettre à jour la logique tant que le joueur ne quitte pas --- */
    if (!application->actions.quitter) {
        mettre_a_jour_logique_jeu(&application->etat,
                                  &application->configuration,
                                  &application->commandes);
    }

    /* --- dessiner la scène de jeu complète --- */
    dessiner_jeu(&application->ressources, &application->etat);
}
