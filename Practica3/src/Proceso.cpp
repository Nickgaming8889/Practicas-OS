#include "../headers/Proceso.h"

Proceso::Proceso(int id, char op, vector<int> dat, int tiempo, int reloj_global)
    : id_programa(id), operacion(op), datos(dat), tiempo_max(tiempo), 
      tiempo_llegada(reloj_global), resultado(0) {}

void Proceso::ejecutar() {
    switch (operacion) {
        case '+': resultado = datos[0] + datos[1]; break;
        case '-': resultado = datos[0] - datos[1]; break;
        case '*': resultado = datos[0] * datos[1]; break;
        case '/': resultado = (datos[1] != 0) ? (double)datos[0] / datos[1] : NAN; break;
        case '%': resultado = datos[0] % datos[1]; break;
        case '^': resultado = pow(datos[0], datos[1]); break;
    }
}

string Proceso::getEstadoString() const {
    switch(estado) {
        case NUEVO: return "Nuevo";
        case LISTO: return "Listo";
        case EJECUCION: return "Ejecucion";
        case BLOQUEADO: return "Bloqueado";
        case TERMINADO: return "Terminado";
        default: return "Desconocido";
    }
}

void Proceso::calcularTiempos(int reloj_global) {
    if (estado == TERMINADO) {
        tiempo_finalizacion = reloj_global;
    }
    // Actualizar otros tiempos según sea necesario
}