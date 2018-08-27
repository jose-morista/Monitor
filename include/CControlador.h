class CControlador{

private:

int id;
SDL_GameController *ctrl;
SDL_Joystick *joy;
int qtdEixos,qtdBotoes;
char nome[50];

public:


CControlador(int idControle){
    id = idControle;
    ctrl = SDL_GameControllerOpen(id);
    joy = SDL_GameControllerGetJoystick(ctrl);
    qtdEixos = SDL_JoystickNumAxes(joy);
    qtdBotoes = SDL_JoystickNumButtons(joy);
    strcpy(nome,SDL_JoystickName(joy));
}

~CControlador(){
    SDL_GameControllerClose(ctrl);
}

int BotaoPressionado(int botao){
    return SDL_GameControllerGetButton(ctrl,(SDL_GameControllerButton)botao);
}

int EixoAcionado(int eixo){
    return SDL_GameControllerGetAxis(ctrl,(SDL_GameControllerAxis)eixo);
}

int GetQtdEixos(){
    return qtdEixos;
}

int GetQtdBotoes(){
    return qtdBotoes;
}

void GetNome(char *nomeControle){
    strcpy(nomeControle,nome);
}

};

typedef CControlador *Controlador;
