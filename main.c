#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define tamanhoCobra 301
#define tamanhoQuadrado 31
#define MAX_SCORE 3
#define MAX_NAME 4

typedef struct Cobrinha {
  Vector2 posicao;
  Vector2 tamanho;
  Vector2 velocidade;
  Color cor;
} Cobrinha;

typedef struct Comida {
  Vector2 posicao;
  Vector2 tamanho;
  bool ativa;
  Color cor;
} Comida;

typedef struct Aguia {
  Vector2 posicao;
  bool ativo;
  float alphaAguia;
  Color cor;
} Aguia;

typedef struct
{
  char playername[MAX_NAME];
  int score;
  float tempo;
  int cond;
} ParametrosPlacar;

const int larguraTela = 1280;
const int alturaTela = 720;

int contadorFrames = 0;
bool gameOver = false;
bool pausado = false;

Comida comida = { 0 };
Cobrinha cobrinha[tamanhoCobra] = { 0 };
Vector2 posicaoCobrinha[tamanhoCobra] = { 0 };
bool permitirMover = false;
Vector2 deslocamento = { 0 };
int tamanhoCauda = 0;
Aguia aguia;
ParametrosPlacar placar[MAX_SCORE] = {0};

typedef enum { MENU, JOGO, GAMEOVER, CADASTRO } EstadoJogo;
int estadoJogo = MENU;

int ponto;
int tempo;
int tempoAguia;
int placarvisivel = 0;

void IniciarJogo(void);
void AtualizarJogo(void);
void DesenharJogo(void);
void DesenharPlacar(int x, int y);
void DescarregarJogo(void);
void AtualizarPlacar(const char* name, int score, float tempo);

Music musicaJogo;
Music musicaMenu;
Texture2D cabecinha;
Texture2D corpinho;
Texture2D texturaComida;
Texture2D mapa;
Texture2D imagemFundo;
Texture2D imagemGameOver;
Texture2D aguiaTextura;
Sound somComer;
Sound somMorrer;
Sound somMover;
Sound somDano;

int main(void){

  InitWindow(larguraTela, alturaTela, "Jogo da Cobrinha");
  InitAudioDevice();
  texturaComida = LoadTexture("Graphics/comida.png");
  cabecinha = LoadTexture("Graphics/cabeca.png");
  corpinho = LoadTexture("Graphics/corpo.png");
  mapa = LoadTexture("Graphics/Mapa.png");
  imagemFundo = LoadTexture("Graphics/Menuf.png");
  imagemGameOver = LoadTexture("Graphics/Menug.png");
  somComer = LoadSound("Sound/coleta.mp3");
  somMorrer = LoadSound("Sound/gameOver.mp3");
  somDano = LoadSound("Sound/ouch.mp3");
  somMover = LoadSound("Sound/Move.wav");
  musicaMenu = LoadMusicStream("Sound/menu.mp3");
  musicaJogo = LoadMusicStream("Sound/AnVeganSnakeHungry4Apples.mp3");
  aguiaTextura = LoadTexture("Graphics/aguia.png");
  PlayMusicStream(musicaMenu);

  IniciarJogo();
  SetTargetFPS(60);

  while (!WindowShouldClose())
  {
    switch (estadoJogo)
    {
      case MENU:
        UpdateMusicStream(musicaMenu);

        if(IsKeyPressed(KEY_B))
        {
          placarvisivel = !placarvisivel;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(imagemFundo, 0, 0, WHITE);
        
        if(placarvisivel)
        {
          DrawRectangle(15, 15, 350, 140, Fade(DARKGRAY, 0.85f));
          DesenharPlacar(30,30);
        }
        EndDrawing();

        if (IsKeyPressed(KEY_SPACE)) 
        {
          estadoJogo = JOGO;
          IniciarJogo();
          StopMusicStream(musicaMenu);
          PlayMusicStream(musicaJogo);
        }
        break;

      case JOGO:
        AtualizarJogo();
        DesenharJogo();
        UpdateMusicStream(musicaJogo);
        if (gameOver)
        {
          StopMusicStream(musicaJogo);
          estadoJogo = CADASTRO;
        }
        break;

      case GAMEOVER:
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(imagemGameOver, 0, 0, WHITE);
        EndDrawing();

        if (IsKeyPressed(KEY_SPACE)) 
        {
        estadoJogo = MENU;
        PlayMusicStream(musicaMenu);
        }
        break;
      case CADASTRO:
        static char nomejogador[MAX_NAME] = "";
        static int nomeLen = 0;

        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Digite seu nome (ate 3 letras):", 400, 300, 30, WHITE);
        DrawText(nomejogador, 400, 350, 40, YELLOW);
        EndDrawing();

        int key = GetCharPressed();
        if(key >= 32 && key <= 125 && nomeLen < MAX_NAME-1)
        {
          nomejogador[nomeLen++] = (char)key;
          nomejogador[nomeLen] = '\0';
        }
        if(IsKeyPressed(KEY_BACKSPACE) && nomeLen > 0)
        {
          nomejogador[--nomeLen] = '\0';
        }
        if(IsKeyPressed(KEY_ENTER) && nomeLen > 0)
        {
          AtualizarPlacar(nomejogador, ponto, (float)tempo);
          nomeLen = 0;
          nomejogador[0] = '\0';
          estadoJogo = GAMEOVER;
        }
        
    }
    UpdateMusicStream(musicaMenu);
  }

  DescarregarJogo();
  CloseWindow();
  return 0;
}

void IniciarJogo(void)
{
  contadorFrames = 0;
  gameOver = false;
  pausado = false;

  ponto = 0;
  tempo = 0;
  tempoAguia = 0;

  tamanhoCauda = 1;
  permitirMover = false;

  aguia.ativo = true;
  aguia.posicao = (Vector2){0,0};
  aguia.alphaAguia = 1.0f;

  deslocamento.x = larguraTela%tamanhoQuadrado;
  deslocamento.y = alturaTela%tamanhoQuadrado;

  for (int i = 0; i < tamanhoCobra; i++)
  {
    cobrinha[i].posicao = (Vector2){ deslocamento.x/2, deslocamento.y/2 };
    cobrinha[i].tamanho = (Vector2){ tamanhoQuadrado, tamanhoQuadrado };
    cobrinha[i].velocidade = (Vector2){ tamanhoQuadrado, 0 };

    if (i == 0) cobrinha[i].cor = DARKBLUE;
    else cobrinha[i].cor = BLUE;
  }

  for (int i = 0; i < tamanhoCobra; i++)
  {
    posicaoCobrinha[i] = (Vector2){ 0.0f, 0.0f };
  }

  comida.tamanho = (Vector2){ tamanhoQuadrado, tamanhoQuadrado };
  comida.cor = WHITE;
  comida.ativa = false;
}

void AtualizarJogo(void)
{
  if (!gameOver)
  {
    if (IsKeyPressed('P')) pausado = !pausado;

    if (!pausado)
    {
      if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && (cobrinha[0].velocidade.x == 0) && permitirMover)
      {
        cobrinha[0].velocidade = (Vector2){ tamanhoQuadrado, 0 };
        permitirMover = false;
        PlaySound(somMover);
      }
      if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && (cobrinha[0].velocidade.x == 0) && permitirMover)
      {
        cobrinha[0].velocidade = (Vector2){ -tamanhoQuadrado, 0 };
        permitirMover = false;
        PlaySound(somMover);
      }
        if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && (cobrinha[0].velocidade.y == 0) && permitirMover)
      {
        cobrinha[0].velocidade = (Vector2){ 0, -tamanhoQuadrado };
        permitirMover = false;
        PlaySound(somMover);
      }
      if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && (cobrinha[0].velocidade.y == 0) && permitirMover)
      {
        cobrinha[0].velocidade = (Vector2){ 0, tamanhoQuadrado };
        permitirMover = false;
        PlaySound(somMover);
      }

      for (int i = 0; i < tamanhoCauda; i++) posicaoCobrinha[i] = cobrinha[i].posicao;

      if ((contadorFrames%5) == 0)
      {
        for (int i = 0; i < tamanhoCauda; i++)
        {
          if (i == 0)
          {
            cobrinha[0].posicao.x += cobrinha[0].velocidade.x;
            cobrinha[0].posicao.y += cobrinha[0].velocidade.y;
            permitirMover = true;
          }
            else cobrinha[i].posicao = posicaoCobrinha[i-1];
        }
      }

      if (((cobrinha[0].posicao.x) > (larguraTela - deslocamento.x)) || ((cobrinha[0].posicao.y) > (alturaTela - deslocamento.y)) || (cobrinha[0].posicao.x < 0) || (cobrinha[0].posicao.y < 0))
      {
        if (!gameOver)
        {
          PlaySound(somMorrer);
          PlaySound(somDano);
        }
        gameOver = true;
      }

      for (int i = 1; i < tamanhoCauda; i++)
      {
        if ((cobrinha[0].posicao.x == cobrinha[i].posicao.x) && (cobrinha[0].posicao.y == cobrinha[i].posicao.y)) 
        {
          if (!gameOver) PlaySound(somMorrer);
          gameOver = true;
        }
      }

      if (!comida.ativa)
      {
        comida.ativa = true;
        comida.posicao = (Vector2){ GetRandomValue(0, (larguraTela/tamanhoQuadrado) - 1)*tamanhoQuadrado + deslocamento.x/2, GetRandomValue(0, (alturaTela/tamanhoQuadrado) - 1)*tamanhoQuadrado + deslocamento.y/2 };

        for (int i = 0; i < tamanhoCauda; i++)
        {
          while ((comida.posicao.x == cobrinha[i].posicao.x) && (comida.posicao.y == cobrinha[i].posicao.y))
          {
            comida.posicao = (Vector2){ GetRandomValue(0, (larguraTela/tamanhoQuadrado) - 1)*tamanhoQuadrado + deslocamento.x/2, GetRandomValue(0, (alturaTela/tamanhoQuadrado) - 1)*tamanhoQuadrado + deslocamento.y/2 };
            i = 0;
          }
        }
      }

      if ((cobrinha[0].posicao.x < (comida.posicao.x + comida.tamanho.x) && (cobrinha[0].posicao.x + cobrinha[0].tamanho.x) > comida.posicao.x) && (cobrinha[0].posicao.y < (comida.posicao.y + comida.tamanho.y) && (cobrinha[0].posicao.y + cobrinha[0].tamanho.y) > comida.posicao.y))
      {
        cobrinha[tamanhoCauda].posicao = posicaoCobrinha[tamanhoCauda - 1];
        tamanhoCauda += 1;
        comida.ativa = false;
        PlaySound(somComer);
        ponto++;
      }

      contadorFrames++;
      if(contadorFrames % 60 == 0) tempo++;
    }
  } 
  else
  {
    if (IsKeyPressed(KEY_ENTER))
    {
      IniciarJogo();
      gameOver = false;
    }
  }
  
  if(aguia.ativo){
    aguia.posicao.x -= 5;
    aguia.posicao.y = 250;
    if(aguia.posicao.x + aguiaTextura.width * 0.5f < 0){
      aguia.ativo = false;
      tempoAguia = 600;
    }
  } else {
    if(tempoAguia > 0) tempoAguia--;
    else {
      aguia.ativo = true;
      aguia.posicao.x = larguraTela;
      aguia.posicao.y = GetRandomValue(0, alturaTela - aguiaTextura.height);
      aguia.alphaAguia = 0.5f;
    }
  } 

}

void DesenharJogo(void)
{
    BeginDrawing();
    
    ClearBackground(RAYWHITE);

    DrawTexture(mapa,0,0,WHITE);

    aguia.alphaAguia = 0.5f;

    if (!gameOver)
    {
        
        /*for (int i = 0; i < larguraTela / tamanhoQuadrado + 1; i++)
        {
            DrawLineV((Vector2){tamanhoQuadrado * i + deslocamento.x / 2, deslocamento.y / 2},
                      (Vector2){tamanhoQuadrado * i + deslocamento.x / 2, alturaTela - deslocamento.y / 2}, LIGHTGRAY);
        }

        for (int i = 0; i < alturaTela / tamanhoQuadrado + 1; i++)
        {
            DrawLineV((Vector2){deslocamento.x / 2, tamanhoQuadrado * i + deslocamento.y / 2},
                      (Vector2){larguraTela - deslocamento.x / 2, tamanhoQuadrado * i + deslocamento.y / 2}, LIGHTGRAY);
        }*/

        
        for (int i = 0; i < tamanhoCauda; i++)
        {
            Rectangle destino = {
                cobrinha[i].posicao.x,
                cobrinha[i].posicao.y,
                tamanhoQuadrado,
                tamanhoQuadrado
            };

            Rectangle origem = {0, 0, (float)tamanhoQuadrado, (float)tamanhoQuadrado};
            Vector2 origemRotacao = {tamanhoQuadrado / 2.0f, tamanhoQuadrado / 2.0f };
            float angulo = 0.0f;

            if (i == 0) // Cabeça da cobra
            {
              // Lógica de rotação da cabeça (já existente)
              if (cobrinha[0].velocidade.x > 0 ) angulo = 0.0f;
              else if (cobrinha[0].velocidade.x < 0) angulo = 180.0f;
              else if (cobrinha[0].velocidade.y < 0) angulo = 270.0f;
              else if (cobrinha[0].velocidade.y > 0) angulo = 90.0f; 

              Rectangle destinoRot = {
                cobrinha[0].posicao.x + origemRotacao.x,
                cobrinha[0].posicao.y + origemRotacao.y,
                tamanhoQuadrado,
                tamanhoQuadrado
              };

              DrawTexturePro(cabecinha, origem, destinoRot, origemRotacao, angulo , WHITE);
            }
            else 
            {
                
              
                Vector2 direcaoSegmento;
                if (i > 0) {
                    direcaoSegmento.x = cobrinha[i].posicao.x - cobrinha[i-1].posicao.x;
                    direcaoSegmento.y = cobrinha[i].posicao.y - cobrinha[i-1].posicao.y;
                } else { 
                    direcaoSegmento.x = cobrinha[0].posicao.x - posicaoCobrinha[0].x;
                    direcaoSegmento.y = cobrinha[0].posicao.y - posicaoCobrinha[0].y;
                }

                
                if (direcaoSegmento.x > 0) angulo = 0.0f;    
                else if (direcaoSegmento.x < 0) angulo = 180.0f; 
                else if (direcaoSegmento.y < 0) angulo = 270.0f; 
                else if (direcaoSegmento.y > 0) angulo = 90.0f;  
                

                Rectangle destinoRot = {
                  cobrinha[i].posicao.x + origemRotacao.x,
                  cobrinha[i].posicao.y + origemRotacao.y,
                  tamanhoQuadrado,
                  tamanhoQuadrado
                };
                DrawTexturePro(corpinho, origem, destinoRot, origemRotacao, angulo, WHITE);
            }
        }

        
        DrawTextureV(texturaComida, comida.posicao, WHITE);

        
        if (pausado)
            DrawText("PAUSA", larguraTela / 2 - MeasureText("PAUSA", 40) / 2, alturaTela / 2 - 40, 40, GRAY);

        char textoPontos[32];
        sprintf(textoPontos, "Pontos: %d", ponto);

        char textoTempo[32];
        sprintf(textoTempo, "Tempo: %02d:%02d", tempo/60, tempo%60);

        DrawText(textoPontos, 20, 20, 30, WHITE);
        DrawText(textoTempo, 200, 20, 30, WHITE);
    }
    else
    {
        DrawText("PRESSIONE [ESPACO] PARA CONTINUAR", GetScreenWidth() / 2 - MeasureText("PRESSIONE [ESPACO] PARA CONTINUAR", 20) / 2,
                 GetScreenHeight() / 2 - 50, 20, GRAY);
    }
   if(aguia.ativo){
    aguia.cor = WHITE;
    aguia.cor.a = (unsigned char)(aguia.alphaAguia * 255);
    DrawTexture(aguiaTextura, aguia.posicao.x, aguia.posicao.y, aguia.cor);
   }
    EndDrawing();
}

void DesenharPlacar(int x, int y)
{
  DrawText("Placar:", x, y, 20, YELLOW);
  for(int i=0; i<MAX_SCORE; i++)
  {
    char buffer[64];
    if(placar[i].cond)
    {
      snprintf(buffer, sizeof(buffer), "%d. %s - %d pts - %.1fs", i+1, placar[i].playername, placar[i].score, placar[i].tempo);
    }
    else
    {
      snprintf(buffer, sizeof(buffer), "%d. vazio", i+1);
    }
    DrawText(buffer, x, y + 30 + i*30, 18, LIGHTGRAY);
  }
}

void DescarregarJogo(void)
{
  UnloadTexture(texturaComida);
  UnloadMusicStream(musicaMenu);
  UnloadMusicStream(musicaJogo);
  UnloadTexture(imagemFundo);
  UnloadTexture(imagemGameOver);
  UnloadTexture(aguiaTextura);
  UnloadSound(somComer);
  UnloadSound(somMorrer);
  UnloadSound(somMover);
  CloseAudioDevice();
}

void AtualizarPlacar (const char* name, int score, float tempo)
{
  ParametrosPlacar new;
  strncpy(new.playername, name, MAX_NAME-1);
  new.playername[MAX_NAME-1] = '\0';
  new.score = score;
  new.tempo = tempo;
  new.cond = 1;
  int inserirIdx = -1;
  for(int i = 0; i < MAX_SCORE; i++)
  {
    if(!placar[i].cond || score > placar[i].score)
    {
capotagem:      inserirIdx = i;
      break;
    }
  }
  if(inserirIdx == -1)
  {
    return;
  }
  for(int i = MAX_SCORE - 1; i>inserirIdx; i--)
  {
    placar[i] = placar[i-1];
  }
  placar[inserirIdx] = new;
}