#include "PIG.h"

/*****
Definições e enumerações
*****/

//Definições de cores adicionais
#define AZUL_PISCINA ((PIG_Cor){108,255,255,255})
#define VERMELHO_SANGUE ((PIG_Cor){178,0,0,255})
#define VERDE_ESCURO ((PIG_Cor){35,140,0,255})
#define AZUL_CLARO ((PIG_Cor){0,178,178,255})
#define LARANJA_FOSCO ((PIG_Cor){140,70,0,255})
#define CINZA_ESCURO ((PIG_Cor){49,49,64,255})
#define CINZA_CLARO ((PIG_Cor) {240,240,225,255})

//Definições dos possíveis quadros
#define NORMAL 1
#define QD2 2

//enumeração dos sinais vitais
enum sinaisVitais{HR, BP1, BP2, SPO2, RESP, TEMP};

enum telas{tFormulario, tMonitor};

/*****
Estruturas de dados
*****/

typedef struct CaixadeTexto
{
  int x, y, alt, larg, sel, tamMax, numerico;
  char *texto;
}CaixadeTexto;


typedef struct agendamento{
  int qdr, ti, dur;
  struct agendamento *prox;
}Agendamento;

typedef struct grafico{
  int x, y;
  struct grafico *prox;
} Grafico;

typedef struct sinalVital{
  int valor, timerGrafico;
  Grafico *g;
}sinalVital;

typedef struct Paciente{
  char nome[50], diagMed[50], diagEnf[50], quartoLeito[20], sexo, evolucao[100];
  int idade, diasInter;
  int quadro;
  sinalVital sinais[6];
}Paciente;

/*****
Variáveis globais
*****/

PIG_Evento evento;
PIG_Teclado meuTeclado;

int TELA= tFormulario;

//Fila de agendamento
Agendamento *filaAgd=NULL;

//Paciente
Paciente p;

//Timers
int timerIniSim, timerDurSim;

//Fontes
int fntHr, fntBp, fntSpo2, fntResp, fntTemp, fntPainel, fntForm;

/*****
Funções de manipulção das estruturas de dados
*****/


CaixadeTexto *criarCaixadeTexto(int x,int y,int alt,int larg,int tamMax, int numerico = 0)
{
  CaixadeTexto *novo = (CaixadeTexto*)malloc(sizeof(CaixadeTexto));
  novo->x=x;
  novo->y=y;
  novo->alt=alt;
  novo->larg=larg;
  novo->sel=0;
  novo->numerico=numerico;
  novo->tamMax=tamMax;
  novo->texto="";
  return novo;
}

int CaixadeTextoClicada(int x,int y,int alt,int larg)
{
    int x_mouse=evento.mouse.posX;
    int y_mouse=evento.mouse.posY;
    if(x_mouse >= x && x_mouse <= (x+larg) && y_mouse >= y && y_mouse<=(y+alt))
        {
            return 1;
        }
    return 0;
}

void selecionarCaixadeTexto(CaixadeTexto **inputs, int numInputs)
{
  if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao==MOUSE_ESQUERDO)
    {
      int i;
      for(i=0;i<numInputs;i++)
        {
          inputs[i]->sel = CaixadeTextoClicada(inputs[i]->x,inputs[i]->y,inputs[i]->alt,inputs[i]->larg);
        }
    }
}

char *getinput(CaixadeTexto *input)
{
    char *texto = (char*)malloc(sizeof(char)*input->tamMax);
    sprintf(texto,"");
    if(input->texto!=NULL)
    {
        sprintf(texto,"%s",input->texto);
        free(input->texto);
    }
    if(evento.tipoEvento==EVENTO_TECLADO & evento.teclado.acao==TECLA_PRESSIONADA){
                  int i = evento.teclado.tecla;
                  if(strlen(texto)<input->tamMax){
                  if(i>=TECLA_a && i<=TECLA_z && !input->numerico){
                      i =i + 'a' - TECLA_a;
                      sprintf(texto,"%s%c",texto,i);
                  } else if(i==TECLA_BARRAESPACO && !input->numerico)
                    {
                        sprintf(texto,"%s ",texto);
                    }else if(i>=TECLA_1 && i<= TECLA_0)
                    {
                      if( i!= TECLA_0)
                        i =i - TECLA_1 + '0' +1;
                      else
                        i= 48;
                      sprintf(texto,"%s%c",texto,i);
                    }else if(i>=TECLA_KP_1 && i<= TECLA_KP_0)
                    {
                      if( i!= TECLA_KP_0)
                        i =i - TECLA_KP_1 + '0' +1;
                      else
                        i= 48;
                      sprintf(texto,"%s%c",texto,i);
                    }
                  }
                  if(i==TECLA_BACKSPACE && strlen(texto)>0)
                    {
                        char *aux = (char*)malloc(sizeof(char)*input->tamMax);
                        strncpy(aux,texto,(strlen(texto)-1));
                        aux[strlen(texto)-1]='\0';
                        strcpy(texto,aux);
                        free(aux);
                    }
        }
    return texto;
}

void desenhaCaixadeTexto(CaixadeTexto *input) {
  DesenhaRetanguloVazado(input->x,input->y,input->alt,input->larg,CINZA);
  if (input->sel) {
    DesenhaRetanguloVazado(input->x,input->y,input->alt,input->larg,AZUL);
    input->texto = getinput(input);
  }
  if (input->texto!=NULL)
    EscreverEsquerda(input->texto,input->x+3,input->y+input->alt-30,fntForm);
}

Grafico *pushPonto(Grafico *l,int x,int y)
{
    if (l==NULL)
    {
      Grafico *novo = (Grafico*)malloc(sizeof(Grafico));
      novo->x = x;
      novo->y = y;
      novo->prox=NULL;
      return novo;
    } else {
      l->prox = pushPonto(l->prox,x,y);
      return l;
    }
}

Agendamento *pushAgendamento(Agendamento *l,int quadro,int tempo_ini,int duracao)
{
    if(l==NULL)
    {
        Agendamento *novo = (Agendamento*)malloc(sizeof(Agendamento));
        novo->dur=duracao;
        novo->ti=tempo_ini;
        novo->qdr = quadro;
        novo->prox = NULL;
        return novo;
    }else
    {
        l->prox = pushAgendamento(l->prox,quadro,tempo_ini,duracao);
        return l;
    }
}

Agendamento *popAgendamento(Agendamento *l)
{
    if(l!=NULL)
    {
        Agendamento *aux = l->prox;
        free(l);
        return aux;
    }
}

/*****
Funções auxiliares
*****/

//Imprimir valores dos sinais vitais
void escreverInteiro(int num,int posx,int posy,int fnt)
{
    char aux[5]="";
    sprintf(aux,"%02d",num);
    EscreverEsquerda(aux,posx,posy,fnt);
}

void escreverSinaisVitais()
{
    char aux1[8],aux2[3];
    escreverInteiro(p.sinais[HR].valor,1225,691,fntHr);
    sprintf(aux1,"%02d/%02d",p.sinais[BP1].valor,p.sinais[BP2].valor);
    EscreverEsquerda(aux1,1225,581,fntBp);
    escreverInteiro(p.sinais[SPO2].valor,1225,409,fntSpo2);
    escreverInteiro(p.sinais[RESP].valor,1225,268,fntResp);
    sprintf(aux2,"%d",p.sinais[TEMP].valor);
    EscreverEsquerda(aux2,1225,132,fntTemp);
}

char* converterSegMin(int segundos)
{
    char *aux = (char*)malloc(sizeof(10));
    int minutos=0;
    minutos = segundos/60;
    segundos = segundos % 60;
    sprintf(aux,"%02d:%02d",minutos,segundos);
    return aux;
}

int clicado(int objeto)
{
    if(evento.tipoEvento==EVENTO_MOUSE && evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao==MOUSE_ESQUERDO)
    {
        int x_mouse,y_mouse,x_obj,y_obj,alt,larg;
        x_mouse = evento.mouse.posX;
        y_mouse = evento.mouse.posY;
        GetXYObjeto(objeto,&x_obj,&y_obj);
        GetDimensoesObjeto(objeto,&alt,&larg);
        if(x_mouse >= x_obj && x_mouse <= (x_obj+larg) && y_mouse >= y_obj && y_mouse<=(y_obj+alt))
        {
            return 1;
        }

    }
    return 0;
}

//Controlador de agendamentos
void controleAgd()
{
    if(filaAgd!=NULL){
        if(p.quadro==NORMAL && TempoDecorrido(timerIniSim) > filaAgd->ti)
        {
            p.quadro = filaAgd->qdr;
            ReiniciaTimer(timerDurSim);
            printf("Quadro alterado!\n");
        }

        if(p.quadro == filaAgd->qdr && TempoDecorrido(timerDurSim) > filaAgd->dur)
        {
            p.quadro = NORMAL;
            ReiniciaTimer(timerIniSim);
            filaAgd = popAgendamento(filaAgd);
            printf("Quadro normalizado!\n");
        }
    }
}

void desenhaBarra(int timerBarra)
{
    int x;
    float t;
    t = TempoDecorrido(timerBarra)/7.0;

    x = (0*(1 - t)) + (930*(t));

    DesenhaRetangulo(x,172,700,15,PRETO);

    if(t>=1)
    {
        ReiniciaTimer(timerBarra);
    }
}

/*****
Funções de manipulação dos gráficos
*****/

void moveGrafico(Grafico *g,int timer,float tmp,int desloc)
{
    if(TempoDecorrido(timer)>tmp)
    {
        Grafico *aux = g;
        while(aux!=NULL)
        {
            aux->x-=desloc;
            if(aux->x<0)
            {
                aux->x=900-desloc;
            }
            aux=aux->prox;
        }
        ReiniciaTimer(timer);
    }
}

void desenhaGrafico(Grafico *l,int posy,SDL_Color cor)
{
    if(l!=NULL && l->prox !=NULL)
    {
        if(abs(l->prox->x - l->x) <= 10)
        {
            DesenhaLinhaSimples(l->x,l->y+posy,l->prox->x,l->prox->y+posy,cor);
            DesenhaLinhaSimples(l->x,l->y+posy+1,l->prox->x,l->prox->y+posy+1,cor);
            //DesenhaLinhaSimples(l->x,l->y+posy+2,l->prox->x,l->prox->y+posy+2,cor);
        }
        desenhaGrafico(l->prox,posy,cor);
    }
}

Grafico *inicializaGrafico(char *nomearq)
{
    Grafico *g=NULL;
    char caminho[50];
    sprintf(caminho,"../db/%s.txt",nomearq);
    int x,y;
    FILE *arq= fopen(caminho,"rt");
    if(arq==NULL)
    {
        printf("Arquivo de grafico inexistente: %s\n",caminho);
        exit(0);
    }
    while(!feof(arq))
    {
        fscanf(arq,"%d %d\n",&x,&y);
        g=pushPonto(g,x,y);
    }
    return g;
}


/*****
Funções valores ECG
*****/

int gerarBatimentos()
{
    srand(time(NULL));
    int aleat = rand();
    switch(p.quadro)
    {
    case NORMAL:
        {
            p.sinais[HR].valor =(aleat % 40) + 60;
            break;
        }
    case QD2 :
        {
            p.sinais[HR].valor =(aleat % 10) + 60;
            break;
        }
    }
}

/*****
Funções de exibição de telas e janelas
*****/

void telaMonitor() {
    int janPainel = CriaJanela("Painel", ALT_TELA, LARG_TELA);
    GanhaFocoJanela(0);

    int timers[2];
    timers[0] = CriaTimer();
    timers[1] = CriaTimer();

    int fundoMonitor = CriaObjeto("..//imagens//fundos//fundoMonitor.png",0,255);
    int fundoPainel = CriaObjeto("..//imagens//fundos//fundoPainel.png",0,255,janPainel);
    int miniTela = 0;

    p.sinais[HR].g = inicializaGrafico("normal");
    p.quadro = NORMAL;
    p.sinais[HR].timerGrafico = CriaTimer();


    while (JogoRodando() && TELA == tMonitor) {

      evento = GetEvento();

      IniciaDesenho();

        //Tela Painel
        DesenhaObjeto(fundoPainel);
        // EscreverEsquerda(p.nome, 100, 714, fntForm, janPainel);
        if (miniTela != 0)
          DesenhaObjeto(miniTela);

        //Tela Monitor
        DesenhaObjeto(fundoMonitor);

        desenhaGrafico(p.sinais[HR].g, 765, AZUL_PISCINA);
        moveGrafico(p.sinais[HR].g, p.sinais[HR].timerGrafico, 0.2, 5);

        escreverSinaisVitais();

        controleAgd();

        gerarBatimentos();

        char fps[20];
        sprintf(fps,"%.0f",GetFPS());
        // EscreverEsquerda(fps,0,0,fntBp);

        if (TempoDecorrido(timers[1]) >= 1) {
          SalvaTela("telaAtual.bmp");
          DestroiObjeto(miniTela);
          miniTela = CriaObjeto(".//telaAtual.bmp",0,255,janPainel);
          SetDimensoesObjeto(miniTela,253,490);
          MoveObjeto(miniTela,62,217);
          ReiniciaTimer(timers[1]);
        }

        desenhaBarra(timers[0]);

      EncerraDesenho();

    }
}

void telaFormulario() {

  int fundoFormulario = CriaObjeto("../imagens//fundos//fundoFormulario.png",0);
  int btnIniciar = CriaObjeto("../imagens//btns//btnIniciar.png",0);
  MoveObjeto(btnIniciar, 1378, 48);
  int numCaixasdeTexto = 8, i;

  CaixadeTexto **inputs = (CaixadeTexto**)malloc(numCaixasdeTexto*sizeof(CaixadeTexto*));
  inputs[0] = criarCaixadeTexto(187,668,40,860,50);
  inputs[1] = criarCaixadeTexto(1174,668,40,65,3,1);
  inputs[2] = criarCaixadeTexto(1378,668,40,50,1);
  inputs[3] = criarCaixadeTexto(90,549,37,1349,50);
  inputs[4] = criarCaixadeTexto(90,431,37,1350,50);
  inputs[5] = criarCaixadeTexto(362,356,37,70,4,1);
  inputs[6] = criarCaixadeTexto(665,356,37,134,20);
  inputs[7] = criarCaixadeTexto(448,149,151,991,100);


  while (JogoRodando() && TELA == tFormulario) {

      evento = GetEvento();

      IniciaDesenho();
        DesenhaObjeto(fundoFormulario);
        DesenhaObjeto(btnIniciar);
          selecionarCaixadeTexto(inputs,numCaixasdeTexto);

          for (i=0; i< numCaixasdeTexto; i++) {
              desenhaCaixadeTexto(inputs[i]);
          }
          if (clicado(btnIniciar)) {
              sprintf(p.nome,"%s", inputs[0]->texto);
              p.idade = atoi(inputs[1]->texto);
              p.sexo = inputs[2]->texto[0];
              sprintf(p.diagMed,"%s", inputs[3]->texto);
              sprintf(p.diagEnf,"%s", inputs[4]->texto);
              p.diasInter = atoi(inputs[5]->texto);
              sprintf(p.quartoLeito,"%s", inputs[6]->texto);
              sprintf(p.evolucao,"%s", inputs[7]->texto);
              TELA = tMonitor;
          }

      EncerraDesenho();

    }
}

/*****
Fluxo principal
*****/

int main( int argc, char* args[] ) {

    CriaJogo("Monitor ECG",0,ALT_TELA,LARG_TELA);

    meuTeclado = GetTeclado();

    //Criação dos timers
    timerIniSim = CriaTimer();
    timerDurSim = CriaTimer();

    //Criação das fontes
    fntHr = CriaFonteNormal("..//fontes//Carlito.ttf", 120, AZUL_PISCINA, 0, AZUL_PISCINA, ESTILO_NORMAL);
    fntBp = CriaFonteNormal("..//fontes//Carlito.ttf", 80, VERMELHO_SANGUE, 0, VERMELHO_SANGUE, ESTILO_NORMAL);
    fntSpo2 = CriaFonteNormal("..//fontes//Carlito.ttf", 95, VERDE_ESCURO, 0, VERDE_ESCURO, ESTILO_NORMAL);
    fntResp = CriaFonteNormal("..//fontes//Carlito.ttf", 95, AZUL_CLARO, 0, AZUL_CLARO, ESTILO_NORMAL);
    fntTemp= CriaFonteNormal("..//fontes//Carlito.ttf", 95, LARANJA_FOSCO, 0, LARANJA_FOSCO, ESTILO_NORMAL);
    fntPainel = CriaFonteNormal("..//fontes//Carlito.ttf", 15, CINZA_ESCURO, 0, AZUL_PISCINA, ESTILO_NEGRITO);
    fntForm = CriaFonteNormal("..//fontes//Consolas.ttf", 20, CINZA_ESCURO, 0, PRETO, ESTILO_NORMAL);

    while(JogoRodando()) {
      switch (TELA) {
        case tFormulario: {
          telaFormulario();
          break;
        }
        case tMonitor: {
          telaMonitor();
          break;
        }
      }
    }

    return 0;
}
