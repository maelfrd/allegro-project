# Organisation du dossier

Ce dossier a ete reordonne pour separer clairement le jeu, la bibliotheque Allegro, Docker et les anciens fichiers de build.

## Dossiers utiles

- `01_projet_jeu` : ton vrai projet de jeu.
- `02_bibliotheque_allegro` : le code source de la bibliotheque Allegro 4.4.2.
- `03_configuration_docker` : le `Dockerfile`.
- `99_build_temporaire` : anciens builds, fichiers generes par CLion, objets `.o`, executable et essais a ne pas modifier en priorite.

## Dans `01_projet_jeu`

- `src` : le code C principal.
- `assets` : les images du jeu.
- `sauvegardes` : les fichiers de sauvegarde.
- `archives_code` : anciens fichiers gardes de cote pour ne rien perdre.

## Par ou commencer

1. Lire `CMakeLists.txt` a la racine.
2. Travailler dans `01_projet_jeu/src`.
3. Utiliser `01_projet_jeu/assets` pour les images.
4. Ne toucher a `99_build_temporaire` qu'en cas de besoin precis.

## Remarques

- Le projet CMake pointe maintenant vers `01_projet_jeu/src`.
- Le jeu charge maintenant ses assets depuis `01_projet_jeu/assets`.
- Le `Dockerfile` a ete deplace dans `03_configuration_docker`.
