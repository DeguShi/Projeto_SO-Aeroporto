#include "aviao.h"
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>

// Define limites da tela para o movimento do avião
constexpr float LIMITE_ESQUERDO = 50.0f;
constexpr float LIMITE_DIREITO = 750.0f; // Supondo LARGURA_JANELA = 800
constexpr float VELOCIDADE_MOVIMENTO = 1.0f; // Pixels por atualização
constexpr int ATRASO_MOVIMENTO_MS = 16;   // Aproximadamente 60 FPS

// Define possíveis alturas de spawn (coordenadas y)
constexpr float POSICOES_Y_SPAWN[4] = {50.0f, 70.0f, 90.0f, 30.0f};

// Função de simulação do avião
void* aviao(void* data) {
    AviaoData* dados_aviao = static_cast<AviaoData*>(data);

    int aviao_id = dados_aviao->id;
    int tempo_pouso = dados_aviao->tempo_pouso;
    int tempo_desembarque = dados_aviao->tempo_desembarque;
    int tempo_decolagem = dados_aviao->tempo_decolagem;
    Semaforo* pistas = dados_aviao->pistas;
    std::vector<Plane>* avioes = dados_aviao->avioes;
    std::vector<Runway>* pistasList = dados_aviao->pistasList;
    std::mutex* mutex_avioes = dados_aviao->mutex_avioes;
    int atraso_inicio = dados_aviao->atraso_inicio;

    // Atraso antes de iniciar a simulação baseado no id do avião e intervalo
    if(atraso_inicio > 0) {
        sleep(atraso_inicio);
    }

    std::cout << "Avião " << aviao_id << " iniciando voo.\n";

    // Inicializa avião no céu
    Plane aviao;
    aviao.id = aviao_id;
    aviao.x = 100.0f + (rand() % static_cast<int>(LIMITE_DIREITO - LIMITE_ESQUERDO)); // Inicia em algum lugar na tela
    aviao.y = POSICOES_Y_SPAWN[rand() % 3]; // Seleciona aleatoriamente uma das três alturas de spawn
    aviao.cor = {255, 255, 255, 255}; // Cor branca
    aviao.movendoParaDireita = (rand() % 2 == 0); // Direção inicial aleatória
    aviao.aterrissou = false;
    aviao.pistaAtribuida = -1; // Nenhuma pista atribuída inicialmente

    // Adiciona avião à lista compartilhada
    {
        std::lock_guard<std::mutex> lock(*mutex_avioes);
        avioes->push_back(aviao);
        //std::cout << "Plane " << aviao_id << " added to simulation at position (" << aviao.x << ", " << aviao.y << ").\n";
    }

    // Loop de movimento do avião
    bool pousoSolicitado = false;
    int contador_tempo_voo = 0; // Contador para simular tempo de voo antes do pouso

    while(!pousoSolicitado) {
        // Atualiza a posição do avião baseado na direção atual
        if(aviao.movendoParaDireita) {
            aviao.x += VELOCIDADE_MOVIMENTO;
            if(aviao.x >= LIMITE_DIREITO) {
                aviao.movendoParaDireita = false; // Muda direção para esquerda
            }
        }
        else {
            aviao.x -= VELOCIDADE_MOVIMENTO;
            if(aviao.x <= LIMITE_ESQUERDO) {
                aviao.movendoParaDireita = true; // Muda direção para direita
            }
        }

        // Atualiza a posição do avião na lista compartilhada
        {
            std::lock_guard<std::mutex> lock(*mutex_avioes);
            for(auto& p : *avioes) {
                if(p.id == aviao_id) {
                    p.x = aviao.x;
                    p.movendoParaDireita = aviao.movendoParaDireita;
                    p.y = aviao.y; // Assegura que y é consistente
                }
            }
        }

        // Sleep para controlar a velocidade do movimento
        usleep(ATRASO_MOVIMENTO_MS * 1000); // Converte ms para us

        // Simula tempo de voo antes do pouso (ex: voa por ~5 segundos)
        contador_tempo_voo++;
        if(contador_tempo_voo >= 300) { // 300 * 16ms ≈ 4.8 segundos
            pousoSolicitado = true;
        }
    }

    std::cout << "Avião " << aviao_id << " solicitando pouso...\n";

    // Atribui pista
    int pistaAtribuidaId = -1;
    SDL_Rect pistaAtribuidaRect;
    {
        std::lock_guard<std::mutex> lock(*mutex_avioes);
        // Encontra a primeira pista disponível
        for(auto& pista : *pistasList) {
            if(!pista.occupied) {
                pista.occupied = true;
                pistaAtribuidaId = pista.id;
                aviao.pistaAtribuida = pista.id;
                aviao.aterrissou = true;
                // Atualiza a posição do avião para o centro acima da pista
                aviao.x = pista.rect.x + pista.rect.w / 2.0f;
                aviao.y = pista.rect.y - 10.0f; // Ligeiramente acima da pista
                aviao.cor = {255, 0, 0, 255}; // Cor vermelha para indicar pouso
                pistaAtribuidaRect = pista.rect;
                std::cout << "Avião " << aviao_id << " atribuído à Pista " << pista.id << ".\n";
                break;
            }
        }

        if(pistaAtribuidaId == -1) {
            // Nenhuma pista disponível, espera até que uma se libere
            std::cout << "Avião " << aviao_id << " aguardando pista disponível.\n";
            while(pistaAtribuidaId == -1) {
                // Espera uma pista se tornar disponível
                sleep(1); // Espera 1 segundo antes de tentar novamente

                // Tenta adquirir uma pista novamente
                for(auto& pista : *pistasList) {
                    if(!pista.occupied) {
                        pista.occupied = true;
                        pistaAtribuidaId = pista.id;
                        aviao.pistaAtribuida = pista.id;
                        aviao.aterrissou = true;
                        // Atualiza a posição do avião para o centro acima da pista
                        aviao.x = pista.rect.x + pista.rect.w / 2.0f;
                        aviao.y = pista.rect.y - 10.0f; // Ligeiramente acima da pista
                        aviao.cor = {255, 0, 0, 255}; // Cor vermelha para indicar pouso
                        pistaAtribuidaRect = pista.rect;
                        std::cout << "Avião " << aviao_id << " atribuído à Pista " << pista.id << " após esperar.\n";
                        break;
                    }
                }
            }
        }

        // Atualiza o status do avião na lista compartilhada
        for(auto& p : *avioes) {
            if(p.id == aviao_id) {
                p.aterrissou = aviao.aterrissou;
                p.pistaAtribuida = aviao.pistaAtribuida;
                p.x = aviao.x;
                p.y = aviao.y;
                p.cor = aviao.cor;
                break;
            }
        }
    }

    // Simula tempo de pouso
    sleep(tempo_pouso);

    std::cout << "Avião " << aviao_id << " pousou na pista " << aviao.pistaAtribuida << ".\n";

    // Simula desembarque de passageiros
    sleep(tempo_desembarque);

    // Simula decolagem
    std::cout << "Avião " << aviao_id << " decolando da pista " << aviao.pistaAtribuida << "...\n";

    // Atualiza avião para decolagem (muda cor para verde e aponta para cima)
    {
        std::lock_guard<std::mutex> lock(*mutex_avioes);
        for(auto& p : *avioes) {
            if(p.id == aviao_id) {
                p.cor = {0, 255, 0, 255}; // Cor verde para indicar decolagem
                break;
            }
        }
    }

    // Implementa movimento suave para cima com orientação para cima
    for(int i = 0; i < 20; ++i) { // Move para cima durante 20 quadros (~320ms)
        {
            std::lock_guard<std::mutex> lock(*mutex_avioes);
            for(auto& p : *avioes) {
                if(p.id == aviao_id) {
                    p.y -= 1.0f; // Move avião para cima
                    break;
                }
            }
        }
        usleep(ATRASO_MOVIMENTO_MS * 1000); // ~60 FPS
    }

    // Libera a pista
    {
        std::lock_guard<std::mutex> lock(*mutex_avioes);
        for(auto& pista : *pistasList) {
            if(pista.id == aviao.pistaAtribuida) {
                pista.occupied = false;
                std::cout << "Avião " << aviao_id << " liberou a pista " << pista.id << ".\n";
                break;
            }
        }

        // Remove avião da lista compartilhada
        avioes->erase(std::remove_if(avioes->begin(), avioes->end(),
            [&](const Plane& p) { return p.id == aviao_id; }), avioes->end());
    }

    pistas->notificar();

    delete dados_aviao; // Libera memória alocada
    pthread_exit(NULL);
}
