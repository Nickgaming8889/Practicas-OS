#ifndef PROCESO_H
#define PROCESO_H

#include <iostream>
#include <vector>
#include <cmath>
#include <string>

using namespace std;

enum Estado { NUEVO, LISTO, EJECUCION, BLOQUEADO, TERMINADO };

struct Proceso {
    int id_programa;
    char operacion;
    vector<int> datos;
    int tiempo_max;
    int tiempo_transcurrido = 0;
    int tiempo_bloqueado = 0;
    double resultado;
    Estado estado = NUEVO;
    
    // Tiempos para estadísticas
    int tiempo_llegada = 0;
    int tiempo_finalizacion = 0;
    int tiempo_respuesta = -1;
    int tiempo_espera = 0;
    int tiempo_servicio = 0;
    
    Proceso(int id, char op, vector<int> dat, int tiempo, int reloj_global);
    void ejecutar();
    string getEstadoString() const;
    void calcularTiempos(int reloj_global);
};

#endif