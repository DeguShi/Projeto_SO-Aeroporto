// pista.h
#ifndef PISTA_H
#define PISTA_H

#include <SDL.h>

// Estrutura para representar uma pista
struct Runway {
    int id;                 // Identificador da pista
    SDL_Rect rect;          // Posição e tamanho da pista
    bool occupied;          // Status de ocupação

    Runway(int runway_id, SDL_Rect runway_rect)
        : id(runway_id), rect(runway_rect), occupied(false) {}
};

#endif // PISTA_H
