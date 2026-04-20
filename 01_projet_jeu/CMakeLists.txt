cmake_minimum_required(VERSION 3.13)
project(jeu_allegro C)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)

add_executable(jeu_allegro
        src/main.c
        src/affichage.c
        src/logique_jeu.c
        src/IHM.c
        src/sauvegarde.c)

target_include_directories(jeu_allegro PRIVATE
        ${CMAKE_SOURCE_DIR}/allegro-4.4.2/include)

target_link_directories(jeu_allegro PRIVATE /usr/local/lib)
target_link_libraries(jeu_allegro PRIVATE alleg)
