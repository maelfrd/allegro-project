cmake_minimum_required(VERSION 3.13)
project(jeu_allegro C)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
    set(PROJET_JEU_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
else()
    set(PROJET_JEU_DIR "${CMAKE_CURRENT_SOURCE_DIR}/01_projet_jeu")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/allegro-4.4.2/include")
    set(ALLEGRO_LOCAL_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/allegro-4.4.2/include")
else()
    set(ALLEGRO_LOCAL_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../allegro-4.4.2/include")
endif()

add_executable(jeu_allegro
        "${PROJET_JEU_DIR}/src/main.c"
        "${PROJET_JEU_DIR}/src/affichage.c"
        "${PROJET_JEU_DIR}/src/logique_jeu.c"
        "${PROJET_JEU_DIR}/src/IHM.c"
        "${PROJET_JEU_DIR}/src/sauvegarde.c")

target_include_directories(jeu_allegro PRIVATE
        "${PROJET_JEU_DIR}/src"
        "${ALLEGRO_LOCAL_INCLUDE}")

target_link_directories(jeu_allegro PRIVATE /usr/local/lib)
target_link_libraries(jeu_allegro PRIVATE alleg)
