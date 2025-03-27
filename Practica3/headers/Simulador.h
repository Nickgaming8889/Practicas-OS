#ifndef SIMULADOR_H
#define SIMULADOR_H

#include "Proceso.h"
#include <queue>
#include <vector>
#include <thread>
#include <list>

#ifdef _WIN32
    #include <conio.h>
    #include <chrono>
#elif __linux__
    #include <termios.h> 
    #include <unistd.h>  
    #include <fcntl.h>
#endif

class Simulador {
private:
    queue<Proceso> nuevos;
    list<Proceso> listos;
    list<Proceso> bloqueados;
    vector<Proceso> terminados;
    Proceso* ejecutando = nullptr;
    int reloj_global = 0;
    int id_counter = 1;
    const int MAX_PROCESOS_EN_MEMORIA = 3;

    void mostrarTablaProcesos();
    void actualizarEstados();
    void agregarProcesoAutomatico();
    void agregarProcesoManual();
    void mostrarInterfaz();
    void mostrarBCP(const Proceso& p);
    void procesarTeclas(char tecla);

public:
    void automatic();
    void manual();
    void ejecutar();
};

#endif