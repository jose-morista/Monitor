#include "PIG.h"

/*****
Defini��es e enumera��es
*****/

//Defini��es de cores adicionais
#define AZUL_PISCINA ((PIG_Cor){108,255,255,255})
#define VERMELHO_SANGUE ((PIG_Cor){178,0,0,255})
#define VERDE_ESCURO ((PIG_Cor){35,140,0,255})
#define AZUL_CLARO ((PIG_Cor){0,178,178,255})
#define LARANJA_FOSCO ((PIG_Cor){140,70,0,255})
#define CINZA_ESCURO ((PIG_Cor){49,49,49,255})
#define CINZA_CLARO ((PIG_Cor) {240,240,240,255})

//Defini��es dos poss�veis quadros
#define NORMAL 0
#define BRAD 1
#define TAQU 2
#define ASSI 3

//enumera��o dos sinais vitais
enum sinaisVitais{HR, BP1, BP2, SPO2, RESP, TEMP};

enum telas{tFormulario, tMonitor, tEntrada};

/*****
Estruturas de dados
*****/

typedef struct CaixadeTexto {
  int x, y, alt, larg, sel, tamMax, numerico;
  char *texto;
}CaixadeTexto;

typedef struct agendamento {
  int qdr, ti, dur;
  char nomeQdr[20];
  struct agendamento *prox;
}Agendamento;

typedef struct grafico {
  int x, y;
  struct grafico *prox;
} Grafico;

typedef struct sinalVital {
  int valor, timerGrafico, timerValor;
  Grafico *g;
}sinalVital;

typedef struct Paciente {
  char nome[50], diagMed[50], diagEnf[50], quartoLeito[20], sexo;
  int idade, diasInter;
  int quadro;
  sinalVital sinais[6];
}Paciente;

/*****
Vari�veis globais
*****/

PIG_Evento evento;
PIG_Teclado meuTeclado;

int TELA= tEntrada;
int telaCheia = 1;

//Fila de agendamento
Agendamento *filaAgd=NULL;
Agendamento *historico=NULL;


//Paciente
Paciente p;

//Timers
int timerIniSim, timerDurSim;

//Fontes
int fntHr, fntBp, fntSpo2, fntResp, fntTemp, fntForm, fntPainel;

//Janelas
int janPainel;

//Audios
int sons[5];

//Vari�veis de controle
int popupOpen = 0;

/*****
Fun��es de manipul��o das estruturas de dados
*****/


CaixadeTexto criarCaixadeTexto(int x,int y,int alt,int larg,int tamMax, int numerico = 0) {
  CaixadeTexto novo;// = (CaixadeTexto*)malloc(sizeof(CaixadeTexto));
  novo.x=x;
  novo.y=y;
  novo.alt=alt;
  novo.larg=larg;
  novo.sel=0;
  novo.numerico=numerico;
  novo.tamMax=tamMax;
  novo.texto= (char*)malloc(tamMax+1); // Aloca a string e sempre a usa diretamente
  novo.texto[0] = '\0';
  return novo;
}

int CaixadeTextoClicada(int x,int y,int alt,int larg) {
  int x_mouse=evento.mouse.posX;
  int y_mouse=evento.mouse.posY;
  if (x_mouse >= x && x_mouse <= (x+larg) && y_mouse >= y && y_mouse<=(y+alt)) {
    return 1;
  }
  return 0;
}

void selecionarCaixadeTexto(CaixadeTexto inputs[], int numInputs) {
  if (evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao==MOUSE_ESQUERDO) {
    int i;
    for (i=0;i<numInputs;i++) {
      inputs[i].sel = CaixadeTextoClicada(inputs[i].x,inputs[i].y,inputs[i].alt,inputs[i].larg);
    }
  }
}

char *getinput(CaixadeTexto input) {
  char* texto = input.texto;
  // EDIT: desnecessario ficar alocando mais memoria, basta usar direto a string da caixa de texto
  //char *texto = (char*)malloc(sizeof(char)*input->tamMax);
  //sprintf(texto,"");
  //if (input->texto!=NULL) {
  //  sprintf(texto,"%s",input->texto);
  //  free(input->texto);
  //}
  if (evento.tipoEvento==EVENTO_TECLADO & evento.teclado.acao==TECLA_PRESSIONADA) {
    int i = evento.teclado.tecla;
    if (strlen(texto)<input.tamMax) {
      if (i>=TECLA_a && i<=TECLA_z && !input.numerico) {
        i =i + 'a' - TECLA_a;
        sprintf(texto,"%s%c",texto,i);
      } else if (i==TECLA_BARRAESPACO && !input.numerico) {
        sprintf(texto,"%s ",texto);
      } else if (i>=TECLA_1 && i<= TECLA_0) {
        if (i!= TECLA_0)
          i =i - TECLA_1 + '0' +1;
        else
          i= 48;
        sprintf(texto,"%s%c",texto,i);
      } else if (i>=TECLA_KP_1 && i<= TECLA_KP_0) {
          if(i!= TECLA_KP_0)
            i =i - TECLA_KP_1 + '0' +1;
          else
            i= 48;
          sprintf(texto,"%s%c",texto,i);
      }
    }
    if (i==TECLA_BACKSPACE && strlen(texto)>0) {
      //char *aux = (char*)malloc(sizeof(char)*input->tamMax);
      //strncpy(aux,texto,(strlen(texto)-1));
      texto[strlen(texto)-1]='\0';
      //strcpy(texto,aux);
      //free(aux);
    }
  }
  return texto;
}

void desenhaCaixadeTexto(CaixadeTexto input) {
  DesenhaRetanguloVazado(input.x,input.y,input.alt,input.larg,CINZA);
  if (input.sel) {
    DesenhaRetanguloVazado(input.x,input.y,input.alt,input.larg,AZUL);
    input.texto = getinput(input);
  }
  if (input.texto!=NULL)
    EscreverEsquerda(input.texto,input.x+3,input.y+input.alt-30,fntForm);
}

Grafico *pushPonto(Grafico *l,int x,int y) {
  if (l==NULL) {
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

Agendamento *pushAgendamento(Agendamento *l,int quadro,int tempo_ini,int duracao, const char *nomeQdr) {
  if (l==NULL) {
    Agendamento *novo = (Agendamento*)malloc(sizeof(Agendamento));
    novo->dur=duracao;
    novo->ti=tempo_ini;
    novo->qdr = quadro;
    sprintf(novo->nomeQdr, "%s", nomeQdr);
    novo->prox = NULL;
    return novo;
  } else {
    l->prox = pushAgendamento(l->prox,quadro,tempo_ini,duracao, nomeQdr);
    return l;
  }
}

Agendamento *popAgendamento(Agendamento *l) {
  if (l!=NULL) {
    Agendamento *aux = l->prox;
    free(l);
    return aux;
  }
}

/*****
Fun��es de manipula��o dos gr�ficos
*****/

void moveGrafico(Grafico *g,int timer,float tmp,int desloc) {
  if (TempoDecorrido(timer)>tmp) {
    Grafico *aux = g;
    while (aux!=NULL) {
      aux->x-=desloc;
      if (aux->x<0) {
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

Grafico *inicializaGrafico(const char *nomearq)
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
Fun��es auxiliares
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

char* converterSegMin(int segundos, char* aux)
{
    // EDIT: usar string pre-alocada
    //char *aux = (char*)malloc(sizeof(10));
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
  switch (p.quadro) {
    case NORMAL: {
      p.sinais[HR].g = inicializaGrafico("normal");
      break;
    }
    case BRAD: {
      p.sinais[HR].g = inicializaGrafico("bradicardia");
      break;
    }
    case TAQU: {
      p.sinais[HR].g = inicializaGrafico("taquicardia");
      break;
    }
    case ASSI: {
      p.sinais[HR].g = inicializaGrafico("assistolia");
      break;
    }
  }
}

void controleSons() {
      StopTudoAudio();
      PlayAudio(sons[p.quadro]);
/* EDIT: Aqui foi possivel simplificar bastante
  switch (p.quadro) {
    case NORMAL: {
      StopTudoAudio();
      PlayAudio(sons[NORMAL]);
      break;
    }
    case BRAD: {
      StopTudoAudio();
      PlayAudio(sons[BRAD]);
      break;
    }
    case TAQU: {
      StopTudoAudio();
      PlayAudio(sons[TAQU]);
      break;
    }
    case ASSI: {
      StopTudoAudio();
      PlayAudio(sons[ASSI]);
      break;
    }
  }*/
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
            controleSons();
        }

        if(p.quadro == filaAgd->qdr && TempoDecorrido(timerDurSim) > filaAgd->dur)
        {
            p.quadro = NORMAL;
            ReiniciaTimer(timerIniSim);
            filaAgd = popAgendamento(filaAgd);
            controleGraficos();
            controleSons();
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

  if(p.quadro != NORMAL) {
    DesenhaRetangulo(1054,721,43,410,CINZA_CLARO,janPainel);
  }

  while (aux!=NULL) {
    char agd[50];
    char ti[10], dur[10]; // EDIT: strings para usar na funcao convertesSegMin
    sprintf(agd, "%s TI: %s DUR: %s", aux->nomeQdr, converterSegMin(aux->ti, ti), converterSegMin(aux->dur, dur));
    EscreverEsquerda(agd, 1063, (730-(numAgd * 35)), fntPainel);
    DesenhaLinhaSimples(1053,(730 -(numAgd * 35) - 8), 1465,(730-(numAgd * 35) - 8), PRETO, janPainel);
    numAgd++;
    aux = aux->prox;
  }

};

/*****
Fun��es valores ECG
*****/

int gerarBatimentos()
{
  if(TempoDecorrido(p.sinais[HR].timerValor) > 0.2) {
    srand(time(NULL));
    int aleat = rand();
    switch(p.quadro)
    {
      case NORMAL:
        {
          p.sinais[HR].valor = (aleat % 40) + 60;
          break;
        }
      case BRAD:
        {
          p.sinais[HR].valor = (aleat % 10) + 60;
          break;
        }
      case TAQU:
        {
          p.sinais[HR].valor = (aleat % 40) + 120;
          break;
        }
      case ASSI:
        {
          p.sinais[HR].valor = 0;
          break;
        }
    }
    ReiniciaTimer(p.sinais[HR].timerValor);
  }
}

void controlePainelAgendamento( int *dur, int *ini, int *btns, int *quadro, char *nomeQuadro) {
  int tDuracao, tInicio;
  if (clicado(btns[0])) {
    tDuracao = *dur;
    tInicio = *ini;
    if ( filaAgd==NULL ) {
      ReiniciaTimer(timerIniSim);
    }
    filaAgd = pushAgendamento(filaAgd, *quadro, tInicio, tDuracao, nomeQuadro);
    historico = pushAgendamento(historico, *quadro, tInicio, tDuracao, nomeQuadro);
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
  } else if (clicado(btns[5])) {
    popupOpen = !popupOpen;
  }
  if (popupOpen) {
    if (clicado(btns[6])) {
        *quadro = BRAD;
        strcpy(nomeQuadro, "Bradicardia");
        popupOpen = !popupOpen;
    } else if (clicado(btns[7])) {
      *quadro = TAQU;
      strcpy(nomeQuadro, "Taquicardia");
      popupOpen = !popupOpen;
    } else if (clicado(btns[8])) {
      *quadro = ASSI;
      strcpy(nomeQuadro, "Assistolia");
      popupOpen = !popupOpen;
    }
  }
}

/*****
Fun��es de exibi��o de telas e janelas
*****/

void telaMonitor() {
  janPainel = CriaJanela("Painel", ALT_TELA, LARG_TELA);
  SDL_RenderSetLogicalSize(CGerenciadorJanelas::GetJanela(janPainel)->GetRenderer(), LARG_TELA, ALT_TELA);
  SetModoJanela(JANELA_TELACHEIA_DISPLAY,janPainel);

  int btnFechar = CriaObjeto("..//imagens//btns/btnFechar.png", 0 ,255, janPainel);
  MoveObjeto(btnFechar, 1570, 870);

  int timers[2], i;
  timers[0] = CriaTimer();
  timers[1] = CriaTimer();

  int posPainelx = 0, posPainely = 0;
  int fundoMonitor = CriaObjeto("..//imagens//fundos//fundoMonitor.png",0,255);
  int fundoPainel = CriaObjeto("..//imagens//fundos//fundoPainel.png",0,255,janPainel);

  p.sinais[HR].g = inicializaGrafico("normal");
  p.sinais[BP1].valor = 120;
  p.sinais[BP2].valor = 80;
  p.sinais[SPO2].valor = 95;
  p.sinais[RESP].valor= 20;
  p.sinais[TEMP].valor= 37;
  p.quadro = NORMAL;

  p.sinais[HR].timerGrafico = CriaTimer();
  p.sinais[HR].timerValor = CriaTimer();

  fntPainel = CriaFonteNormal("..//fontes//Carlito.ttf", 18, VERMELHO, 0, AZUL_PISCINA, ESTILO_NEGRITO,janPainel);
  int fntPainelG = CriaFonteNormal("..//fontes//Carlito.ttf", 25, VERMELHO, 0, AZUL_PISCINA, ESTILO_NEGRITO,janPainel);
  int fntPainelS = CriaFonteNormal("..//fontes//Carlito.ttf", 14, VERMELHO, 0, AZUL_PISCINA, ESTILO_NEGRITO,janPainel);

  int btns[9], tInicio = 0, tDuracao = 0, quadro = BRAD;
  char nomeQuadro[20] = "Bradicardia";

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
  btns[5] = CriaObjeto("..//imagens//btns//btnPopup.png",0,255,janPainel);
  MoveObjeto(btns[5], 524, 131);
  btns[6] = CriaObjeto("..//imagens//btns//btnBrad.png", 0 ,255, janPainel);
  MoveObjeto(btns[6], 170, 162);
  btns[7] = CriaObjeto("..//imagens//btns//btnTaqu.png", 0 ,255, janPainel);
  MoveObjeto(btns[7], 170, 192);
  btns[8] = CriaObjeto("..//imagens//btns//btnInfa.png", 0 ,255, janPainel);
  MoveObjeto(btns[8], 170, 222);

  char idade[4], diasInter[10], sexo[2];
  sprintf(idade, "%d", p.idade);
  sprintf(diasInter, "%d", p.diasInter);
  sprintf(sexo, "%c", p.sexo);

  //Cria��o dos timers
  timerIniSim = CriaTimer();
  timerDurSim = CriaTimer();

  PlayAudio(sons[NORMAL]);
  int numAgd = 0;
  Agendamento *aux = historico;

  while (JogoRodando() && TELA == tMonitor) {

    evento = GetEvento();


    if (evento.tipoEvento==EVENTO_TECLADO&&evento.teclado.acao==TECLA_PRESSIONADA){
        if (evento.teclado.tecla == TECLA_F7) {
          telaCheia = !telaCheia;
          if (telaCheia==1){
            SetModoJanela(JANELA_TELACHEIA_DISPLAY,0);
            SetModoJanela(JANELA_TELACHEIA_DISPLAY, janPainel);
          }else{
            SetModoJanela(JANELA_NORMAL,0);
            SetModoJanela(JANELA_NORMAL, janPainel);
            SetPosicaoJanela(50, 50, janPainel);
            SetPosicaoJanela(50, 50);
          }
        }
      }
    IniciaDesenho();

      //Tela Painel
      DesenhaObjeto(fundoPainel);
      DesenhaObjeto(btnFechar);
      if (clicado(btnFechar)) {
        FinalizaJogo();
      }

      numAgd = 0;
      aux = historico;
      while (aux!=NULL) {
        char agd[50];
	char ti[10], dur[10];
        sprintf(agd, "%d:%s-%s-%s", (numAgd + 1), aux->nomeQdr, converterSegMin(aux->ti, ti), converterSegMin(aux->dur, dur));
        if (numAgd >= 12) {
          EscreverEsquerda(agd, 310, (443-((numAgd - 12) * 20)), fntPainelS);
        } else {
          EscreverEsquerda(agd, 80, (443-(numAgd * 20)), fntPainelS);
        }
        numAgd++;
        aux = aux->prox;
      }

      //Escrevendo os dados do paciente
      EscreverEsquerda(p.nome, 160, 717, fntPainel);
      EscreverEsquerda(idade, 160, 681, fntPainel);
      EscreverEsquerda(diasInter, 420, 681, fntPainel);
      EscreverEsquerda(sexo, 540, 681, fntPainel);
      EscreverEsquerda(p.quartoLeito, 740, 681, fntPainel);
      EscreverEsquerda(p.diagMed, 278, 642, fntPainel);
      EscreverEsquerda(p.diagEnf, 324, 604, fntPainel);

      //Painel Fila de agendamento
      imprimirFilaAgendamento();

      // Painel de agendamento
      for (i=0; i < 6; i++) {
        DesenhaObjeto(btns[i]);
      }
      if (popupOpen) {
        for (i=6; i < 9; i++) {
          DesenhaObjeto(btns[i]);
        }
      }

      controlePainelAgendamento(&tDuracao, &tInicio, btns, &quadro, nomeQuadro);
      char dur[10], ini[10];
      converterSegMin(tDuracao, dur);
      converterSegMin(tInicio, ini);
      EscreverEsquerda(dur,528,85,fntPainel);
      EscreverEsquerda(ini,243,85,fntPainel);
      EscreverEsquerda(nomeQuadro,187,135,fntPainel);

      //Painel quadro atual
      if (p.quadro == NORMAL) {
        EscreverEsquerda("Normal", 710, 375, fntPainelG);
        if (filaAgd != NULL) {
          char aux[10];
          converterSegMin(filaAgd->ti - TempoDecorrido(timerIniSim), aux);
          EscreverEsquerda(aux,745,260,fntPainelG);
        }
      } else {
        char aux[10];
        EscreverEsquerda(filaAgd->nomeQdr, 696, 375, fntPainelG);
        converterSegMin(filaAgd->dur - TempoDecorrido(timerDurSim), aux);
        EscreverEsquerda(aux,745,260,fntPainelG);
      }

      //Tela Monitor
      DesenhaObjeto(fundoMonitor);

      //Desenho dos gr�ficos
      desenhaGrafico(p.sinais[HR].g, 765, AZUL_PISCINA);
      moveGrafico(p.sinais[HR].g, p.sinais[HR].timerGrafico, 0.3, 5);

      escreverSinaisVitais();

      controleAgd();

      gerarBatimentos();

      /*char fps[20];
      sprintf(fps,"%.0f",GetFPS());
      EscreverEsquerda(fps,0,0,fntBp);*/

      desenhaBarra(timers[0]);

    EncerraDesenho();

  }
}

void telaEntrada() {
  int fundoEntrada = CriaObjeto("../imagens//fundos//fundoEntrada.png",0);
  int btnFechar = CriaObjeto("..//imagens//btns/btnFechar.png", 0 ,255);
  MoveObjeto(btnFechar, 1570, 870);
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
      DesenhaObjeto(btnFechar);
      if (clicado(btnFechar)) {
        FinalizaJogo();
      }
    EncerraDesenho();
  }

}

void telaFormulario() {

  int fundoFormulario = CriaObjeto("../imagens//fundos//fundoFormulario.png",0);
  int btnIniciar = CriaObjeto("../imagens//btns//btnIniciar.png",0);
  int btnFechar = CriaObjeto("..//imagens//btns/btnFechar.png", 0 ,255);
  MoveObjeto(btnFechar, 1570, 870);
  MoveObjeto(btnIniciar, 1378, 48);
  int numCaixasdeTexto = 7, i;

  //CaixadeTexto **inputs = (CaixadeTexto**)malloc(numCaixasdeTexto*sizeof(CaixadeTexto*));
  CaixadeTexto inputs[numCaixasdeTexto];
  inputs[0] = criarCaixadeTexto(187,668,40,860,50);
  inputs[1] = criarCaixadeTexto(1174,668,40,65,3,1);
  inputs[2] = criarCaixadeTexto(1378,668,40,50,1);
  inputs[3] = criarCaixadeTexto(90,549,37,957,50);
  inputs[4] = criarCaixadeTexto(90,431,37,957,50);
  inputs[5] = criarCaixadeTexto(362,356,37,70,4,1);
  inputs[6] = criarCaixadeTexto(665,356,37,134,20);


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
              sprintf(p.nome,"%s", inputs[0].texto);
              p.idade = atoi(inputs[1].texto);
              p.sexo = inputs[2].texto[0];
              sprintf(p.diagMed,"%s", inputs[3].texto);
              sprintf(p.diagEnf,"%s", inputs[4].texto);
              p.diasInter = atoi(inputs[5].texto);
              sprintf(p.quartoLeito,"%s", inputs[6].texto);
              TELA = tMonitor;
          }
          DesenhaObjeto(btnFechar);
          if (clicado(btnFechar)) {
            FinalizaJogo();
          }
      EncerraDesenho();

    }
}

/*****
Fluxo principal
*****/

int main( int argc, char* args[] ) {

    CriaJogo("Monitor ECG",0,ALT_TELA,LARG_TELA);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");//usar "best" ou "linear"
    //cria um sistema de coordenadas lógicas, usando a altura e largura definidas para a janela
    SDL_RenderSetLogicalSize(CGerenciadorJanelas::GetJanela(0)->GetRenderer(), LARG_TELA, ALT_TELA);
    SetModoJanela(JANELA_TELACHEIA_DISPLAY,0);
    meuTeclado = GetTeclado();

    //Cria��o dos audios
    sons[NORMAL] = CriaAudio("..//audios//normal.mp3", -1, 0);
    sons[BRAD] = CriaAudio("..//audios//brad.mp3", -1, 0);
    sons[TAQU] = CriaAudio("..//audios//taqu.mp3", -1, 0);
    sons[ASSI] = CriaAudio("..//audios//assis.mp3", -1, 0);

    //Cria��o das fontes
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
