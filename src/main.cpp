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

enum telas{tFormulario, tMonitor, tEntrada};

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

int TELA= tEntrada;

//Fila de agendamento
Agendamento *filaAgd=NULL;

//Paciente
Paciente p;

//Timers
int timerIniSim, timerDurSim;

//Fontes
int fntHr, fntBp, fntSpo2, fntResp, fntTemp, fntForm, fntPainel;

//Janelas
int janPainel;

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

void controleGraficos() {
  switch(p.quadro) {
    case NORMAL: {
      p.sinais[HR].g = inicializaGrafico("normal");
      break;
    }
    case QD2: {
      p.sinais[HR].g = inicializaGrafico("bradicardia");
      break;
    }
  }
}

//Controlador de agendamentos
void controleAgd()
{
    if(filaAgd!=NULL){
        if(p.quadro==NORMAL && TempoDecorrido(timerIniSim) > filaAgd->ti)
        {
            p.quadro = filaAgd->qdr;
            ReiniciaTimer(timerDurSim);
            controleGraficos();
        }

        if(p.quadro == filaAgd->qdr && TempoDecorrido(timerDurSim) > filaAgd->dur)
        {
            p.quadro = NORMAL;
            ReiniciaTimer(timerIniSim);
            filaAgd = popAgendamento(filaAgd);
            controleGraficos();
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

void imprimirFilaAgendamento() {
  Agendamento * aux = filaAgd;

  int numAgd = 0;

  while (aux!=NULL) {
    char agd[20];
    sprintf(agd, "%s %s %s", "Bradicardia", converterSegMin(aux->ti), converterSegMin(aux->dur));
    EscreverEsquerda(agd, 1063, (730-(numAgd * 35)), fntPainel);
    DesenhaLinhaSimples(1053,(730 -(numAgd * 35) - 8), 1465,(730-(numAgd * 35) - 8), PRETO, janPainel);
    numAgd++;
    aux = aux->prox;
  }

};

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

void controlePainelAgendamento( int *dur, int *ini, int *btns) {
  int tDuracao, tInicio;
  if (clicado(btns[0])) {
    tDuracao = *dur;
    tInicio = *ini;
    filaAgd = pushAgendamento(filaAgd, QD2, tInicio, tDuracao);
    printf("Quadro agendado!\n");
    *dur = 0;
    *ini = 0;
  } else if (clicado(btns[1])) {
    *ini = (*ini) + 30;
  } else if (clicado(btns[2])) {
    *ini = (*ini) - 30;
    tInicio = *ini;
    if( tInicio <= 0) {
      *ini = 0;
    }
  } else if (clicado(btns[3])) {
    *dur = (*dur) + 30;
  } else if (clicado(btns[4])) {
    *dur = (*dur) - 30;
    tDuracao = *dur;
    if( tDuracao <= 0) {
      *dur = 0;
    }
  }
}

/*****
Funções de exibição de telas e janelas
*****/

void telaMonitor() {
    janPainel = CriaJanela("Painel", ALT_TELA, LARG_TELA);
    GanhaFocoJanela(0);

    int timers[2], i;
    timers[0] = CriaTimer();
    timers[1] = CriaTimer();

    int fundoMonitor = CriaObjeto("..//imagens//fundos//fundoMonitor.png",0,255);
    int fundoPainel = CriaObjeto("..//imagens//fundos//fundoPainel.png",0,255,janPainel);
    int miniTela = 0;

    p.sinais[HR].g = inicializaGrafico("normal");
    p.quadro = NORMAL;
    p.sinais[HR].timerGrafico = CriaTimer();

    fntPainel = CriaFonteNormal("..//fontes//Carlito.ttf", 18, VERMELHO, 0, AZUL_PISCINA, ESTILO_NEGRITO,janPainel);
    int fntPainelG = CriaFonteNormal("..//fontes//Carlito.ttf", 25, VERMELHO, 0, AZUL_PISCINA, ESTILO_NEGRITO,janPainel);

    int btns[5], tInicio = 0, tDuracao = 0;

    btns[0] = CriaObjeto("..//imagens//btns//btnAgendar.png",0,255,janPainel);
    MoveObjeto(btns[0], 824, 56);
    btns[1] = CriaObjeto("..//imagens//btns//btnMais.png",0,255,janPainel);
    MoveObjeto(btns[1], 305, 80);
    btns[2] = CriaObjeto("..//imagens//btns//btnMenos.png",0,255,janPainel);
    MoveObjeto(btns[2], 337, 80);
    btns[3] = CriaObjeto("..//imagens//btns//btnMais.png",0,255,janPainel);
    MoveObjeto(btns[3], 585, 80);
    btns[4] = CriaObjeto("..//imagens//btns//btnMenos.png",0,255,janPainel);
    MoveObjeto(btns[4], 617, 80);

    char idade[4], diasInter[10], sexo[2];
    sprintf(idade, "%d", p.idade);
    sprintf(diasInter, "%d", p.diasInter);
    sprintf(sexo, "%c", p.sexo);

    //Criação dos timers
    timerIniSim = CriaTimer();
    timerDurSim = CriaTimer();

    while (JogoRodando() && TELA == tMonitor) {

      evento = GetEvento();

      IniciaDesenho();

        //Tela Painel

        DesenhaObjeto(fundoPainel);

        //Escrevendo os dados do paciente
        EscreverEsquerda(p.nome, 160, 717, fntPainel);
        EscreverEsquerda(idade, 160, 681, fntPainel);
        EscreverEsquerda(diasInter, 420, 681, fntPainel);
        EscreverEsquerda(sexo, 540, 681, fntPainel);
        EscreverEsquerda(p.quartoLeito, 740, 681, fntPainel);
        EscreverEsquerda(p.diagMed, 278, 642, fntPainel);
        EscreverEsquerda(p.diagEnf, 324, 604, fntPainel);
        EscreverEsquerda(p.evolucao, 304, 568, fntPainel);

        //Painel Fila de agendamento
        imprimirFilaAgendamento();

        // Painel de agendamento
        for (i=0; i < 5; i++) {
          DesenhaObjeto(btns[i]);
        }
        controlePainelAgendamento(&tDuracao, &tInicio, btns);
        char *dur, *ini;
        dur = converterSegMin(tDuracao);
        ini = converterSegMin(tInicio);
        EscreverEsquerda(dur,528,85,fntPainel);
        EscreverEsquerda(ini,243,85,fntPainel);

        //Painel quadro atual
        switch (p.quadro) {
          case NORMAL: {
            EscreverEsquerda("Normal", 710, 375, fntPainelG);
            break;
          }
          case QD2: {
            EscreverEsquerda("Bradicardia", 696, 375, fntPainelG);
            char *aux;
            aux = converterSegMin(filaAgd->dur - TempoDecorrido(timerDurSim));
            EscreverEsquerda(aux,745,260,fntPainelG);
            break;
          }
        }

        //Painel monitoramento
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
        EscreverEsquerda(fps,0,0,fntBp);

        if (TempoDecorrido(timers[1]) >= 0.5) {
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

void telaEntrada() {
  int fundoEntrada = CriaObjeto("../imagens//fundos//fundoEntrada.png",0);
  int timerEntrada = CriaTimer();
  float t;

  while(TELA == tEntrada && JogoRodando()) {
    evento = GetEvento();
    t = TempoDecorrido(timerEntrada);
    if( TempoDecorrido(timerEntrada) >= 1)
      SetOpacidadeObjeto(fundoEntrada,255 * (1 - (t-1)/2.5));
    if (TempoDecorrido(timerEntrada) > 3.5) {
      DestroiTimer(timerEntrada);
      TELA = tFormulario;
    }
    IniciaDesenho();
      DesenhaObjeto(fundoEntrada);
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

    //Criação das fontes
    fntHr = CriaFonteNormal("..//fontes//Carlito.ttf", 120, AZUL_PISCINA, 0, AZUL_PISCINA, ESTILO_NORMAL);
    fntBp = CriaFonteNormal("..//fontes//Carlito.ttf", 80, VERMELHO_SANGUE, 0, VERMELHO_SANGUE, ESTILO_NORMAL);
    fntSpo2 = CriaFonteNormal("..//fontes//Carlito.ttf", 95, VERDE_ESCURO, 0, VERDE_ESCURO, ESTILO_NORMAL);
    fntResp = CriaFonteNormal("..//fontes//Carlito.ttf", 95, AZUL_CLARO, 0, AZUL_CLARO, ESTILO_NORMAL);
    fntTemp= CriaFonteNormal("..//fontes//Carlito.ttf", 95, LARANJA_FOSCO, 0, LARANJA_FOSCO, ESTILO_NORMAL);
    fntForm = CriaFonteNormal("..//fontes//Consolas.ttf", 20, VERMELHO, 0, PRETO, ESTILO_NORMAL);

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
        case tEntrada: {
          telaEntrada();
          break;
        }
      }
    }

    return 0;
}
