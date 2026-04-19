cmake_minimum_required(VERSION 3.13)
project(jeu_allegro C)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-L/usr/local/lib -lalleg")

add_executable(jeu_allegro
        01_projet_jeu/src/main.c
        01_projet_jeu/src/affichage.c
        01_projet_jeu/src/logique_jeu.c
        01_projet_jeu/src/IHM.c
        01_projet_jeu/src/sauvegarde.c)
