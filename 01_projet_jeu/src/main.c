#include "affichage.h"
#include "IHM.h"
#include "logique_jeu.h"
#include "sauvegarde.h"

#include <stdio.h>
#include <string.h>

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
    ECRAN_SELECTION_NIVEAU,
    ECRAN_SELECTION_SAUVEGARDE
} EcranActif;

typedef struct {
    RessourcesJeu ressources;
    ConfigurationJeu configuration;
    EtatJeu etatJeu;
    ActionsIHM actions;
    CommandesJeu commandes;
    EcranActif ecranActuel;
    int repriseDisponible;
    int modeDemonstrationActif;
    int niveauMaximumDebloque;
    int valeurDecompte;
    int tempsRestantDecompteMs;
    int sauvegardeActuelle;
    int nombreSauvegardesDisponibles;
    int sauvegardesDisponibles[NOMBRE_SLOTS_SAUVEGARDE];
    char pseudosSauvegardes[NOMBRE_SLOTS_SAUVEGARDE][TAILLE_PSEUDO_MAX];
    char pseudoJoueur[TAILLE_PSEUDO_MAX];
} ApplicationJeu;

static void construire_configuration_jeu(ConfigurationJeu *configuration,
                                         const RessourcesJeu *ressources);
static void initialiser_decompte_depart(ApplicationJeu *application);
static void vider_pseudo_joueur(ApplicationJeu *application);
static int demarrer_systemes_jeu(ApplicationJeu *application);
static void fermer_systemes_jeu(ApplicationJeu *application);
static int lancer_niveau(ApplicationJeu *application, int numeroNiveau);
static int construire_chemin_sauvegarde_slot(char *cheminSauvegarde, size_t tailleCheminSauvegarde, int indexSlot);
static void actualiser_sauvegardes(ApplicationJeu *application);
static int trouver_slot_sauvegarde_pour_pseudo(ApplicationJeu *application, const char *pseudoJoueur);
static int partie_en_cours_sauvegardable(const ApplicationJeu *application);
static int sauvegarder_partie_courante(ApplicationJeu *application);
static int charger_partie_et_lancer_decompte(ApplicationJeu *application, int indexSlot);
static void traiter_ecran_menu(ApplicationJeu *application);
static void traiter_ecran_saisie_pseudo(ApplicationJeu *application);
static void traiter_ecran_decompte(ApplicationJeu *application);
static void traiter_ecran_parametres(ApplicationJeu *application);
static void traiter_ecran_regles(ApplicationJeu *application);
static void traiter_ecran_selection_niveau(ApplicationJeu *application);
static void traiter_ecran_selection_sauvegarde(ApplicationJeu *application);
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
        actualiser_sauvegardes(&application);
        application.repriseDisponible = application.nombreSauvegardesDisponibles > VALEUR_NULLE;

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

            case ECRAN_SELECTION_SAUVEGARDE:
                traiter_ecran_selection_sauvegarde(&application);
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
    configuration->positionSolY = hauteurFenetre - dimension_relative(hauteurFenetre, DIVISEUR_HAUTEUR_SOL);
    configuration->limiteGaucheTerrain = VALEUR_NULLE;
    configuration->limiteDroiteTerrain = largeurFenetre;

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
    int indexSauvegarde;

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
    if (!initialiser_logique_jeu(&application->etatJeu, &application->configuration)) {
        liberer_ressources_jeu(&application->ressources);
        fermer_affichage();
        return FAUX;
    }

    /* --- ouvrir le système de sauvegarde --- */
    if (!initialiser_sauvegarde()) {
        fermer_logique_jeu(&application->etatJeu);
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
    application->sauvegardeActuelle = INDEX_INVALIDE;
    application->nombreSauvegardesDisponibles = VALEUR_NULLE;
    for (indexSauvegarde = INDEX_PREMIER; indexSauvegarde < NOMBRE_SLOTS_SAUVEGARDE; indexSauvegarde++) {
        application->sauvegardesDisponibles[indexSauvegarde] = FAUX;
        application->pseudosSauvegardes[indexSauvegarde][CHAINE_DEBUT] = CARACTERE_FIN_CHAINE;
    }
    vider_pseudo_joueur(application);
    actualiser_sauvegardes(application);

    return VRAI;
}

static void fermer_systemes_jeu(ApplicationJeu *application) {
    /* --- fermer les sous-systèmes dans l'ordre inverse de l'initialisation --- */
    if (partie_en_cours_sauvegardable(application)) {
        sauvegarder_partie_courante(application);
    }
    fermer_sauvegarde();
    fermer_logique_jeu(&application->etatJeu);
    liberer_ressources_jeu(&application->ressources);
    fermer_affichage();
}

static int lancer_niveau(ApplicationJeu *application, int numeroNiveau) {
    /* --- réinitialiser la partie sur le niveau demandé --- */
    if (!reinitialiser_partie(&application->etatJeu, &application->configuration, numeroNiveau)) {
        return FAUX;
    }

    /* --- afficher ensuite le décompte de départ --- */
    initialiser_decompte_depart(application);
    return VRAI;
}

static int construire_chemin_sauvegarde_slot(char *cheminSauvegarde, size_t tailleCheminSauvegarde, int indexSlot) {
    if (!cheminSauvegarde || tailleCheminSauvegarde == VALEUR_NULLE ||
        indexSlot < INDEX_PREMIER || indexSlot >= NOMBRE_SLOTS_SAUVEGARDE) {
        return FAUX;
    }

    return snprintf(cheminSauvegarde,
                    tailleCheminSauvegarde,
                    FORMAT_CHEMIN_SAUVEGARDE_SLOT,
                    indexSlot + INDEX_SUIVANT) < (int) tailleCheminSauvegarde;
}

static void actualiser_sauvegardes(ApplicationJeu *application) {
    int indexSauvegarde;
    char cheminSauvegarde[TAILLE_CHEMIN_RESSOURCE];

    if (!application) {
        return;
    }

    application->nombreSauvegardesDisponibles = VALEUR_NULLE;
    for (indexSauvegarde = INDEX_PREMIER; indexSauvegarde < NOMBRE_SLOTS_SAUVEGARDE; indexSauvegarde++) {
        application->sauvegardesDisponibles[indexSauvegarde] = FAUX;
        application->pseudosSauvegardes[indexSauvegarde][CHAINE_DEBUT] = CARACTERE_FIN_CHAINE;

        if (construire_chemin_sauvegarde_slot(cheminSauvegarde, sizeof(cheminSauvegarde), indexSauvegarde) &&
            charger_pseudo_sauvegarde(cheminSauvegarde,
                                      application->pseudosSauvegardes[indexSauvegarde],
                                      TAILLE_PSEUDO_MAX)) {
            application->sauvegardesDisponibles[indexSauvegarde] = VRAI;
            application->nombreSauvegardesDisponibles++;
        }
    }
}

static int trouver_slot_sauvegarde_pour_pseudo(ApplicationJeu *application, const char *pseudoJoueur) {
    int indexSauvegarde;

    if (!application || !pseudoJoueur || pseudoJoueur[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE) {
        return INDEX_INVALIDE;
    }

    actualiser_sauvegardes(application);
    for (indexSauvegarde = INDEX_PREMIER; indexSauvegarde < NOMBRE_SLOTS_SAUVEGARDE; indexSauvegarde++) {
        if (application->sauvegardesDisponibles[indexSauvegarde] &&
            strcmp(application->pseudosSauvegardes[indexSauvegarde], pseudoJoueur) == VALEUR_NULLE) {
            return indexSauvegarde;
        }
    }

    for (indexSauvegarde = INDEX_PREMIER; indexSauvegarde < NOMBRE_SLOTS_SAUVEGARDE; indexSauvegarde++) {
        if (!application->sauvegardesDisponibles[indexSauvegarde]) {
            return indexSauvegarde;
        }
    }

    return INDEX_PREMIER;
}

static int partie_en_cours_sauvegardable(const ApplicationJeu *application) {
    if (!application) {
        return FAUX;
    }

    if (application->ecranActuel != ECRAN_JEU &&
        application->ecranActuel != ECRAN_DECOMPTE) {
        return FAUX;
    }

    return application->etatJeu.pseudoJoueur[CHAINE_DEBUT] != CARACTERE_FIN_CHAINE &&
           !application->etatJeu.partiePerdue &&
           !application->etatJeu.partieGagnee &&
           application->etatJeu.tempsRestantNiveauMs > VALEUR_NULLE &&
           application->etatJeu.nombreBulles > VALEUR_NULLE;
}

static int sauvegarder_partie_courante(ApplicationJeu *application) {
    char cheminSauvegarde[TAILLE_CHEMIN_RESSOURCE];
    int indexSauvegarde;

    if (!application ||
        application->etatJeu.pseudoJoueur[CHAINE_DEBUT] == CARACTERE_FIN_CHAINE ||
        application->etatJeu.partiePerdue ||
        application->etatJeu.partieGagnee) {
        return FAUX;
    }

    indexSauvegarde = trouver_slot_sauvegarde_pour_pseudo(application, application->etatJeu.pseudoJoueur);
    if (indexSauvegarde == INDEX_INVALIDE ||
        !construire_chemin_sauvegarde_slot(cheminSauvegarde, sizeof(cheminSauvegarde), indexSauvegarde)) {
        return FAUX;
    }

    if (!sauvegarder_etat_jeu(&application->etatJeu, cheminSauvegarde)) {
        return FAUX;
    }

    application->sauvegardeActuelle = indexSauvegarde;
    actualiser_sauvegardes(application);
    return VRAI;
}

static int charger_partie_et_lancer_decompte(ApplicationJeu *application, int indexSlot) {
    char cheminSauvegarde[TAILLE_CHEMIN_RESSOURCE];

    if (!construire_chemin_sauvegarde_slot(cheminSauvegarde, sizeof(cheminSauvegarde), indexSlot)) {
        return FAUX;
    }

    /* --- charger la sauvegarde depuis le disque --- */
    if (!charger_etat_jeu(&application->etatJeu, cheminSauvegarde)) {
        return FAUX;
    }
    if (application->etatJeu.niveauActuel > application->niveauMaximumDebloque) {
        application->niveauMaximumDebloque = application->etatJeu.niveauActuel;
    }
    application->sauvegardeActuelle = indexSlot;
    strncpy(application->pseudoJoueur, application->etatJeu.pseudoJoueur, TAILLE_PSEUDO_MAX - INDEX_SUIVANT);
    application->pseudoJoueur[TAILLE_PSEUDO_MAX - INDEX_SUIVANT] = CARACTERE_FIN_CHAINE;

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
        actualiser_sauvegardes(application);
        if (application->nombreSauvegardesDisponibles > VALEUR_NULLE) {
            application->actions.sauvegardeSelection = INDEX_PREMIER;
            application->ecranActuel = ECRAN_SELECTION_SAUVEGARDE;
        }
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
            definir_pseudo_joueur(&application->etatJeu, application->pseudoJoueur);
            application->sauvegardeActuelle = trouver_slot_sauvegarde_pour_pseudo(application, application->pseudoJoueur);
        }
    }

    /* --- dessiner l'écran de saisie du pseudo --- */
    dessiner_saisie_pseudo(&application->ressources, application->pseudoJoueur);
}

static void traiter_ecran_decompte(ApplicationJeu *application) {
    /* --- afficher le chiffre courant du décompte --- */
    dessiner_decompte_depart(&application->ressources,
                             &application->etatJeu,
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
                              application->etatJeu.niveauMaximum);
}

static void traiter_ecran_selection_sauvegarde(ApplicationJeu *application) {
    actualiser_sauvegardes(application);
    traiter_ihm_selection_sauvegarde(&application->actions,
                                      application->sauvegardesDisponibles,
                                      NOMBRE_SLOTS_SAUVEGARDE);

    if (application->actions.sauvegardeChoisie != INDEX_INVALIDE) {
        charger_partie_et_lancer_decompte(application, application->actions.sauvegardeChoisie);
    } else if (application->actions.retourMenu) {
        application->ecranActuel = ECRAN_MENU;
    }

    if (application->ecranActuel != ECRAN_SELECTION_SAUVEGARDE) {
        return;
    }

    dessiner_selection_sauvegarde(&application->ressources,
                                  application->actions.sauvegardeSelection,
                                  application->pseudosSauvegardes,
                                  application->sauvegardesDisponibles);
}

static void traiter_retour_depuis_le_jeu(ApplicationJeu *application) {
    if (partie_en_cours_sauvegardable(application)) {
        sauvegarder_partie_courante(application);
    }

    /* --- revenir au choix de niveau après une victoire --- */
    if (application->etatJeu.partieGagnee) {
        if (application->etatJeu.niveauActuel < application->etatJeu.niveauMaximum) {
            if (application->etatJeu.niveauActuel + INDEX_SUIVANT > application->niveauMaximumDebloque) {
                application->niveauMaximumDebloque = application->etatJeu.niveauActuel + INDEX_SUIVANT;
            }
            application->actions.niveauSelection = application->etatJeu.niveauActuel;
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
                    application->etatJeu.partiePerdue || application->etatJeu.partieGagnee);

    /* --- sauvegarder ou charger la partie à la demande --- */
    if (application->actions.sauvegarder) {
        sauvegarder_partie_courante(application);
    }

    if (application->actions.charger) {
        if (application->sauvegardeActuelle != INDEX_INVALIDE) {
            charger_partie_et_lancer_decompte(application, application->sauvegardeActuelle);
        } else {
            application->ecranActuel = ECRAN_SELECTION_SAUVEGARDE;
        }
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
        mettre_a_jour_logique_jeu(&application->etatJeu,
                                  &application->configuration,
                                  &application->commandes);
    } else {
        sauvegarder_partie_courante(application);
    }

    /* --- dessiner la scène de jeu complète --- */
    dessiner_jeu(&application->ressources, &application->etatJeu);
}
