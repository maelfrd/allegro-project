#include "affichage.h"
#include "IHM.h"
#include "logique_jeu.h"
#include "sauvegarde.h"

#include <stdio.h>

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
    ECRAN_MENU,
    ECRAN_SAISIE_PSEUDO,
    ECRAN_DECOMPTE,
    ECRAN_JEU,
    ECRAN_PARAMETRES,
    ECRAN_REGLES,
    ECRAN_SELECTION_NIVEAU
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
    int niveauMaximumDebloque;
    int valeurDecompte;
    int tempsRestantDecompteMs;
    char pseudoJoueur[TAILLE_PSEUDO_MAX];
} ApplicationJeu;

static void construire_configuration_jeu(ConfigurationJeu *configuration,
                                         const RessourcesJeu *ressources);
static int dimension_relative(int reference, int diviseur);
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
static void traiter_ecran_selection_niveau(ApplicationJeu *application);
static void traiter_retour_depuis_le_jeu(ApplicationJeu *application);
static void traiter_ecran_jeu(ApplicationJeu *application);

int main(void) {
    ApplicationJeu application;

    /* --- initialiser Allegro, les ressources, la logique et la sauvegarde --- */
    if (!demarrer_systemes_jeu(&application)) {
        return RETOUR_PROGRAMME_ERREUR;
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

            case ECRAN_SELECTION_NIVEAU:
                traiter_ecran_selection_niveau(&application);
                break;
        }

        /* --- limiter la boucle de jeu à un rythme stable --- */
        rest(DUREE_UNE_IMAGE_MS);
    }

    /* --- libérer proprement les systèmes du jeu --- */
    fermer_systemes_jeu(&application);

    return RETOUR_PROGRAMME_OK;
}
END_OF_MAIN();

static void construire_configuration_jeu(ConfigurationJeu *configuration,
                                         const RessourcesJeu *ressources) {
    int indexTailleBulle;
    int largeurFenetre = screen ? SCREEN_W : VALEUR_NULLE;
    int hauteurFenetre = screen ? SCREEN_H : VALEUR_NULLE;

    /* --- calculer les dimensions générales du terrain de jeu --- */
    configuration->largeurFenetre = largeurFenetre;
    configuration->hauteurFenetre = hauteurFenetre;
    configuration->groundY = hauteurFenetre - dimension_relative(hauteurFenetre, DIVISEUR_HAUTEUR_SOL);
    configuration->leftLimit = VALEUR_NULLE;
    configuration->rightLimit = largeurFenetre;

    /* --- adapter la vitesse du joueur à la fenêtre --- */
    configuration->vitesseJoueur = dimension_relative(largeurFenetre, DIVISEUR_VITESSE_JOUEUR);
    if (configuration->vitesseJoueur < VALEUR_UNITAIRE) {
        configuration->vitesseJoueur = VALEUR_UNITAIRE;
    }

    /* --- recopier les dimensions des sprites du joueur et des bonus --- */
    configuration->joueurLargeur = ressources->player->w;
    configuration->joueurHauteur = ressources->player->h;
    configuration->chapeauLargeur = ressources->chapeau->w;
    configuration->chapeauHauteur = ressources->chapeau->h;
    configuration->explosionLargeur = ressources->explosion->w;
    configuration->explosionHauteur = ressources->explosion->h;

    /* --- dimensionner le tir en gardant une taille minimale lisible --- */
    configuration->projectileLargeur = dimension_relative(largeurFenetre, DIVISEUR_LARGEUR_PROJECTILE);
    configuration->projectileHauteur = dimension_relative(hauteurFenetre, DIVISEUR_HAUTEUR_PROJECTILE);
    configuration->projectileVitesse = dimension_relative(hauteurFenetre, DIVISEUR_VITESSE_PROJECTILE);

    /* --- mémoriser la taille des bulles pour chaque niveau de découpe --- */
    for (indexTailleBulle = INDEX_PREMIER; indexTailleBulle < BULLE_TAILLES_TOTAL; indexTailleBulle++) {
        configuration->largeurBulles[indexTailleBulle] = ressources->sprites[indexTailleBulle]->w;
        configuration->hauteurBulles[indexTailleBulle] = ressources->sprites[indexTailleBulle]->h;
    }
}

static int dimension_relative(int reference, int diviseur) {
    int valeur;

    if (reference <= VALEUR_NULLE || diviseur <= VALEUR_NULLE) {
        return VALEUR_NULLE;
    }

    valeur = reference / diviseur;
    if (valeur <= VALEUR_NULLE) {
        valeur = reference / reference;
    }

    return valeur;
}

static int fichier_existe(const char *cheminFichier) {
    FILE *fichier;

    /* --- refuser un chemin vide ou nul --- */
    if (!cheminFichier || cheminFichier[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE) {
        return FAUX;
    }

    /* --- tenter l'ouverture du fichier en lecture binaire --- */
    fichier = fopen(cheminFichier, "rb");
    if (!fichier) {
        return FAUX;
    }

    /* --- fermer le fichier immédiatement après le test --- */
    fclose(fichier);
    return VRAI;
}

static void initialiser_decompte_depart(ApplicationJeu *application) {
    /* --- préparer l'écran de décompte avant d'entrer en jeu --- */
    application->valeurDecompte = VALEUR_DECOMPTE_DEPART;
    application->tempsRestantDecompteMs = DUREE_UNE_SECONDE_MS;
    application->ecranActuel = ECRAN_DECOMPTE;
}

static void vider_pseudo_joueur(ApplicationJeu *application) {
    /* --- repartir d'un pseudo vide pour une nouvelle partie --- */
    application->pseudoJoueur[CHAINE_DEBUT] = CARACTERE_FIN_CHAINE;
}

static int demarrer_systemes_jeu(ApplicationJeu *application) {
    /* --- ouvrir la fenêtre Allegro --- */
    if (!initialiser_affichage(CHEMIN_FOND_PRINCIPAL,
                               PROFONDEUR_COULEUR_JEU)) {
        return FAUX;
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
        return FAUX;
    }

    /* --- construire la configuration de jeu à partir des sprites chargés --- */
    construire_configuration_jeu(&application->configuration, &application->ressources);

    /* --- initialiser l'état logique de la partie --- */
    if (!initialiser_logique_jeu(&application->etat, &application->configuration)) {
        liberer_ressources_jeu(&application->ressources);
        fermer_affichage();
        return FAUX;
    }

    /* --- ouvrir le système de sauvegarde --- */
    if (!initialiser_sauvegarde()) {
        fermer_logique_jeu(&application->etat);
        liberer_ressources_jeu(&application->ressources);
        fermer_affichage();
        return FAUX;
    }

    /* --- préparer les commandes et l'état global de l'application --- */
    initialiser_actions_ihm(&application->actions);
    initialiser_commandes_jeu(&application->commandes);
    application->ecranActuel = ECRAN_MENU;
    application->repriseDisponible = FAUX;
    application->modeDemonstrationActif = FAUX;
    application->niveauMaximumDebloque = PREMIER_NIVEAU_JEU;
    application->valeurDecompte = VALEUR_NULLE;
    application->tempsRestantDecompteMs = VALEUR_NULLE;
    vider_pseudo_joueur(application);

    return VRAI;
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
        return FAUX;
    }

    /* --- afficher ensuite le décompte de départ --- */
    initialiser_decompte_depart(application);
    return VRAI;
}

static int charger_partie_et_lancer_decompte(ApplicationJeu *application) {
    /* --- charger la sauvegarde depuis le disque --- */
    if (!charger_etat_jeu(&application->etat, CHEMIN_SAUVEGARDE)) {
        return FAUX;
    }
    if (application->etat.niveau > application->niveauMaximumDebloque) {
        application->niveauMaximumDebloque = application->etat.niveau;
    }

    /* --- relancer un décompte avant de rendre la main au joueur --- */
    initialiser_decompte_depart(application);
    return VRAI;
}

static void traiter_ecran_menu(ApplicationJeu *application) {
    /* --- lire les actions du menu principal --- */
    traiter_ihm_menu(&application->actions, application->repriseDisponible);

    /* --- appliquer le choix du joueur dans le menu --- */
    if (application->actions.nouvellePartie) {
        vider_pseudo_joueur(application);
        application->niveauMaximumDebloque = PREMIER_NIVEAU_JEU;
        application->actions.niveauSelection = INDEX_PREMIER;
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
    if (application->tempsRestantDecompteMs > VALEUR_NULLE) {
        return;
    }

    /* --- passer au chiffre suivant ou démarrer le jeu --- */
    application->valeurDecompte--;
    if (application->valeurDecompte <= VALEUR_NULLE) {
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
        if (!application->modeDemonstrationActif && application->actions.parametresSelection > PARAM_SELECTION_RETOUR_SIMPLE) {
            application->actions.parametresSelection = INDEX_PREMIER;
        }
    } else if (application->actions.lancerDemoNiveau > VALEUR_NULLE && application->modeDemonstrationActif) {
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

static void traiter_ecran_selection_niveau(ApplicationJeu *application) {
    /* --- lire le choix du niveau après un retour depuis la fin d'un niveau --- */
    traiter_ihm_selection_niveau(&application->actions,
                                 application->niveauMaximumDebloque);

    if (application->actions.niveauChoisi > VALEUR_NULLE) {
        lancer_niveau(application, application->actions.niveauChoisi);
    } else if (application->actions.retourMenu) {
        application->ecranActuel = ECRAN_MENU;
    }

    if (application->ecranActuel != ECRAN_SELECTION_NIVEAU) {
        return;
    }

    dessiner_selection_niveau(&application->ressources,
                              application->actions.niveauSelection,
                              application->niveauMaximumDebloque,
                              application->etat.niveauMaximum);
}

static void traiter_retour_depuis_le_jeu(ApplicationJeu *application) {
    /* --- revenir au choix de niveau après une victoire --- */
    if (application->etat.gagne) {
        if (application->etat.niveau < application->etat.niveauMaximum) {
            if (application->etat.niveau + INDEX_SUIVANT > application->niveauMaximumDebloque) {
                application->niveauMaximumDebloque = application->etat.niveau + INDEX_SUIVANT;
            }
            application->actions.niveauSelection = application->etat.niveau;
            application->ecranActuel = ECRAN_SELECTION_NIVEAU;
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
