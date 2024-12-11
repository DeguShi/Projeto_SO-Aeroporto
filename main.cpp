#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <algorithm>

#include <SDL.h>        
#include <SDL_ttf.h>    

#include "aviao.h"
#include "semaforo.h"
#include "utils.h"
#include "pista.h" 

using namespace std;

// Dimensões da janela
constexpr int LARGURA_JANELA = 800;
constexpr int ALTURA_JANELA = 600;

// Estrutura de configurações
struct Configuracao {
    int tempo_pouso = 1;
    int tempo_desembarque = 5;
    int tempo_decolagem = 1;
    int tempo_entre_avioes = 10;
    int num_pistas = 2;
    int num_avioes = 5;
};

// Enum para representar o estado atual da aplicação
enum class EstadoApp {
    CONFIGURACAO,
    SIMULACAO
};

// Estrutura para gerenciar recursos compartilhados
struct RecursosCompartilhados {
    vector<Plane> avioes;
    vector<Runway> pistas; // Lista de pistas
    mutex mutex_avioes;
    Semaforo* semaforo_pistas;
};

// Função para renderizar interface de configuração
void renderizarConfiguracao(SDL_Renderer* renderer, TTF_Font* font, const Configuracao& config, 
                            const vector<SDL_Rect>& botoes_mais, const vector<SDL_Rect>& botoes_menos, 
                            const SDL_Rect& botao_play, const SDL_Color& preto, const SDL_Color& verde, 
                            const SDL_Color& vermelho, const SDL_Color& branco) {
    // Limpa a tela com fundo branco
    SDL_SetRenderDrawColor(renderer, branco.r, branco.g, branco.b, branco.a);
    SDL_RenderClear(renderer);

    // Renderiza títulos e rótulos
    renderizarTexto(renderer, font, "Configurações de Simulação", 250, 50, preto);

    // Rótulos para cada configuração
    renderizarTexto(renderer, font, "Tempo de Pouso (s): " + to_string(config.tempo_pouso), 100, 150, preto);
    renderizarTexto(renderer, font, "Tempo de Desembarque (s): " + to_string(config.tempo_desembarque), 100, 200, preto);
    renderizarTexto(renderer, font, "Tempo de Decolagem (s): " + to_string(config.tempo_decolagem), 100, 250, preto);
    renderizarTexto(renderer, font, "Tempo entre Aviões (s): " + to_string(config.tempo_entre_avioes), 100, 300, preto);
    renderizarTexto(renderer, font, "Número de Pistas: " + to_string(config.num_pistas), 100, 350, preto);
    renderizarTexto(renderer, font, "Número de Aviões: " + to_string(config.num_avioes), 100, 400, preto);

    // Renderizar botões "+"
    for(size_t i = 0; i < botoes_mais.size(); ++i) {
        if(i == 4 && config.num_pistas >= 3) { // Botão "+" para pistas quando o máximo for atingido
            // Renderiza botão "+" desabilitado (cinza escuro)
            SDL_SetRenderDrawColor(renderer, 169, 169, 169, 255); // Cinza Escuro
            SDL_RenderFillRect(renderer, &botoes_mais[i]);
            // Renderiza símbolo "+" em branco
            renderizarTexto(renderer, font, "+", botoes_mais[i].x + 10, botoes_mais[i].y + 5, branco);
        } else {
            // Renderiza botão "+" ativo (verde)
            SDL_SetRenderDrawColor(renderer, verde.r, verde.g, verde.b, verde.a);
            SDL_RenderFillRect(renderer, &botoes_mais[i]);
            // Renderiza símbolo "+" em branco
            renderizarTexto(renderer, font, "+", botoes_mais[i].x + 10, botoes_mais[i].y + 5, branco);
        }
    }

    // Renderizar botões "-"
    for(size_t i = 0; i < botoes_menos.size(); ++i) {
        // Sempre renderizar botões "-" em vermelho
        SDL_SetRenderDrawColor(renderer, vermelho.r, vermelho.g, vermelho.b, vermelho.a);
        
        // Se for o botão "-" para tempo_entre_avioes e já estiver no mínimo, renderize em cinza
        if(i == 3 && config.tempo_entre_avioes <= 4) {
            SDL_SetRenderDrawColor(renderer, 169, 169, 169, 255);  // Cinza para desabilitado
        }
        
        SDL_RenderFillRect(renderer, &botoes_menos[i]);
        
        // Renderizar símbolo "-" em branco
        renderizarTexto(renderer, font, "-", botoes_menos[i].x + 10, botoes_menos[i].y + 5, branco);
    }

    // Renderizar botão Play
    SDL_SetRenderDrawColor(renderer, verde.r, verde.g, verde.b, verde.a);
    SDL_RenderFillRect(renderer, &botao_play);
    renderizarTexto(renderer, font, "Play", botao_play.x + 25, botao_play.y + 10, preto);
}

// Define um tamanho constante para todos os aviões
constexpr float TAMANHO_AVIAO = 15.0f;

// Função para renderizar interface de simulação
void renderizarSimulacao(SDL_Renderer* renderer, TTF_Font* font, const Configuracao& config, 
                         RecursosCompartilhados& recursos, const SDL_Color& branco, 
                         const SDL_Color& azul_celar, const SDL_Color& cinza_escuro, const SDL_Color& cinza) {
    // Limpa a tela com fundo azul celar
    SDL_SetRenderDrawColor(renderer, azul_celar.r, azul_celar.g, azul_celar.b, azul_celar.a);
    SDL_RenderClear(renderer);

    // Desenha solo
    SDL_SetRenderDrawColor(renderer, cinza_escuro.r, cinza_escuro.g, cinza_escuro.b, cinza_escuro.a);
    SDL_Rect solo = {0, ALTURA_JANELA - 100, LARGURA_JANELA, 100};
    SDL_RenderFillRect(renderer, &solo);

    // Desenha pistas
    for(auto& pista : recursos.pistas) {
        SDL_Rect retangulo_pista = pista.rect;
        if(pista.occupied) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Vermelho se ocupada
        } else {
            SDL_SetRenderDrawColor(renderer, cinza.r, cinza.g, cinza.b, cinza.a); // Cinza se livre
        }
        SDL_RenderFillRect(renderer, &retangulo_pista);

        // Renderizar ID da pista no centro
        std::string label_pista = "Pista " + std::to_string(pista.id);
        int largura_label, altura_label;
        TTF_SizeUTF8(font, label_pista.c_str(), &largura_label, &altura_label);
        renderizarTexto(renderer, font, label_pista, pista.rect.x + (pista.rect.w - largura_label) / 2, 
                       pista.rect.y + (pista.rect.h - altura_label) / 2, branco);
    }

    // Renderizar título da simulação
    renderizarTexto(renderer, font, "Simulação de Aeroporto", 300, 20, branco);

    // Renderizar aviões
    {
        lock_guard<mutex> lock(recursos.mutex_avioes);
        size_t numAvioes = recursos.avioes.size();
        //cout << "Rendering simulation: " << numAvioes << " aviões.\n"; // Mensagem de depuração
        for (const auto& aviao : recursos.avioes) {
            if(!aviao.aterrissou) {
                // Avião está no céu
                if(aviao.movendoParaDireita) {
                    // Apontando para a direita
                    preencherTriangulo(renderer,
                                      static_cast<Sint16>(aviao.x),
                                      static_cast<Sint16>(aviao.y),
                                      TAMANHO_AVIAO,
                                      OrientacaoTriangulo::DIREITA,
                                      aviao.cor);
                } else {
                    // Apontando para a esquerda
                    preencherTriangulo(renderer,
                                      static_cast<Sint16>(aviao.x),
                                      static_cast<Sint16>(aviao.y),
                                      TAMANHO_AVIAO,
                                      OrientacaoTriangulo::ESQUERDA,
                                      aviao.cor);
                }
            }
            else {
                // Avião aterrissado ou decolando
                if(aviao.cor.r == 255 && aviao.cor.g == 0 && aviao.cor.b == 0) {
                    // Avião aterrissado: vermelho e apontando para baixo
                    preencherTriangulo(renderer,
                                      static_cast<Sint16>(aviao.x),
                                      static_cast<Sint16>(aviao.y),
                                      TAMANHO_AVIAO,
                                      OrientacaoTriangulo::BAIXO,
                                      aviao.cor);
                }
                else if(aviao.cor.r == 0 && aviao.cor.g == 255 && aviao.cor.b == 0) {
                    // Avião decolando: verde e apontando para cima
                    preencherTriangulo(renderer,
                                      static_cast<Sint16>(aviao.x),
                                      static_cast<Sint16>(aviao.y),
                                      TAMANHO_AVIAO,
                                      OrientacaoTriangulo::CIMA,
                                      aviao.cor);
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // Inicializa gerador de números aleatórios
    srand(static_cast<unsigned int>(time(nullptr)));

    // Inicializa SDL
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        cerr << "Erro na inicialização do SDL: " << SDL_GetError() << endl;
        return 1;
    }
    cout << "SDL inicializado com sucesso.\n";

    // Inicializa SDL_ttf
    if(TTF_Init() != 0) {
        cerr << "Erro na inicialização do SDL_ttf: " << TTF_GetError() << endl;
        SDL_Quit();
        return 1;
    }
    cout << "SDL_ttf inicializado com sucesso.\n";

    // Cria janela SDL
    SDL_Window* window = SDL_CreateWindow("Simulação de Aeroporto", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          LARGURA_JANELA, ALTURA_JANELA, SDL_WINDOW_SHOWN);
    if(!window) {
        cerr << "Erro na criação da janela SDL: " << SDL_GetError() << endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    cout << "Janela criada com sucesso.\n";

    // Cria renderer SDL
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        cerr << "Erro na criação do renderer SDL: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    cout << "Renderer criado com sucesso.\n";

    // Carrega fonte
    // Assegure-se de que "arial.ttf" está no mesmo diretório que o executável ou forneça o caminho completo
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if(!font) {
        cerr << "Falha ao carregar a fonte: " << TTF_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    cout << "Fonte carregada com sucesso.\n";

    // Define botões "+" e "-" para cada configuração
    // Posições para botões "+"
    vector<SDL_Rect> botoes_mais = {
        {450, 150, 30, 30}, // Tempo de Pouso
        {450, 200, 30, 30}, // Tempo de Desembarque
        {450, 250, 30, 30}, // Tempo de Decolagem
        {450, 300, 30, 30}, // Tempo entre Aviões
        {450, 350, 30, 30}, // Número de Pistas
        {450, 400, 30, 30}  // Número de Aviões
    };

    // Posições para botões "-"
    vector<SDL_Rect> botoes_menos = {
        {500, 150, 30, 30}, // Tempo de Pouso
        {500, 200, 30, 30}, // Tempo de Desembarque
        {500, 250, 30, 30}, // Tempo de Decolagem
        {500, 300, 30, 30}, // Tempo entre Aviões
        {500, 350, 30, 30}, // Número de Pistas
        {500, 400, 30, 30}  // Número de Aviões
    };

    // Botão Play
    SDL_Rect botao_play = {350, 500, 100, 50};

    // Cores
    SDL_Color preto = {0, 0, 0, 255};
    SDL_Color verde = {0, 255, 0, 255};
    SDL_Color vermelho = {255, 0, 0, 255};
    SDL_Color branco = {255, 255, 255, 255};
    SDL_Color azul_celar = {135, 206, 235, 255};
    SDL_Color cinza_escuro = {50, 50, 50, 255};
    SDL_Color cinza = {169, 169, 169, 255};

    // Estado inicial
    EstadoApp estado_atual = EstadoApp::CONFIGURACAO;

    // Configurações
    Configuracao config;

    // Recursos compartilhados
    RecursosCompartilhados recursos;
    recursos.semaforo_pistas = nullptr; // Será inicializado ao iniciar a simulação

    // Threads de aviões
    vector<pthread_t> threads;

    // Loop principal
    bool rodando = true;
    while (rodando) {
        SDL_Event evento;
        while(SDL_PollEvent(&evento)) {
            if(evento.type == SDL_QUIT) {
                cout << "Evento SDL_QUIT recebido. Encerrando...\n";
                rodando = false;
            }

            // Lidar com eventos baseado no estado atual
            if(estado_atual == EstadoApp::CONFIGURACAO) {
                if(evento.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX = evento.button.x;
                    int mouseY = evento.button.y;

                    // Verificar se o clique foi em um botão "+"
                    for(size_t i = 0; i < botoes_mais.size(); ++i) {
                        if(mouse_noRetangulo(mouseX, mouseY, botoes_mais[i])) {
                            switch(i) {
                                case 0: 
                                    config.tempo_pouso++; 
                                    cout << "Tempo de Pouso incrementado para " << config.tempo_pouso << " segundos." << endl;
                                    break;
                                case 1: 
                                    config.tempo_desembarque++; 
                                    cout << "Tempo de Desembarque incrementado para " << config.tempo_desembarque << " segundos." << endl;
                                    break;
                                case 2: 
                                    config.tempo_decolagem++; 
                                    cout << "Tempo de Decolagem incrementado para " << config.tempo_decolagem << " segundos." << endl;
                                    break;
                                case 3: 
                                    config.tempo_entre_avioes++; 
                                    cout << "Tempo entre Aviões incrementado para " << config.tempo_entre_avioes << " segundos." << endl;
                                    break;
                                case 4: // Número de Pistas
                                    if(config.num_pistas < 3) { // Impõe máximo de 3
                                        config.num_pistas++; 
                                        cout << "Número de Pistas incrementado para " << config.num_pistas << endl;
                                    } else {
                                        cout << "Máximo de 3 Pistas já atingido.\n";
                                    }
                                    break;
                                case 5: 
                                    config.num_avioes++; 
                                    cout << "Número de Aviões incrementado para " << config.num_avioes << endl;
                                    break;
                            }
                        }
                    }

                    // Verificar se o clique foi em um botão "-"
                    for(size_t i = 0; i < botoes_menos.size(); ++i) {
                        if(mouse_noRetangulo(mouseX, mouseY, botoes_menos[i])) {
                            switch(i) {
                                case 0: 
                                    if(config.tempo_pouso > 1) {
                                        config.tempo_pouso--; 
                                        cout << "Tempo de Pouso decrementado para " << config.tempo_pouso << " segundos." << endl;
                                    }
                                    break;
                                case 1: 
                                    if(config.tempo_desembarque > 1) {
                                        config.tempo_desembarque--; 
                                        cout << "Tempo de Desembarque decrementado para " << config.tempo_desembarque << " segundos." << endl;
                                    }
                                    break;
                                case 2: 
                                    if(config.tempo_decolagem > 1) {
                                        config.tempo_decolagem--; 
                                        cout << "Tempo de Decolagem decrementado para " << config.tempo_decolagem << " segundos." << endl;
                                    }
                                    break;
                                case 3: 
                                    if(config.tempo_entre_avioes > 4) { // Alterado para > 4
                                        config.tempo_entre_avioes--; 
                                        cout << "Tempo entre Aviões decrementado para " << config.tempo_entre_avioes << " segundos." << endl;
                                    }
                                    else {
                                        cout << "Tempo entre Aviões não pode ser menor que 4 segundos." << endl;
                                    }
                                    break;
                                case 4: // Número de Pistas
                                    if(config.num_pistas > 1) { 
                                        config.num_pistas--; 
                                        cout << "Número de Pistas decrementado para " << config.num_pistas << endl;
                                    }
                                    break;
                                case 5: 
                                    if(config.num_avioes > 1) {
                                        config.num_avioes--; 
                                        cout << "Número de Aviões decrementado para " << config.num_avioes << endl;
                                    }
                                    break;
                            }
                        }
                    }

                    // Verificar se o clique foi no botão Play
                    if(mouse_noRetangulo(mouseX, mouseY, botao_play)) {
                        cout << "Mudando para o estado SIMULACAO.\n";
                        estado_atual = EstadoApp::SIMULACAO;

                        // Inicializar semáforo
                        recursos.semaforo_pistas = new Semaforo(config.num_pistas);
                        cout << "Semáforo inicializado com " << config.num_pistas << " pistas.\n";

                        // Inicializar pistas
                        int largura_pista = 200;
                        int altura_pista = 20;
                        int espaco = 50;
                        int largura_total = config.num_pistas * largura_pista + (config.num_pistas - 1) * espaco;
                        int inicioX = (LARGURA_JANELA - largura_total) / 2;
                        int Y_pista = ALTURA_JANELA - 100; // Posicionar pistas sobre o solo

                        // Criar pistas e adicionar a RecursosCompartilhados
                        for(int i = 0; i < config.num_pistas; ++i) {
                            SDL_Rect retangulo_pista = { inicioX + i * (largura_pista + espaco), Y_pista, largura_pista, altura_pista };
                            recursos.pistas.emplace_back(i + 1, retangulo_pista);
                        }

                        cout << config.num_pistas << " pistas inicializadas.\n";

                        // Criar threads para cada avião sem dormir
                        threads.resize(config.num_avioes);
                        for (int i = 0; i < config.num_avioes; ++i) {
                            AviaoData* dados_aviao = new AviaoData;
                            dados_aviao->id = i + 1;
                            dados_aviao->tempo_pouso = config.tempo_pouso;
                            dados_aviao->tempo_desembarque = config.tempo_desembarque;
                            dados_aviao->tempo_decolagem = config.tempo_decolagem;
                            dados_aviao->pistas = recursos.semaforo_pistas;
                            dados_aviao->avioes = &recursos.avioes;
                            dados_aviao->pistasList = &recursos.pistas; // Passa pistas para as threads dos aviões
                            dados_aviao->mutex_avioes = &recursos.mutex_avioes;
                            dados_aviao->atraso_inicio = i * config.tempo_entre_avioes; // Atribui atraso de início

                            if (pthread_create(&threads[i], NULL, aviao, dados_aviao) != 0) {
                                cerr << "Erro ao criar thread para o avião " << i + 1 << endl;
                                delete dados_aviao;
                            }
                            // Removido sleep para evitar bloqueio da thread principal
                        }
                    }
                }
            }
            else if(estado_atual == EstadoApp::SIMULACAO) {
                // Lidar com eventos específicos da simulação, se houver (ex: pausar, reiniciar)
                // Atualmente, nenhum tratamento adicional
            }
        }

        // Renderização baseada no estado atual
        if(estado_atual == EstadoApp::CONFIGURACAO) {
            renderizarConfiguracao(renderer, font, config, botoes_mais, botoes_menos, botao_play, preto, verde, vermelho, branco);
        }
        else if(estado_atual == EstadoApp::SIMULACAO) {
            renderizarSimulacao(renderer, font, config, recursos, branco, azul_celar, cinza_escuro, cinza);
        }

        // Apresenta o renderer
        SDL_RenderPresent(renderer);

        // Delay para controlar taxa de quadros
        SDL_Delay(16); // Aproximadamente 60 FPS

        // Verifica se a simulação terminou
        if(estado_atual == EstadoApp::SIMULACAO) {
            // Bloqueia a lista de aviões para verificar se está vazia
            {
                lock_guard<mutex> lock(recursos.mutex_avioes);
                if(recursos.avioes.empty()) {
                    cout << "A simulação terminou, obrigado por viajar conosco...\n";
                    // Espera alguns segundos antes de fechar a simulação
                    sleep(2);
                    rodando = false;
                }
            }
        }
    }

    // Limpeza
    // Se estiver no estado SIMULACAO, destruir semáforo
    if(recursos.semaforo_pistas) {
        delete recursos.semaforo_pistas;
    }

    // Esperar todas as threads de aviões terminarem
    for(auto& thread : threads) {
        pthread_join(thread, NULL);
    }

    // Limpar recursos SDL
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
