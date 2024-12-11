#ifndef AVIAO_H
#define AVIAO_H

#include <SDL.h>
#include <string>
#include <vector>
#include <mutex>
#include "semaforo.h"
#include "utils.h"
#include "pista.h"

// Estrutura para armazenar dados do avião
struct Plane {
    int id;
    float x, y;            // Posição atual
    SDL_Color cor;
    bool movendoParaDireita;     // Direção do movimento: true para direita, false para esquerda
    bool aterrissou;          // Indica se o avião aterrissou
    int pistaAtribuida;   // ID da pista atribuída
};

// Estrutura para passar dados para threads dos aviões
struct AviaoData {
    int id;
    int tempo_pouso;
    int tempo_desembarque;
    int tempo_decolagem;
    Semaforo* pistas;           // Ponteiro para semáforo
    std::vector<Plane>* avioes;  // Ponteiro para lista compartilhada de aviões
    std::vector<Runway>* pistasList; // Ponteiro para lista compartilhada de pistas
    std::mutex* mutex_avioes;    // Ponteiro para mutex dos aviões
    int atraso_inicio;             // Atraso antes de iniciar a simulação
};

// Função de simulação do avião
void* aviao(void* data);

#endif
