#include "CParticula.h"

class CGeradorParticulas{

private:
Particula parts[MAX_PARTICULAS];
int inix,iniy,dx,dy;            //posicao e direcao atual das particulas
int pivoAbsX,pivoAbsY;          //pivo das particulas
float pivoRelX,pivoRelY;
bool modoPivoRelativo;
SDL_Renderer* renderer;
int hp;
float anguloRot;
float escalaInicial,escalaFinal;
int maxParticulas;
char nome[50];
int ativas;
int usaGerTimer;

public:

    CGeradorParticulas(int maximoParticulas,SDL_Renderer* renderizador, char *nomeArq, int usaGerenciadorTimer){
        maxParticulas = maximoParticulas;
        strcpy(nome,nomeArq);
        usaGerTimer = usaGerenciadorTimer;

        for (int i=0;i<maxParticulas;i++)
            parts[i] = NULL;

        renderer = renderizador;
	    dx = dy = 0;
		pivoAbsX = pivoAbsY = 0;
		pivoRelX = pivoRelY = 0.0f;
		hp = 1;
		anguloRot = 0.0f;
		escalaInicial = escalaFinal = 1.0f;
		ativas = 0;
		modoPivoRelativo = false;
    }

    ~CGeradorParticulas(){
        for (int i=0;i<maxParticulas;i++)
            if (parts[i]) delete parts[i];
    }

    void MoveParticulas(){
        for (int i=0;i<maxParticulas;i++)
            if (parts[i]) {
                parts[i]->Move();
                if (!parts[i]->viva){
                    delete parts[i];
                    parts[i] = NULL;
                    ativas--;
                }
            }
    }

    void Move(int nx,int ny){
        inix = nx;
        iniy = ny;
    }

    void MudaDirecao(int novaDx,int novaDy){
        dx = novaDx;
        dy = novaDy;
    }

    void MudaRotacao(float angPerSec){
        anguloRot = angPerSec;
    }

    void MudaEscala(float escalaIni,float escalaFim){
        escalaInicial = escalaIni;
        escalaFinal = escalaFim;
    }

    void MudaHP(int novoValor){
        hp = novoValor;
    }

    void Desenha(){
        for (int i=0;i<maxParticulas;i++)
            if ((parts[i])&&(parts[i]->viva)) {
                parts[i]->Desenha(NULL);
            }
    }

    void SetPivo(int pivoX,int pivoY){
        pivoAbsX = pivoX;
        pivoAbsY = pivoY;
        modoPivoRelativo = false;
    }

    void SetPivo(float pivoX,float pivoY){
        pivoRelX = pivoX;
        pivoRelY = pivoY;
        modoPivoRelativo = true;
    }

    int CriaParticula(int fadingOut=0,int minX=-50,int minY=-50,int maxX=LARG_TELA+50,int maxY=ALT_TELA+50,float maxTempo=9999999.9){
        int i=0;
        if (ativas>=maxParticulas) return -1;
        while (parts[i])
            i++;
        parts[i] = new CParticula(inix,iniy,dx,dy,hp,anguloRot,escalaInicial,escalaFinal,fadingOut,nome,usaGerTimer);
        if (modoPivoRelativo==false){
            parts[i]->SetPivo(pivoAbsX,pivoAbsY);
        }else{
            parts[i]->SetPivo(pivoRelX,pivoRelY);
        }
        parts[i]->DefineLimites(minX,maxX,minY,maxY,maxTempo);
        ativas++;
        return i;
    }

    int Colisao(Objeto outro){
        int resp = 0;
        int i=0,cont=0;
        while (!resp && i<maxParticulas && cont<ativas){
            if (parts[i]){
                cont++;
                resp = parts[i]->Colisao(outro);
            }
            i++;
        }
        return resp;
    }

    int Colisao(){
        int resp = 0;
        int i=0;
        while (!resp && i<maxParticulas-1){
            if (parts[i]){
                int j=i+1;
                while (!resp&&j<maxParticulas){
                    if (parts[j]){
                        resp = parts[i]->Colisao(parts[j]);
                    }
                    j++;
                }
            }
            i++;
        }
        return resp;
    }

    int GetQtdAtivas(){
        return ativas;
    }
};

typedef CGeradorParticulas* GeradorParticulas;
