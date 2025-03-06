#ifndef SIMULADOR_H
#define SIMULADOR_H

#include "Proceso.h"
#include <queue>
#include <vector>
#include <thread>

#ifdef _WIN32
    #include <conio.h>
    #include <chrono>
#elif __linux__
    #include <termios.h> 
    #include <unistd.h>  
    #include <fcntl.h>
#endif


struct Lote {
    vector<Proceso> procesos;
};

class Simulador {
private:
    queue<Lote> lotes;
    vector<pair<int, double>> procesos_terminados;
    int reloj_global = 0;

public:
    void agregar_proceso_manual();
    void automatic();
    void ejecutar();
};

#endif