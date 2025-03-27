#include "../headers/Simulador.h"

int main() {
    Simulador simulador;
    
    cout << "Seleccione modo:\n";
    cout << "1. Automatico\n";
    cout << "2. Manual\n";
    cout << "Opcion: ";
    
    int opcion;
    cin >> opcion;
    
    if (opcion == 1) {
        simulador.automatic();
    } else {
        simulador.manual();
    }
    
    simulador.ejecutar();
    return 0;
}