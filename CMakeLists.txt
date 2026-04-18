cmake_minimum_required(VERSION 3.13)
project(jeu_allegro C)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-L/usr/local/lib -lalleg")

add_executable(jeu_allegro
        cmake-build-debug-docker/src/main.c
        cmake-build-debug-docker/src/affichage.c
        cmake-build-debug-docker/src/logique_jeu.c
        cmake-build-debug-docker/src/IHM.c
        cmake-build-debug-docker/src/sauvegarde.c)
