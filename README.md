# Relógio em C com Raylib 🕒

Um projeto de estudo implementando um Cronômetro e Temporizador gráfico usando a biblioteca Raylib.

## Funcionalidades
- Cronômetro (Iniciar, Pausar, Resetar, persistência de dados).
- Temporizador (Alarme sonoro, seleção de tempo).
- Persistência de dados binária (o tempo não perde ao fechar).

## Como Compilar (Windows)
Este projeto utiliza Make. Certifique-se de ter o GCC/MinGW instalado e o Make configurado.

1. Clone o repositório.
2. No terminal, rode:
   ```bash
   make
Caso não tenha o Make
rode este comando:
   ```bash
    gcc src/main.c -o relogio.exe -I./include -L./lib-lraylib -lopengl32 -lgdi32 -lwinmm -luser32 -lshell32

	./relogio.exe
