#include "../headers/Proceso.h"

Proceso::Proceso(int id, string prog, char op, vector<int> dat, int tiempo)
    : id_programa(id), programador(prog), operacion(op), datos(dat), tiempo_max(tiempo), resultado(0) {}

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