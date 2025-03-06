#ifndef PROCESO_H
#define PROCESO_H

#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

struct Proceso {
    int id_programa;
    string programador;
    char operacion;
    vector<int> datos;
    int tiempo_max;
    int tiempo_transcurrido = 0;
    double resultado;

    Proceso(int id, string prog, char op, vector<int> dat, int tiempo);
    Proceso(int id, char op, int tiempo);
    Proceso(int id, char op, vector<int> dat, int tiempo);
    void ejecutar();
};

#endif