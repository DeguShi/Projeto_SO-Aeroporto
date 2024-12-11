#include "utils.h"
#include <iostream>
#include <algorithm>

// Função para renderizar texto usando SDL_ttf
void renderizarTexto(SDL_Renderer* renderer, TTF_Font* font, const std::string& texto, int x, int y, SDL_Color cor) {
    // Renderiza texto para uma superfície
    SDL_Surface* superficie = TTF_RenderUTF8_Solid(font, texto.c_str(), cor);
    if (!superficie) {
        std::cerr << "Erro ao criar a superfície de texto: " << TTF_GetError() << std::endl;
        return;
    }
    // Cria textura a partir da superfície
    SDL_Texture* textura = SDL_CreateTextureFromSurface(renderer, superficie);
    if (!textura) {
        std::cerr << "Erro ao criar a textura de texto: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(superficie);
        return;
    }
    // Define o retângulo de destino
    SDL_Rect retanguloDestino = { x, y, superficie->w, superficie->h };
    // Copia a textura para o renderer
    SDL_RenderCopy(renderer, textura, NULL, &retanguloDestino);
    // Limpeza
    SDL_FreeSurface(superficie);
    SDL_DestroyTexture(textura);
}

// Função para verificar se o mouse está dentro de um retângulo
bool mouse_noRetangulo(int mouseX, int mouseY, const SDL_Rect& retangulo) {
    return mouseX >= retangulo.x && mouseX <= (retangulo.x + retangulo.w) &&
           mouseY >= retangulo.y && mouseY <= (retangulo.y + retangulo.h);
}

// Função para preencher triângulos manualmente com orientação
void preencherTriangulo(SDL_Renderer* renderer, Sint16 x, Sint16 y, float tamanho, OrientacaoTriangulo orientacao, SDL_Color cor) {
    Sint16 x1, y1, x2, y2, x3, y3;

    switch(orientacao) {
        case OrientacaoTriangulo::DIREITA:
            x1 = x;
            y1 = y;
            x2 = x - tamanho;
            y2 = y - tamanho / 2;
            x3 = x - tamanho;
            y3 = y + tamanho / 2;
            break;
        case OrientacaoTriangulo::ESQUERDA:
            x1 = x;
            y1 = y;
            x2 = x + tamanho;
            y2 = y - tamanho / 2;
            x3 = x + tamanho;
            y3 = y + tamanho / 2;
            break;
        case OrientacaoTriangulo::BAIXO:
            x1 = x;
            y1 = y;
            x2 = x - tamanho / 2;
            y2 = y - tamanho;
            x3 = x + tamanho / 2;
            y3 = y - tamanho;
            break;
        case OrientacaoTriangulo::CIMA:
            x1 = x;
            y1 = y;
            x2 = x - tamanho / 2;
            y2 = y + tamanho;
            x3 = x + tamanho / 2;
            y3 = y + tamanho;
            break;
        default:
            // Padrão para orientação direita
            x1 = x;
            y1 = y;
            x2 = x - tamanho;
            y2 = y - tamanho / 2;
            x3 = x - tamanho;
            y3 = y + tamanho / 2;
            break;
    }

    // Ordena os vértices por coordenada y crescente (y1 <= y2 <= y3)
    if (y2 < y1) { std::swap(x1, x2); std::swap(y1, y2); }
    if (y3 < y1) { std::swap(x1, x3); std::swap(y1, y3); }
    if (y3 < y2) { std::swap(x2, x3); std::swap(y2, y3); }

    // Calcula as inclinações
    float dx1 = 0, dx2 = 0, dx3 = 0;
    if (y3 - y1 != 0) dx1 = static_cast<float>(x3 - x1) / (y3 - y1);
    if (y2 - y1 != 0) dx2 = static_cast<float>(x2 - x1) / (y2 - y1);
    if (y3 - y2 != 0) dx3 = static_cast<float>(x3 - x2) / (y3 - y2);

    float atualX1 = x1;
    float atualX2 = x1;

    // Desenha a primeira parte do triângulo
    for (int y_coord = y1; y_coord <= y2; y_coord++) {
        SDL_SetRenderDrawColor(renderer, cor.r, cor.g, cor.b, cor.a);
        SDL_RenderDrawLine(renderer, static_cast<int>(atualX1), y_coord, static_cast<int>(atualX2), y_coord);
        atualX1 += dx1;
        atualX2 += dx2;
    }

    atualX1 = x2;

    // Desenha a segunda parte do triângulo
    for (int y_coord = y2; y_coord <= y3; y_coord++) {
        SDL_SetRenderDrawColor(renderer, cor.r, cor.g, cor.b, cor.a);
        SDL_RenderDrawLine(renderer, static_cast<int>(atualX1), y_coord, static_cast<int>(atualX2), y_coord);
        atualX1 += dx3;
        atualX2 += dx1;
    }
}
