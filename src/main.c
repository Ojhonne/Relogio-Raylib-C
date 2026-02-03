#include <stdio.h>
#include "raylib.h"
#include <time.h> 

//----------------------------------------------------------------------------------
// MACROS
//----------------------------------------------------------------------------------

#define SLOTS_CAIXA_RELOGIO 3
#define SLOTS_TEMPORIZADOR 6
#define TEXTO_RETORNO_JANELA "Voce deseja retonar para o menu inicial? [S/N]"
#define TEXTO_SAIR_RELOGIO "Voce tem certeza que deseja sair? [S/N]"

//Macros para desenhar relogio
#define RAIO_INTERNO 70
#define RAIO_EXTERNO 75
#define LADOS 0
#define ANGULO_INICIAL 0
#define ANGULO_FINAL 360

//Macros Main
#define FPS 60

#define DESATIVADO -1 //macro que define uma variavel como desativada

//macros temporizador
#define TEMPO_ZERADO 0.0f
#define TEMPO_30_SEG 30
#define TEMPO_1_MIN 60
#define TEMPO_5_MIN 300
#define TEMPO_10_MIN 600
#define TEMPO_25_MIN 1500
#define TEMPO_30_MIN 1800
#define TEMPO_50_MIN 3000
#define TEMPO_90_MIN 3600
#define TEMPO_120_MIN 7200

//macros Alarme
#define TEMPO_MUSICA_TOCANDO 10.0f 
#define VOLUME_MUSICA 0.5

//macro que define qual menu vai executar
#define MENU_INICIAL 0
#define MENU_CRONOMETRO 1
#define MENU_TEMPORIZADOR 2

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef struct {
    float timer;
    bool pause; 
} Clock; //struct que guarda informacoes do tempo

typedef struct{
    float tempoPiscar;
    float tempoCorrido;
    bool textoVisivel;
}alarmeAtivado; //struct que guarda infromacoes quando o alarme tocou

typedef struct{
    bool alarmeTocou;
    bool modoAlarme;
    bool reset;
}flagsAlarme; //struct que guarda as flags do temporizador

typedef struct{ 
    int posicaoMouseBotaoTemporizador;
    int posicaoMouseBotaoTempoPredefinido;
    int posicaoMouseBotaoAlteraTempo;

    int indiceBotaoTemporizador;
    int indiceBotaoAlteraTempo;
    int indiceBotaoTempoDefinido;
}statusGUI; //Struct que guarda as opcoes definidas ou selecionadas pelo mouse do usuario

typedef struct{
    Clock cronometro;
    Clock temporizador;
}estadoAplicativo; //Struct que guarda as configuraçoes do Relogio

static const char *textoMenu[] = { //vetores de string que guarda os texto do menu
    "1. CRONOMETRO",
    "2. TEMPORIZADOR",
    "3. SAIR"
};

static const char *textoCronometro[] = { //vetores de string que guarda os texto do cronometro
    "INICIAR",
    "PAUSAR",
    "RESETAR"
};

static const char *textoSlotsTemporizador[] = { //vetores de string que guarda os tempos predefinido do temporizador
    "10m", "25m", "30m","50m","1h",
    "2h"
};

static const char *textoTempoTemporizador[] = { ////vetores de string que guarda o incremento ou decremento do tempo
    "-30s", "-1m", "-5m",
    "+30s", "+1m", "+5m"
};
static const char *nomeArquivo = "resources/DadosRelogio.bin"; //lugar aonde esta o arquivo bin
static const char *nomeArquivoSomAlarme = "resources/somTemporizador.mp3"; //lugar aonde esta o arquivo da musica
static const char *nomeDoExecutavel = "RELOGIO"; // defiine o nome do executavel

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------

//tamanho da tela
const int screenWidth = 800;
const int screenHeight = 600;
    
bool exitWindow; // Flag para sair do relogio

//Retangulos para desenhar os botoes do relogio
Rectangle caixasMenu[SLOTS_CAIXA_RELOGIO] = {0};
Rectangle botoesRelogio[SLOTS_CAIXA_RELOGIO] = {0};
Rectangle caixasTempoPredefido[SLOTS_TEMPORIZADOR] = {0};
Rectangle caixasTemporizador[SLOTS_TEMPORIZADOR] = {0};

//Vetores para desenhar os circulos
Vector2 posicaoCirculoCronometro[SLOTS_CAIXA_RELOGIO] = {0}; 
Vector2 posicaoCirciculoTemporizador[SLOTS_CAIXA_RELOGIO] = {0};

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------

//Funcoes iniciais 
static void inicializaVariaveis(statusGUI *gui, alarmeAtivado *ativado);
static void voltarMenuRelogio(int *opcao, int *escolhaJanelas, bool *exitWindowRequested);

//Funcao que cuida do tempo
static void processaTempo(Clock *tempo, int* segundos, int* minutos, int* horas);

//Funcoes responsaveis pela logica do relogio
static void logicaMenuRelogio(int *posicaoMouseMenu, int *escolhaJanelas,  int *opcaoMenu, bool *exitWindowRequested);
static void logicaCronometro(Clock *cronometro, int *opcaoMenuCronometro);
static void logicaTemporizador(Clock *temporizador, int *escolhaJanelas, alarmeAtivado *alarmeAtivo, flagsAlarme *alarme, Sound somTemporizador);

//Funcoes responsaveis pelo input das funcoes de cima
static void pegaPosicaoMouseSlotsRelogio(int *posicaoMouse, Rectangle *botoes, int *opcaoEscolhida);
static void processaInputCronometro(Clock *cronometro, int* posicaoMouseCronometro, int* escolhaJanelas, int *opcaoMenuCronometro, bool* exitWindowBack);
static void processaInputTemporizador(Clock *temporizador, int *escolhaJanelas, bool* exitWindowBack, flagsAlarme *alarme, statusGUI *gui);

// Funcoes que desenham
static void desenhaMenuRelogio(int posicaoMouseMenu, int opcaoMenu,  bool exitWindowRequested);
static void desenhaTemporizador(Clock *temporizador,  statusGUI gui, bool textoVisivel, bool exitWindowBack);
static void desenhaCronometro(Clock *cronometro, int posicaoMouseCronometro, int opcaoMenuCronometro, bool exitWindowRequested);
static void desenhaLayoutRelogio(Vector2 *posicaoRelogio, const short posY1, const short posY2, short posX, const short tamanho, const short aumentaX, const short diminuiRaio);
static void desenhaLetrasRelogio(const short posX_H, const short posX_M, const short posX_S, short diminuiTamanho);
static void desenhaBotoesRelogio(Rectangle *botoes, short opcao, short posicaoMouse);
static void desenhaMensagemRetornoJanela(bool sairJanela, Color cor, const char *text);

//Funcoes que armazenam dados
static void salvaDadosRelogio(const char* nomeArquivo, estadoAplicativo *Relogio);
static int LerDadosRelogio(const char* nomeArquivo, estadoAplicativo *Relogio);


int main(void){

    int escolhaJanelas = MENU_INICIAL; //comeca no menu inicial
    estadoAplicativo relogio = {0};

    //Menu Inicial
    bool exitWindowRequested = false;
    bool exitWindowBack = false;
    int opcaoMenu = DESATIVADO;
    int posicaoMouseMenu = DESATIVADO; 
  
    //variaveis cronometro
    Clock cronometro = {0};
    int opcaoMenuCronometro = DESATIVADO;
    int posicaoMouseCronometro = DESATIVADO;

    //variaveis temporizador
    Clock temporizador = {0};
    flagsAlarme alarme = {0};
    alarmeAtivado alarmeAtivo = {0};
    statusGUI guiState = {0};

    InitWindow(screenWidth, screenHeight, nomeDoExecutavel); // criando janela 
    InitAudioDevice();   //inicalizando o audio

    Sound somTemporizador = LoadSound(nomeArquivoSomAlarme);
    SetSoundVolume(somTemporizador, VOLUME_MUSICA); //funçao para definir a intensidade do som
    
    LerDadosRelogio(nomeArquivo, &relogio);
    // Distribui os valores (se for a primeira vez, vai ser tudo 0)
    cronometro = relogio.cronometro;
    temporizador = relogio.temporizador;

    if(temporizador.timer == 0) temporizador.timer = TEMPO_25_MIN;
    inicializaVariaveis(&guiState, &alarmeAtivo);
    
    SetTargetFPS(FPS);
    //Game loop 
	while (!exitWindow) {
        
        logicaCronometro(&cronometro, &opcaoMenuCronometro);
        logicaTemporizador(&temporizador, &escolhaJanelas, &alarmeAtivo, &alarme, somTemporizador); 
        
        switch (escolhaJanelas){
        case MENU_INICIAL: logicaMenuRelogio(&posicaoMouseMenu, &escolhaJanelas, &opcaoMenu, &exitWindowRequested); break;
        case MENU_CRONOMETRO: processaInputCronometro(&cronometro, &posicaoMouseCronometro, &escolhaJanelas, &opcaoMenuCronometro, &exitWindowBack); break;
        case MENU_TEMPORIZADOR: processaInputTemporizador(&temporizador, &escolhaJanelas, &exitWindowBack, &alarme, &guiState); break;
        default: break;
        } 
        //Desenho do relogio
        BeginDrawing();
        switch (escolhaJanelas){
        case MENU_INICIAL: desenhaMenuRelogio(posicaoMouseMenu, opcaoMenu, exitWindowRequested); break;
        case MENU_CRONOMETRO:desenhaCronometro(&cronometro, posicaoMouseCronometro, opcaoMenuCronometro, exitWindowBack); break;
        case MENU_TEMPORIZADOR: desenhaTemporizador(&temporizador, guiState, alarmeAtivo.textoVisivel, exitWindowBack); break;
        default: break;
        }
        EndDrawing();

	}
    relogio.cronometro = cronometro;
    relogio.temporizador = temporizador;
    salvaDadosRelogio(nomeArquivo, &relogio); //salvando dados do cronometro

    CloseAudioDevice();     // Close audio device
    CloseWindow();          //close the main window

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

void inicializaVariaveis(statusGUI *gui, alarmeAtivado *ativado){
    SetExitKey(KEY_NULL);

    exitWindow = false;    //Definindo a flag para sair do relogio como FALSE

    ativado->textoVisivel = true;

    gui->posicaoMouseBotaoTemporizador = DESATIVADO;
    gui->posicaoMouseBotaoTempoPredefinido = DESATIVADO;
    gui->posicaoMouseBotaoAlteraTempo = DESATIVADO;
    gui->indiceBotaoTemporizador = DESATIVADO;
    gui->indiceBotaoAlteraTempo = DESATIVADO;
    gui->indiceBotaoTempoDefinido = DESATIVADO;

    int aux = 190;
    //Inicializando retangulos 
    for (int i = 0; i < SLOTS_CAIXA_RELOGIO; i++) caixasMenu[i] = (Rectangle){ 50.0f, (float)(200 + 90*i), 350.0f, 75.0f }; 
    for (int i = 0; i < SLOTS_CAIXA_RELOGIO; i++) botoesRelogio[i] = (Rectangle){(float)(100 + 200*i), 450, 180, 75.0f};
    for (int i = 0; i < SLOTS_TEMPORIZADOR; i++) caixasTempoPredefido[i] = (Rectangle){(float)(240 + 60*i), 145, 50.0f, 50.0f };
    for (int i = 0; i < SLOTS_TEMPORIZADOR; i++){
        if(i <= 1){
            caixasTemporizador[i] = (Rectangle){(float)(10 + 75*i), 260, 55.0f, 55.0f };
        } else if (i == 2){
            caixasTemporizador[i] = (Rectangle){(float)(8 + 20*i), 320, 55.0f, 55.0f };
        } else if ( i > 2 && i < 5){
            caixasTemporizador[i] = (Rectangle){(float)(440 + 75*i), 260, 55.0f, 55.0f };  
        } else caixasTemporizador[i] = (Rectangle){(float)(500 + 40*i), 320, 55.0f, 55.0f }; 
    }
    //inicializando os vetores
    for(int i = 0; i < SLOTS_CAIXA_RELOGIO; i++){ 
        posicaoCirculoCronometro[i] = (Vector2){aux,270};
        aux+=200;
    }
    aux = 250;
    for(int i = 0; i < SLOTS_CAIXA_RELOGIO; i++){ 
        posicaoCirciculoTemporizador[i] = (Vector2){aux, 290};
        aux+=150;
    } 
    
}
void voltarMenuRelogio(int *opcao, int *escolhaJanelas, bool* exitWindowRequested){ //funcao que verifica se deseja retornar ou sair do relogio
    if((*exitWindowRequested) && (*escolhaJanelas) == MENU_INICIAL){
       *opcao = DESATIVADO;
        if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_ENTER)) exitWindow = true;
        else if (IsKeyPressed(KEY_N)) *exitWindowRequested = false;
    } else if ((*exitWindowRequested)){
        *opcao = DESATIVADO;
        if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_ENTER)){
            *exitWindowRequested = false; 
            *escolhaJanelas = MENU_INICIAL;
        } else if (IsKeyPressed(KEY_N)) {
            *exitWindowRequested = false;
        }
    } 
}
void pegaPosicaoMouseSlotsRelogio(int *posicaoMouse, Rectangle *botoes, int *opcaoEscolhida){ //funcao que pega a posicao do mouse com base nos botoes
    for (int i = 0; i < SLOTS_CAIXA_RELOGIO; i++) {
        if (CheckCollisionPointRec(GetMousePosition(), botoes[i])){ 
            *posicaoMouse = i;       
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) *opcaoEscolhida = *posicaoMouse;
            break;
        } else {
            *posicaoMouse = DESATIVADO;
        } 
    }

}

void processaTempo(Clock *tempo, int* segundos, int* minutos, int* horas){ //funcao que realiza os calculos do tempo
    *horas = (int)tempo->timer/3600;
    *minutos = ((int)tempo->timer % 3600) / 60.0;
    *segundos = (int)tempo->timer % 60;
}
void logicaMenuRelogio(int *posicaoMouseMenu, int *escolhaJanelas, int *opcaoMenu, bool* exitWindowRequested){

    pegaPosicaoMouseSlotsRelogio(posicaoMouseMenu, caixasMenu, opcaoMenu);  //logica do mouse
    
    //logica das teclas pressionadas
    int key = GetKeyPressed();
    switch (key){
    case KEY_ONE: *opcaoMenu = 0; break;
    case KEY_TWO: *opcaoMenu = 1; break;   
    case KEY_THREE: *opcaoMenu = 2; break;
    case KEY_DOWN: *opcaoMenu++; if (*opcaoMenu > (SLOTS_CAIXA_RELOGIO - 1)) *opcaoMenu = 0; break;  
    case KEY_UP: *opcaoMenu--;  if (*opcaoMenu < 0) *opcaoMenu = (SLOTS_CAIXA_RELOGIO - 1); break;
    default: if(WindowShouldClose() || IsKeyPressed(KEY_ESCAPE)) *exitWindowRequested = true; break;
    }

    //logica para saida
    voltarMenuRelogio(opcaoMenu, escolhaJanelas, exitWindowRequested);

    //verificacao se deseja mudar de menu
    if(IsKeyPressed(KEY_ENTER) || IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        switch (*opcaoMenu){
        case 0: *escolhaJanelas = MENU_CRONOMETRO; break;
        case 1: *escolhaJanelas = MENU_TEMPORIZADOR; break;
        case 2: *exitWindowRequested = true; break;
        default: break;
        }
        *opcaoMenu = DESATIVADO;
    }

}
void processaInputCronometro(Clock *cronometro, int* posicaoMouseCronometro, int* escolhaJanelas, int *opcaoMenuCronometro, bool* exitWindowBack){
    pegaPosicaoMouseSlotsRelogio(posicaoMouseCronometro, botoesRelogio, opcaoMenuCronometro); //logica do mouse
    //logica das teclas pressionadas
    int key = GetKeyPressed();
    switch (key){
    case KEY_ONE: *opcaoMenuCronometro = 0; break;
    case KEY_TWO: *opcaoMenuCronometro = 1; break;   
    case KEY_THREE: *opcaoMenuCronometro = 2; break;
    case KEY_SPACE: cronometro->pause = !cronometro->pause; *opcaoMenuCronometro = 1; break;
    case KEY_R: *opcaoMenuCronometro = 2; break;
    case KEY_RIGHT: *opcaoMenuCronometro++; if (*opcaoMenuCronometro > (SLOTS_CAIXA_RELOGIO - 1)) *opcaoMenuCronometro = 0; break;  
    case KEY_DOWN: *opcaoMenuCronometro--;  if (*opcaoMenuCronometro < 0) *opcaoMenuCronometro = (SLOTS_CAIXA_RELOGIO - 1); break;
    case KEY_ESCAPE: *exitWindowBack = true; break;
    default: if(WindowShouldClose()) exitWindow = true; break;
    }
    //logica para saida
    voltarMenuRelogio(opcaoMenuCronometro, escolhaJanelas, exitWindowBack); 
    
    if(IsKeyPressed(KEY_ENTER) || IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        switch (*opcaoMenuCronometro){
            case 0: cronometro->pause = false; break;
            case 1: cronometro->pause = !cronometro->pause; break;
            case 2: cronometro->timer = TEMPO_ZERADO; break;
            default: break;
        }
       *opcaoMenuCronometro = DESATIVADO; 
    } 
}
void logicaCronometro(Clock *cronometro,  int *opcaoMenuCronometro){
    // carrega tempo
    if (!cronometro->pause) {
        cronometro->timer += GetFrameTime();
    }
}
void processaInputTemporizador(Clock *temporizador, int *escolhaJanelas, bool* exitWindowBack, flagsAlarme *alarme, statusGUI *gui){
    pegaPosicaoMouseSlotsRelogio(&gui->posicaoMouseBotaoTemporizador, botoesRelogio, (&gui->indiceBotaoTemporizador));
 
    for (int i = 0; i < SLOTS_TEMPORIZADOR; i++) { //veficando o contato com mouse
        if (CheckCollisionPointRec(GetMousePosition(), caixasTempoPredefido[i])){ 
            gui->posicaoMouseBotaoTempoPredefinido = i;        
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) gui->indiceBotaoTempoDefinido = gui->posicaoMouseBotaoTempoPredefinido ;
            break;
        } else if(CheckCollisionPointRec(GetMousePosition(), caixasTemporizador[i])){
            gui->posicaoMouseBotaoAlteraTempo = i;        
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) gui->indiceBotaoAlteraTempo = gui->posicaoMouseBotaoAlteraTempo;
            break;
        } else {
            gui->posicaoMouseBotaoAlteraTempo = DESATIVADO;
            gui->posicaoMouseBotaoTempoPredefinido  = DESATIVADO;
            gui->indiceBotaoTempoDefinido  = DESATIVADO;
            gui->indiceBotaoAlteraTempo = DESATIVADO;
        } 
    }
    //logica das teclas pressionadas
    int key = GetKeyPressed();
    switch (key){
    case KEY_ONE: gui->indiceBotaoTemporizador = 0; break;
    case KEY_TWO: gui->indiceBotaoTemporizador = 1; break;
    case KEY_THREE: gui->indiceBotaoTemporizador = 2; break;
    case KEY_SPACE: temporizador->pause = !temporizador->pause; gui->indiceBotaoTemporizador = 1; break;
    case KEY_R: temporizador->timer = TEMPO_ZERADO; alarme->reset = true; break;
    case KEY_ESCAPE: *exitWindowBack = true; break;
    case KEY_RIGHT: gui->indiceBotaoTemporizador++; if (gui->indiceBotaoTemporizador > (SLOTS_CAIXA_RELOGIO - 1)) gui->indiceBotaoTemporizador = 0; break;
    case KEY_LEFT: gui->indiceBotaoTemporizador--; if (gui->indiceBotaoTemporizador < 0) gui->indiceBotaoTemporizador = (SLOTS_CAIXA_RELOGIO); break;
    case KEY_UP: temporizador->timer++; break;
    case KEY_DOWN: temporizador->timer--;  
    if (temporizador->timer < 0) temporizador->timer = 0; break;
    default: if(WindowShouldClose()) exitWindow = true;  break;
    }
     voltarMenuRelogio((&gui->indiceBotaoTemporizador), escolhaJanelas, exitWindowBack); 
    //executa as opcoes com base na opcao do usuario
    if(IsKeyPressed(KEY_ENTER) || IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        switch (gui->indiceBotaoTempoDefinido){
            case 0:  temporizador->timer = TEMPO_10_MIN; break;
            case 1:  temporizador->timer = TEMPO_25_MIN; break;
            case 2:  temporizador->timer = TEMPO_30_MIN; break; 
            case 3:  temporizador->timer = TEMPO_50_MIN; break;
            case 4:  temporizador->timer = TEMPO_90_MIN; break;
            case 5:  temporizador->timer = TEMPO_120_MIN; break; 
            default: gui->indiceBotaoTempoDefinido = DESATIVADO; break;
        }
        gui->indiceBotaoTempoDefinido = DESATIVADO;
        switch (gui->indiceBotaoAlteraTempo){
            case 0:  temporizador->timer -= TEMPO_30_SEG; break;
            case 1:  temporizador->timer -= TEMPO_1_MIN; break;
            case 2:  temporizador->timer -= TEMPO_5_MIN; break; 
            case 3:  temporizador->timer += TEMPO_30_SEG;  alarme->reset = true; break;
            case 4:  temporizador->timer += TEMPO_1_MIN;  alarme->reset = true;  break;
            case 5:  temporizador->timer += TEMPO_5_MIN; alarme->reset = true; break; 
            default: gui->indiceBotaoAlteraTempo = DESATIVADO; break;
        }
        gui->indiceBotaoAlteraTempo = DESATIVADO;
        switch (gui->indiceBotaoTemporizador){
            case 0: temporizador->pause = false; break;
            case 1: temporizador->pause = !temporizador->pause; break;
            case 2: temporizador->timer = TEMPO_ZERADO; alarme->reset = true; break;
            default: gui->indiceBotaoTemporizador = DESATIVADO; alarme->reset = false; break;
        }
        gui->indiceBotaoTemporizador = DESATIVADO; 
    } 

}
void logicaTemporizador(Clock *temporizador, int *escolhaJanelas, alarmeAtivado *alarmeAtivo, flagsAlarme *alarme, Sound somTemporizador){

    //logica do temporizador propriamente
    if(alarme->modoAlarme){
        if (GetKeyPressed() != 0 || IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            alarme->modoAlarme = false;     // sai do modo alarme
            StopSound(somTemporizador);       // para o som
            alarmeAtivo->textoVisivel = true;    //o texto volta a ficar visível fixo
            alarme->alarmeTocou = false;    // Reseta flag de som
            temporizador->timer = TEMPO_ZERADO; //zera o tempo
            temporizador->pause = true;           // Pausa o sistema para recomeçar
            
        } else { //alarme rodando
            alarmeAtivo->tempoPiscar += GetFrameTime();
            if (alarmeAtivo->tempoPiscar >= 0.5f) {
                alarmeAtivo->tempoPiscar = TEMPO_ZERADO;
                alarmeAtivo->textoVisivel = !alarmeAtivo->textoVisivel;
            }
            alarmeAtivo->tempoCorrido += GetFrameTime();
            if (alarmeAtivo->tempoCorrido < TEMPO_MUSICA_TOCANDO) {
                if (!IsSoundPlaying(somTemporizador) && !alarme->alarmeTocou) {
                    PlaySound(somTemporizador);
                    alarme->alarmeTocou = true; // Marca que já deu play
                }
            } else {
                // Passou de 10 segundos, para o som
                StopSound(somTemporizador); 
            }
       }
    } else if (!temporizador->pause) {
        temporizador->timer -= GetFrameTime();
        // Verificação se o tempo acabou
        if (temporizador->timer <= 0) {
            temporizador->timer = TEMPO_ZERADO;
            // Entra no Modo Alarme se a tecla reset não tiver sido pressionada
            if(!alarme->reset){
                alarme->modoAlarme = true;  
                alarmeAtivo->tempoCorrido = TEMPO_ZERADO; // Zera o contador da musica
                alarmeAtivo->tempoPiscar = TEMPO_ZERADO; // Zera o contador do piscar
                alarme->alarmeTocou = false;
            }
        }
    }
    
}

void desenhaLayoutRelogio(Vector2 *posicaoRelogio, const short posY1, const short posY2, short posX, const short tamanho, const short aumentaX, const short diminuiRaio){
    //funcao que desenha os circulos do cronometro e temporizador
    for(int i = 0; i < SLOTS_CAIXA_RELOGIO; i++){
        DrawRingLines(posicaoRelogio[i], (RAIO_INTERNO - diminuiRaio), (RAIO_EXTERNO - diminuiRaio), ANGULO_INICIAL, ANGULO_FINAL, LADOS, GOLD);
        DrawRing(posicaoRelogio[i], (RAIO_INTERNO - diminuiRaio), (RAIO_EXTERNO - diminuiRaio), ANGULO_INICIAL, ANGULO_FINAL, LADOS, GOLD);
    }
    for(int i = 0; i < 2; i++){
        DrawCircle(posX, posY1, tamanho,WHITE); 
        DrawCircle(posX, posY2, tamanho,WHITE);
        posX+=aumentaX;
    }
}
void desenhaBotoesRelogio(Rectangle *botoes, short opcao, short posicaoMouse){ //funcao que desenha os botoes do menu
    for(int i = 0; i < SLOTS_CAIXA_RELOGIO; i++){
        if(i == (SLOTS_CAIXA_RELOGIO - 1)){
            DrawRectangleRec(botoes[i],((i == opcao) || (i == posicaoMouse)) ? MAROON : LIGHTGRAY);
            DrawRectangleLines((int)botoes[i].x, (int) botoes[i].y, (int) botoes[i].width, (int) botoes[i].height, ((i == opcao) || (i == posicaoMouse)) ? BLACK : GRAY);
            DrawText(textoCronometro[i], (int)(botoes[i].x + botoes[i].width/3 - MeasureText(textoMenu[i],  30)/2), (int)botoes[i].y + 20, 35, ((i == opcao) || (i == posicaoMouse)) ? BLACK : DARKGRAY);
        } else {
            DrawRectangleRec(botoes[i],((i == opcao) || (i == posicaoMouse)) ? SKYBLUE : LIGHTGRAY);
            DrawRectangleLines((int)botoes[i].x, (int) botoes[i].y, (int) botoes[i].width, (int) botoes[i].height, ((i == opcao) || (i == posicaoMouse)) ? BLUE : GRAY);
            DrawText(textoCronometro[i], (int)(botoes[i].x + botoes[i].width/3 - MeasureText(textoCronometro[i], 20)/2), (int)botoes[i].y + 20, 35, ((i == opcao) || (i == posicaoMouse)) ? DARKBLUE : DARKGRAY);
        }
    }
}
void desenhaMensagemRetornoJanela(bool sairJanela, Color cor, const char *text){ //desenha a mensagem de retorno ou saida do relogio
    if (sairJanela){
        DrawRectangle(0, 100, screenWidth, 200, cor);
        DrawText(text, 40, 180, 30, WHITE);
    }
}
void desenhaLetrasRelogio(const short posX_H, const short posX_M, const short posX_S, short diminuiTamanho){ //desenha as letras do cronometro e relogio
    DrawText("H", posX_H, 360, (50 - diminuiTamanho), WHITE);
    DrawText("MIN", posX_M, 360, (50 - diminuiTamanho), WHITE);
    DrawText("SEC", posX_S, 360, (50 - diminuiTamanho), WHITE);
}
void desenhaMenuRelogio(int posicaoMouseMenu, int opcaoMenu, bool exitWindowRequested){
    ClearBackground(DARKBROWN);

    // desenho do retangulo com o nome
    DrawRectangleLines(210, 20, 337, 125, GRAY);
    DrawText("TIMER", 252, 50, 75, GOLD);

    //desenha os botoes do menu inicial
    for(int i = 0; i < SLOTS_CAIXA_RELOGIO; i++){
        if(i == (SLOTS_CAIXA_RELOGIO - 1)){
            DrawRectangleRec(caixasMenu[i],((i == opcaoMenu) || (i == posicaoMouseMenu)) ? MAROON : LIGHTGRAY);
            DrawRectangleLines((int)caixasMenu[i].x, (int) caixasMenu[i].y, (int) caixasMenu[i].width, (int) caixasMenu[i].height, ((i == opcaoMenu) || (i == posicaoMouseMenu)) ? RED : GRAY);
            DrawText(textoMenu[i], (int)(caixasMenu[i].x + caixasMenu[i].width/3 - MeasureText(textoMenu[i], 58)/2), (int)caixasMenu[i].y + 20, 35, ((i == opcaoMenu) || (i == posicaoMouseMenu)) ? BLACK : DARKGRAY);
        } else {
            DrawRectangleRec(caixasMenu[i],((i == opcaoMenu) || (i == posicaoMouseMenu)) ? SKYBLUE : LIGHTGRAY);
            DrawRectangleLines((int)caixasMenu[i].x, (int) caixasMenu[i].y, (int) caixasMenu[i].width, (int) caixasMenu[i].height, ((i == opcaoMenu) || (i == posicaoMouseMenu)) ? BLUE : GRAY);
            DrawText(textoMenu[i], ((i != 1) ? (int)(caixasMenu[i].x + caixasMenu[i].width/3 - MeasureText(textoMenu[i], 25)/2) : (int)(caixasMenu[i].x + caixasMenu[i].width/3 - MeasureText(textoMenu[i], 22)/2)),
            (int)caixasMenu[i].y + 20, 35, ((i == opcaoMenu) || (i == posicaoMouseMenu)) ? DARKBLUE : DARKGRAY);
        }
    }
    //desenha mensagem de saida, caso seja o caso
    desenhaMensagemRetornoJanela(exitWindowRequested, RED,  TEXTO_SAIR_RELOGIO);
}
void desenhaCronometro(Clock *cronometro, int posicaoMouseCronometro, int opcaoMenuCronometro, bool exitWindowBack){ //desenha o menu do cronometro
    ClearBackground(DARKGRAY);

    //desenho do retangulo com o nome
    DrawRectangleLines(100, 20, 600, 125, LIGHTGRAY);
    DrawText("CRONOMETRO", 130, 50, 75, GOLD);

    desenhaLayoutRelogio(posicaoCirculoCronometro, 230, 320, 290, 10, 200, 0); 

    desenhaLetrasRelogio(175, 350, 550, 0); 

    desenhaBotoesRelogio(botoesRelogio, opcaoMenuCronometro, posicaoMouseCronometro);

    //desenho dos cronometro propriamente
    int segundos, minutos, horas;
    processaTempo(cronometro, &segundos, &minutos, &horas);
    const char *clockTime = TextFormat("%02i  %02i  %02i ", horas, minutos, segundos);
    DrawText(clockTime, 130, 225, 100, WHITE);
    //desenha mensagem de saida, caso seja o caso
    desenhaMensagemRetornoJanela(exitWindowBack, BLACK, TEXTO_RETORNO_JANELA);
}
void desenhaTemporizador(Clock *temporizador, statusGUI gui, bool textoVisivel, bool exitWindowBack){
    ClearBackground(DARKGRAY);

    //desenho do retangulo com o nome 
    DrawRectangleLines(90, 20, 630, 120, LIGHTGRAY);
    DrawText("TEMPORIZADOR", 100, 50, 75, GOLD);

    DrawRectangleLines(165, 200, 480, 230, LIGHTGRAY);

    desenhaLayoutRelogio(posicaoCirciculoTemporizador, 250,  330, 325, 8, 150, 15);

    desenhaLetrasRelogio(235, 360, 510, 10);

    desenhaBotoesRelogio(botoesRelogio, gui.indiceBotaoTemporizador, gui.posicaoMouseBotaoTemporizador);

    for(int i  = 0; i < SLOTS_TEMPORIZADOR; i++){
        //desenho dos botoes com tempos predefinidos 
        DrawRectangleRec(caixasTempoPredefido[i],((i == gui.indiceBotaoTempoDefinido) || (i == gui.posicaoMouseBotaoTempoPredefinido)) ? SKYBLUE : LIGHTGRAY);
        DrawRectangleLines((int)caixasTempoPredefido[i].x, (int)caixasTempoPredefido[i].y, (int)caixasTempoPredefido[i].width, (int)caixasTempoPredefido[i].height, ((i == gui.indiceBotaoTempoDefinido) || (i == gui.posicaoMouseBotaoTempoPredefinido)) ? BLUE : GRAY);
        DrawText(textoSlotsTemporizador[i], (int)(caixasTempoPredefido[i].x +caixasTempoPredefido[i].width/3 - MeasureText(textoSlotsTemporizador[i], 20)/2), (int)caixasTempoPredefido[i].y + 10, 30, ((i == gui.indiceBotaoTempoDefinido) || (i == gui.posicaoMouseBotaoTempoPredefinido )) ? DARKBLUE : DARKGRAY);
        //desenho dos botoes que incrementa ou decrementa o tempo
        DrawRectangleRec(caixasTemporizador[i],((i == gui.indiceBotaoAlteraTempo) || (i == gui.posicaoMouseBotaoAlteraTempo)) ? SKYBLUE : LIGHTGRAY);
        DrawRectangleLines((int) caixasTemporizador[i].x, (int) caixasTemporizador[i].y, (int) caixasTemporizador[i].width, (int) caixasTemporizador[i].height, ((i == gui.indiceBotaoAlteraTempo) || (i == gui.posicaoMouseBotaoAlteraTempo)) ? BLUE : GRAY);
        DrawText(textoTempoTemporizador[i], (int)(caixasTemporizador[i].x + caixasTemporizador[i].width/3 - MeasureText(textoTempoTemporizador[i], 20)/2), (int)caixasTemporizador[i].y + 10, 27, ((i == gui.indiceBotaoAlteraTempo) || (i == gui.posicaoMouseBotaoAlteraTempo)) ? DARKBLUE : DARKGRAY);       
    }

    //desenho dos temporizador propriamente
    int segundos, minutos, horas;
    processaTempo(temporizador, &segundos, &minutos, &horas);
    
    const char *clockTime = TextFormat("%02i   %02i   %02i", horas, minutos, segundos);
    if(textoVisivel) DrawText(clockTime, 219, 260, 65, WHITE);
    //desenha mensagem de saida, caso seja o caso
    desenhaMensagemRetornoJanela(exitWindowBack, BLACK, TEXTO_RETORNO_JANELA);

}

void salvaDadosRelogio(const char* nomeArquivo, estadoAplicativo *relogio){ //funcao que salva os dados do relogio
    FILE *fp = fopen(nomeArquivo, "wb");
    if(fp == NULL){
        perror("Arquivo nao criado!!");
        return;
    }
    fwrite(relogio, sizeof(estadoAplicativo), 1, fp);
    fclose(fp);
    
}
int LerDadosRelogio(const char* nomeArquivo, estadoAplicativo *relogio){ //funcao que carrega os dados do relogio
    FILE *fp = fopen(nomeArquivo, "rb");

    if(fp == NULL){
        perror("Erro ao ler o arquivo!!");
        return 0;
    } 

    fread(relogio, sizeof(estadoAplicativo), 1, fp);

    fclose(fp);
    return 1;
}