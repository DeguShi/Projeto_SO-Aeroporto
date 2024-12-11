#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

// Função para renderizar texto
void renderizarTexto(SDL_Renderer* renderer, TTF_Font* font, const std::string& texto, int x, int y, SDL_Color cor);

// Função para verificar se o mouse está dentro de um retângulo
bool mouse_noRetangulo(int mouseX, int mouseY, const SDL_Rect& retangulo);

// Enum para orientação do triângulo
enum class OrientacaoTriangulo {
    ESQUERDA,
    DIREITA,
    BAIXO, // Para aviões aterrissados
    CIMA   // Para aviões decolando
};

// Função para preencher triângulos manualmente com orientação
void preencherTriangulo(SDL_Renderer* renderer, Sint16 x, Sint16 y, float tamanho, OrientacaoTriangulo orientacao, SDL_Color cor);

#endif 
