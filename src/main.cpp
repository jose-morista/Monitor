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

/*****
Estruturas de dados
*****/

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
    char nome[50];
    int quadro;
    sinalVital sinais[6];
}Paciente;

/*****
Variáveis globais
*****/

PIG_Evento evento;
PIG_Teclado meuTeclado;

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

Grafico *pushPonto(Grafico *l,int x,int y)
{
    if(l==NULL)
    {
        Grafico *novo = (Grafico*)malloc(sizeof(Grafico));
        novo->x = x;
        novo->y = y;
        novo->prox=NULL;
        return novo;

    }else
    {
        //l->prox = add_ponto(l->prox,x,y);
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
    escreverInteiro(p.sinais[HR].valor,1075,591,fntHr);
    sprintf(aux1,"%02d/%02d",p.sinais[BP1].valor,p.sinais[BP2].valor);
    EscreverEsquerda(aux1,1075,481,fntBp);
    escreverInteiro(p.sinais[SPO2].valor,1075,309,fntSpo2);
    escreverInteiro(p.sinais[RESP].valor,1075,168,fntResp);
    sprintf(aux2,"%d",p.sinais[TEMP].valor);
    EscreverEsquerda(aux2,1075,32,fntTemp);
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

void Desenha_Barra(int timerBarra)
{
    int x;
    float t;
    t = TempoDecorrido(timerBarra)/7.0;

    x = (0*(1 - t)) + (930*(t));

    DesenhaRetangulo(x,172,548,15,PRETO);

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

void DesenhaGrafico(Grafico *l,int posy,SDL_Color cor)
{
    if(l!=NULL && l->prox !=NULL)
    {
        if(abs(l->prox->x - l->x) <= 10)
        {
            DesenhaLinhaSimples(l->x,l->y+posy,l->prox->x,l->prox->y+posy,cor);
            //DesenhaLinhaSimples(l->x,l->y+posy+1,l->prox->x,l->prox->y+posy+1,cor);
            //DesenhaLinhaSimples(l->x,l->y+posy+2,l->prox->x,l->prox->y+posy+2,cor);
        }
        DesenhaGrafico(l->prox,posy,cor);
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

int gerar_batimentos()
{
    srand(time(NULL));
    int aleat = rand(),aux;
    aux = aleat %2;
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

    int fundoMonitor = CriaObjeto("..//imagens//fundo.png",0,255);
    int fundoPainel = CriaObjeto("..//imagens//fundoPainel.png",0,255,janPainel);

    while (JogoRodando()) {

      evento = GetEvento();

      IniciaDesenho();
        DesenhaObjeto(fundoPainel);
        DesenhaObjeto(fundoMonitor);
      EncerraDesenho();

    }
}

void telaInicial() {
    telaMonitor();
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
    /* fntHr = CriaFonteNormal("..//fontes//Carlito.ttf", 120, AZUL_PISCINA, 0, AZUL_PISCINA, ESTILO_NORMAL);
    fntBp = CriaFonteNormal("..//fontes//Carlito.ttf", 80, VERMELHO_SANGUE, 0, VERMELHO_SANGUE, ESTILO_NORMAL);
    fntSpo2 = CriaFonteNormal("..//fontes//Carlito.ttf", 95, VERDE_ESCURO, 0, VERDE_ESCURO, ESTILO_NORMAL);
    fntResp = CriaFonteNormal("..//fontes//Carlito.ttf", 95, AZUL_CLARO, 0, AZUL_CLARO, ESTILO_NORMAL);
    fntTemp= CriaFonteNormal("..//fontes//Carlito.ttf", 95, LARANJA_FOSCO, 0, LARANJA_FOSCO, ESTILO_NORMAL);
    fntPainel = CriaFonteNormal("..//fontes//Carlito.ttf", 15, CINZA_ESCURO, 0, AZUL_PISCINA, ESTILO_NEGRITO);
    fntForm = CriaFonteNormal("..//fontes//Consolas.ttf", 16, PRETO, 0, PRETO, ESTILO_NORMAL); */

    telaInicial();

    return 0;
}
