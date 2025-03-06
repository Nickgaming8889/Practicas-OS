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

void Simulador::automatic() {
    vector<Proceso> procesos;
    int n;

    cout << "Ingrese el numero de procesos: ";
    cin >> n;

    for (int i = 1; i <= n; ++i) {
        int id_programa = i;
        int tiempo_max = rand() % 11 + 7;
        int opcion = rand() % 5 + 1;
        vector<int> data(2);
        char op;

        cout << "Ingrese los operandos: ";
        cin >> data[0] >> data[1];

        switch (opcion) {
            case 1:
                op = '+';
                break;
            case 2:
                op = '-';
                break;
            case 3:
                op = '*';
                break;
            case 4:
                op = '/';
                break;
            case 5:
                op = '%';
                break;
            default:
                op = '+';
                break;
        }

        procesos.emplace_back(id_programa, op, data, tiempo_max);
    }

    for (size_t i = 0; i < procesos.size(); i += 3) {
        Lote lote;
        for (size_t j = i; j < i + 3 && j < procesos.size(); ++j) {
            lote.procesos.push_back(procesos[j]);
        }        
        lotes.push(lote);
    }
}

void Simulador::ejecutar() {
    bool pause = false;
    while (!lotes.empty()) {
        int num_lote = 1;
        Lote lote_actual = lotes.front();
        lotes.pop();
        cout << "Lotes pendientes: " << lotes.size() << endl;

        auto it = lote_actual.procesos.begin();
        while (it != lote_actual.procesos.end()) {
            Proceso &proceso = *it;
            cout << "\nEjecutando proceso " << proceso.id_programa << ": " << proceso.operacion << endl;

            for (int t = 0; t < proceso.tiempo_max; ++t) {
                if (_kbhit()) {
                    char tecla = _getch();
                    switch (tecla) {
                        case 'I': case 'i':
                            cout << "\nProceso " << proceso.id_programa << " interrumpido y enviado de nuevo al lote actual." << endl;
                            lote_actual.procesos.push_back(proceso);
                            it = lote_actual.procesos.erase(it);
                            continue;
                        case 'E': case 'e':
                            cout << "\nProceso: " << proceso.id_programa << ". Termino con error.";
                            procesos_terminados.push_back({proceso.id_programa, -1});
                            it = lote_actual.procesos.erase(it);
                            break;
                        case 'P': case 'p':
                            cout << "Simulacion pausada. Presione 'C' para continuar..." << endl;
                            pause = true;
                            while (pause) {
                                if (_kbhit()) {
                                    char key = _getch();
                                    if (key == 'C' || key == 'c') {
                                        pause = false;
                                        cout << "Simulacion reanudada." << endl;
                                    }
                                }
                            }
                            break;
                    }
                }

                proceso.tiempo_transcurrido++;
                reloj_global++;
                cout << "\nLotes pendientes: " << lotes.size() <<
                        "\nLote en ejecucion: " << num_lote << endl;
                for (const auto& proceso : lote_actual.procesos) {
                    cout << "ID: " << proceso.id_programa << ". Tiempo Estimado: " << proceso.tiempo_max << endl;
                }
                cout << "\nProceso en ejecucion: " <<
                        "\nID Programa: " << proceso.id_programa <<
                        //"\nProgramador: " << proceso.programador <<
                        "\nOperacion: " << proceso.datos[0] << proceso.operacion << proceso.datos[1] <<
                        "\nTiempo transcurrido: " << proceso.tiempo_transcurrido <<
                        "\nTiempo restante: " << proceso.tiempo_max - proceso.tiempo_transcurrido <<
                        "\nTiempo Global: " << reloj_global << endl;
                this_thread::sleep_for(chrono::seconds(1));
                // Valida si el proceso en ejecucion, ya cumplio su tiempo, para agregarlo a procesos_terminados...
                if (proceso.tiempo_transcurrido >= proceso.tiempo_max) {
                    proceso.ejecutar();
                    procesos_terminados.push_back({proceso.id_programa, proceso.resultado});
                }
                // Muestra la tabla de procesos terminados...
                if (!procesos_terminados.empty()) {
                    cout << "\nProcesos Terminados." << endl;
                    for (const auto& p : procesos_terminados) {
                        if (p.second == -1) {
                            cout << "ID Programa: " << p.first << ", Resultado: ERROR" << endl;
                        } else {
                            cout << "ID Programa: " << p.first << ", Resultado: " << p.second << endl;
                        }
                    }
                }
            }
            //siguiente_proceso:;
            ++it;
        }
        //siguiente_lote:;
        num_lote++;
    }
    cout << "\nTiempo Total: " << reloj_global << endl;
    cout << "Simulación finalizada." << endl;
    this_thread::sleep_for(chrono::seconds(5));
}