#include "../headers/Simulador.h"

int main() {
    Simulador simulador;
    //simulador.agregar_proceso_manual();
    simulador.automatic();
    simulador.ejecutar();
    return 0;
}
