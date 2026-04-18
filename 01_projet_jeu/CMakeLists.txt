cmake_minimum_required(VERSION 3.13)
project(jeu_allegro C)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-L/usr/local/lib -lalleg")

add_executable(jeu_allegro
        src/main.c
        src/affichage.c
        src/logique_jeu.c
        src/IHM.c
        src/sauvegarde.c)
