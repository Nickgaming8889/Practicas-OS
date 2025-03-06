#include "../headers/Simulador.h"
#include <iostream>

void Simulador::agregar_proceso_manual() {
    vector<Proceso> procesos;
    int n;
    cout << "Ingrese el número de procesos: ";
    cin >> n;
    
    for (int i = 1; i <= n; ++i) {
        int id, tiempo_max;
        string programador;
        char operacion;
        vector<int> datos(2);
        
        cout << "Favor de ingresar los datos del proceso No. " << i << endl;
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
            cout << "\nEjecutando proceso " << proceso.id_programa << ": " << proceso.programador << ", " << proceso.operacion << " " << proceso.datos[0] << " " << proceso.datos[1] << endl;
            
            for (int t = 0; t < proceso.tiempo_max; ++t) {
                proceso.tiempo_transcurrido++;
                reloj_global++;
                cout << "\nLotes pendientes: " << lotes.size() <<
                        "\nLote en ejecucion: " << num_lote << endl;
                        for (const auto& proceso : lote_actual.procesos) {
                            cout << "ID: " << proceso.id_programa << ". Tiempo Estimado: " << proceso.tiempo_max << endl;
                        }
                cout << "\nProceso en ejecucion: " <<
                        "\nID Programa: " << proceso.id_programa << 
                        "\nProgramador: " << proceso.programador << 
                        "\nOperacion: " << proceso.datos[0] << proceso.operacion << proceso.datos[1] <<
                        "\nTiempo transcurrido: " << proceso.tiempo_transcurrido << 
                        "\nTiempo restante: " << proceso.tiempo_max - proceso.tiempo_transcurrido << 
                        "\nTiempo Global: " << reloj_global << endl;
                this_thread::sleep_for(chrono::seconds(1));
                //Valida si el proceso en ejecucion, ya cumplio su tiempo, para agregarlo a procesos_terminados...
                if (proceso.tiempo_transcurrido >= proceso.tiempo_max) {
                    proceso.ejecutar();
                    procesos_terminados.push_back({proceso.id_programa, proceso.resultado});
                }
                //Muestra la tabla de procesos terminados...
                if (!procesos_terminados.empty()) {
                    cout << "\nProcesos Terminados." << endl;
                    for (const auto& p : procesos_terminados) {
                        cout << "ID Programa: " << p.first << ", Resultado: " << p.second << endl;
                    }
                }
            }
            //proceso.ejecutar();
            //procesos_terminados.push_back({proceso.id_programa, proceso.resultado});
            /*cout << "\nProceso terminado: " << 
                    "\nID Programa: " << proceso.id_programa + 1 <<

                    "\nResultado: " << proceso.resultado << endl;*/
        }
        num_lote++;
    }
    cout << "\nTiempo Total: " << reloj_global << endl;
    cout << "Simulación finalizada." << endl;
    this_thread::sleep_for(chrono::seconds(5));
}