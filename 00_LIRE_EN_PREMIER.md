# Organisation du projet

Le projet a ete remis dans une structure compatible avec `Debug-Docker`.

## Fichiers et dossiers importants

- `CMakeLists.txt` : configuration principale du projet.
- `Dockerfile` : configuration Docker.
- `allegro-4.4.2` : bibliotheque Allegro.
- `cmake-build-debug-docker/src` : code source principal du jeu.
- `cmake-build-debug-docker/assets` : images du jeu.
- `cmake-build-debug-docker/savegame.dat` : fichier de sauvegarde cree pendant l'execution.

## Par ou commencer

1. Ouvre `CMakeLists.txt`.
2. Modifie surtout les fichiers dans `cmake-build-debug-docker/src`.
3. Garde les images dans `cmake-build-debug-docker/assets`.
4. Utilise `Debug-Docker` comme avant dans CLion.

## Remarque importante

Le dossier `cmake-build-debug-docker` contient a la fois le code du projet et les fichiers de build. Ce n'est pas ideal, mais c'est cette organisation qui correspond a ton environnement Docker actuel et qui evite de casser le debug.
