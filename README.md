# **Projeto SO - Aeroporto**

Bem-vindo ao repositório do projeto **SO - Aeroporto**. Este projeto simula o gerenciamento de um aeroporto com múltiplas pistas, utilizando programação em **C++** e bibliotecas **SDL2** para renderização gráfica.

---

# **Conteúdo**
- [Descrição do Projeto](#descrição-do-projeto)
- [Pré-requisitos](#pré-requisitos)
- [Instruções de Build](#instruções-de-build)
  - [macOS](#macos)
  - [Linux](#linux)
  - [Windows](#windows)
- [Execução](#execução)
- [Contribuições](#contribuições)
- [Licença](#licença)

---

## **Descrição do Projeto**
Este projeto tem como objetivo simular operações de um aeroporto:
- Controle de aviões e suas movimentações.
- Gerenciamento de pistas de pouso e decolagem.
- Controle de semáforos utilizando conceitos de programação concorrente.

As bibliotecas **SDL2** e **SDL2_ttf** são utilizadas para visualização gráfica do projeto.

---

## **Pré-requisitos**

Antes de começar, você precisa ter as seguintes ferramentas instaladas no seu sistema:
- Um compilador **C++** compatível com **C++17** (ex.: `g++`, `clang++`).
- **SDL2** e **SDL2_ttf** (bibliotecas de desenvolvimento).
- [CMake](https://cmake.org/) (opcional, para build multiplataforma).

---

## **Instruções de Build**

### **macOS**
1. Instale as dependências com [Homebrew](https://brew.sh/):
   ```bash
   brew install sdl2 sdl2_ttf
