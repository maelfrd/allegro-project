cmake_minimum_required(VERSION 3.13)
project(jeu_allegro C)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(DOSSIER_JEU "${CMAKE_SOURCE_DIR}/01_projet_jeu")
set(DOSSIER_SOURCES "${DOSSIER_JEU}/src")
set(DOSSIER_ASSETS "${DOSSIER_JEU}/assets")
set(FICHIER_SAUVEGARDE "${DOSSIER_JEU}/sauvegardes/savegame.dat")

add_executable(jeu_allegro
        ${DOSSIER_SOURCES}/main.c
        ${DOSSIER_SOURCES}/affichage.c
        ${DOSSIER_SOURCES}/logique_jeu.c
        ${DOSSIER_SOURCES}/IHM.c
        ${DOSSIER_SOURCES}/sauvegarde.c)

target_include_directories(jeu_allegro PRIVATE ${DOSSIER_SOURCES})
target_compile_definitions(jeu_allegro PRIVATE
        JEU_ASSETS_DIR="${DOSSIER_ASSETS}"
        JEU_SAVE_FILE="${FICHIER_SAUVEGARDE}")
target_link_directories(jeu_allegro PRIVATE /usr/local/lib)
target_link_libraries(jeu_allegro PRIVATE alleg)
