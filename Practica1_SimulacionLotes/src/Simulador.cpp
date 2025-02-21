#include "../headers/Simulador.h"
#include <iostream>

void Simulador::agregar_proceso_manual() {
    vector<Proceso> procesos;
    int n;
    cout << "Ingrese el número de procesos: ";
    cin >> n;
    
    for (int i = 0; i < n; ++i) {
        int id, tiempo_max;
        string programador;
        char operacion;
        vector<int> datos(2);
        
        cout << "Favor de ingresar los datos del proceso No. " << i + 1 << endl;
        id = i;
        cout << "Ingrese nombre del programador: ";
        cin >> programador;
        cout << "Ingrese operación (+, -, *, /, %, ^): ";
        cin >> operacion;
        cout << "Ingrese dos operandos: ";
        cin >> datos[0] >> datos[1];
        cout << "Ingrese tiempo máximo estimado: ";
        cin >> tiempo_max;
        
        procesos.emplace_back(id, programador, operacion, datos, tiempo_max);
    }
    
    for (size_t i = 0; i < procesos.size(); i += 4) {
        Lote lote;
        for (size_t j = i; j < i + 4 && j < procesos.size(); ++j) {
            lote.procesos.push_back(procesos[j]);
        }
        lotes.push(lote);
    }
}

void Simulador::ejecutar() {
    while (!lotes.empty()) {
        int num_lote = 1;
        Lote lote_actual = lotes.front();
        lotes.pop();
        cout << "Lotes pendientes: " << lotes.size() << endl;

        for (auto& proceso : lote_actual.procesos) {
            cout << "\nEjecutando proceso " << proceso.id_programa + 1 << ": " << proceso.programador << ", " << proceso.operacion << " " << proceso.datos[0] << " " << proceso.datos[1] << endl;
            
            for (int t = 0; t < proceso.tiempo_max; ++t) {
                proceso.tiempo_transcurrido++;
                reloj_global++;
                cout << "Lotes pendientes: " << lotes.size() << 
                        "\nLote en ejecucion: " << num_lote << 
                        "\nProceso en ejecucion: " << proceso.id_programa + 1 << 
                        "\nProgramador: " << proceso.programador << " Operacion: " << proceso.operacion << 
                        "\nTiempo transcurrido: " << proceso.tiempo_transcurrido << 
                        "\nTiempo restante: " << proceso.tiempo_max - proceso.tiempo_transcurrido << 
                        "\nTiempo Global: " << reloj_global << endl;
                this_thread::sleep_for(chrono::seconds(1));
            }
            
            proceso.ejecutar();
            procesos_terminados.push_back({proceso.id_programa, proceso.resultado});
            cout << "Proceso terminado: " << proceso.id_programa + 1 << ", Resultado: " << proceso.resultado << endl;
        }
        num_lote++;
    }
    cout << "\nTiempo Total: " << reloj_global << endl;
    cout << "Simulación finalizada." << endl;
    this_thread::sleep_for(chrono::seconds(5));
}