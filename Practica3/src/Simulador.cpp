#include "../headers/Simulador.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

// Implementaciones de getKey() y mySleep() igual que antes...
char getKey() {
    #ifdef _WIN32
        return _getch();
    #elif __linux__
        struct termios oldt, newt;
        char ch;
        int oldf;
    
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    
        ch = getchar();
    
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
    
        return ch;
    #endif
}

void mySleep(int seconds) {
    #ifdef _WIN32
        this_thread::sleep_for(chrono::seconds(1));
    #elif __linux__
        sleep(seconds);
    #endif
}

void Simulador::mostrarTablaProcesos() {
    cout << "\n\n--- TABLA DE PROCESOS (BCP) ---\n";
    cout << "--------------------------------------------\n";
    
    // Procesos nuevos
    queue<Proceso> temp_nuevos = nuevos;
    while (!temp_nuevos.empty()) {
        mostrarBCP(temp_nuevos.front());
        temp_nuevos.pop();
    }
    
    // Procesos listos
    for (const auto& p : listos) {
        mostrarBCP(p);
    }
    
    // Proceso en ejecución
    if (ejecutando) {
        mostrarBCP(*ejecutando);
    }
    
    // Procesos bloqueados
    for (const auto& p : bloqueados) {
        mostrarBCP(p);
    }
    
    // Procesos terminados
    for (const auto& p : terminados) {
        mostrarBCP(p);
    }
    
    cout << "--------------------------------------------\n";
    cout << "Presione 'C' para continuar...\n";
    
    char key;
    do {
        key = getKey();
    } while (key != 'C' && key != 'c');
}

void Simulador::mostrarBCP(const Proceso& p) {
    cout << "ID: " << p.id_programa << " | Estado: " << p.getEstadoString() << "\n";
    cout << "Operacion: " << p.datos[0] << p.operacion << p.datos[1];
    
    if (p.estado == TERMINADO) {
        cout << " | Resultado: ";
        if (isnan(p.resultado)) {
            cout << "ERROR";
        } else {
            cout << p.resultado;
        }
    }
    cout << "\n";
    
    cout << "T. Llegada: " << p.tiempo_llegada;
    if (p.estado == TERMINADO) {
        cout << " | T. Finalizacion: " << p.tiempo_finalizacion
             << " | T. Retorno: " << (p.tiempo_finalizacion - p.tiempo_llegada);
    }
    cout << "\n";
    
    cout << "T. Espera: " << p.tiempo_espera 
         << " | T. Servicio: " << p.tiempo_servicio;
    
    if (p.estado != TERMINADO) {
        cout << " | T. Restante: " << (p.tiempo_max - p.tiempo_transcurrido);
    }
    
    if (p.tiempo_respuesta != -1) {
        cout << " | T. Respuesta: " << p.tiempo_respuesta;
    }
    
    if (p.estado == BLOQUEADO) {
        cout << " | T. Bloqueado restante: " << (8 - p.tiempo_bloqueado);
    }
    
    cout << "\n--------------------------------------------\n";
}


void Simulador::actualizarEstados() {
    // Mover procesos de nuevos a listos si hay espacio
    while (!nuevos.empty() && listos.size() + (ejecutando ? 1 : 0) + bloqueados.size() < MAX_PROCESOS_EN_MEMORIA) {
        Proceso p = nuevos.front();
        nuevos.pop();
        p.estado = LISTO;
        listos.push_back(p);
    }

    // Actualizar procesos bloqueados
    for (auto it = bloqueados.begin(); it != bloqueados.end(); ) {
        it->tiempo_bloqueado++;
        if (it->tiempo_bloqueado >= 10) { // Ahora son 10 unidades de tiempo bloqueado
            it->estado = LISTO;
            listos.push_back(*it);
            it = bloqueados.erase(it);
        } else {
            ++it;
        }
    }

    // Si no hay proceso en ejecución y hay listos, asignar uno
    if (!ejecutando && !listos.empty()) {
        ejecutando = &listos.front();
        ejecutando->estado = EJECUCION;
        if (ejecutando->tiempo_respuesta == -1) {
            ejecutando->tiempo_respuesta = reloj_global - ejecutando->tiempo_llegada;
        }
        listos.pop_front();
    }
}

void Simulador::agregarProcesoAutomatico() {
    int tiempo_max = rand() % 12 + 7; // Entre 7 y 18
    int opcion = rand() % 6;
    vector<int> data(2);
    data[0] = rand() % 20;
    data[1] = rand() % 20 + 4;
    char op;
    
    switch (opcion) {
        case 0: op = '+'; break;
        case 1: op = '-'; break;
        case 2: op = '*'; break;
        case 3: op = '/'; break;
        case 4: op = '%'; break;
        case 5: op = '^'; break;
    }
    
    nuevos.push(Proceso(id_counter++, op, data, tiempo_max, reloj_global));
}

void Simulador::agregarProcesoManual() {
    int tiempo_max;
    char operacion;
    vector<int> datos(2);
    
    cout << "Ingrese operacion (+, -, *, /, %, ^): ";
    cin >> operacion;
    cout << "Ingrese dos operandos: ";
    cin >> datos[0] >> datos[1];
    cout << "Ingrese tiempo maximo estimado (7-18): ";
    cin >> tiempo_max;
    
    nuevos.push(Proceso(id_counter++, operacion, datos, tiempo_max, reloj_global));
}

void Simulador::automatic() {
    srand(time(nullptr));
    int n;
    cout << "Ingrese el numero de procesos: ";
    cin >> n;
    
    for (int i = 0; i < n; ++i) {
        agregarProcesoAutomatico();
    }
}

void Simulador::manual() {
    int n;
    cout << "Ingrese el numero de procesos: ";
    cin >> n;
    
    for (int i = 0; i < n; ++i) {
        agregarProcesoManual();
    }
}

void Simulador::procesarTeclas(char tecla) {
    switch (toupper(tecla)) {
        case 'I': // Interrupción E/S
            if (ejecutando) {
                ejecutando->estado = BLOQUEADO;
                ejecutando->tiempo_bloqueado = 0;
                bloqueados.push_back(*ejecutando);
                ejecutando = nullptr;  // Evita accesos indebidos
            }
            break;
            
        case 'E': // Error
            if (ejecutando) {
                ejecutando->estado = TERMINADO;
                ejecutando->resultado = NAN;
                ejecutando->tiempo_finalizacion = reloj_global;
                terminados.push_back(*ejecutando);
                ejecutando = nullptr;
            }
            break;
            
        case 'N': // Nuevo proceso
            agregarProcesoAutomatico();
            break;
            
        case 'B': // Mostrar tabla de procesos
            mostrarTablaProcesos();
            break;
            
        case 'P': // Pausa
            cout << "\nSimulacion pausada. Presione 'C' para continuar...\n";
            while (true) {
                char key = getKey();
                if (toupper(key) == 'C') break;
                mySleep(1);
            }
            break;
    }
}

void Simulador::mostrarInterfaz() {
    //system("clear || cls");

    cout << "=== SIMULADOR DE PROCESOS ===" << endl;
    cout << "Reloj Global: " << reloj_global << " u.t." << endl;
    cout << "Teclas: I (Interrupcion), E (Error), P (Pausa), C (Continuar), N (Nuevo), B (Tabla)\n\n";

    // Procesos nuevos
    cout << "Procesos nuevos pendientes: " << nuevos.size() << endl;

    // Cola de listos
    cout << "\n--- Cola de Listos ---\n";
    for (const auto& p : listos) {
        cout << "ID: " << p.id_programa
             << " | TME: " << p.tiempo_max
             << " | TR: " << (p.tiempo_max - p.tiempo_transcurrido) << endl;
    }

    // Proceso en ejecución
    cout << "\n--- Proceso en Ejecucion ---\n";
    if (ejecutando) {
        ejecutando->tiempo_transcurrido++;  // 🔹 Aumenta el tiempo ejecutado en cada actualización de interfaz

        cout << "ID: " << ejecutando->id_programa << endl;
        cout << "Valor de datos[0]: " << ejecutando->datos[0] << endl;
        cout << "Valor de operacion: " << ejecutando->operacion << endl;
        cout << "Valor de datos[1]: " << ejecutando->datos[1] << endl; 
        cout << "T. Ejecutado: " << ejecutando->tiempo_transcurrido
             << " | T. Restante: " << (ejecutando->tiempo_max - ejecutando->tiempo_transcurrido) << endl;

        // Barra de progreso
        cout << "Progreso: [";
        int progress = (ejecutando->tiempo_transcurrido * 20) / ejecutando->tiempo_max;
        for (int i = 0; i < 20; ++i) {
            cout << (i < progress ? "=" : " ");
        }
        cout << "] " << (progress * 5) << "%\n";

        // Si el proceso terminó, se mueve a terminados
        if (ejecutando->tiempo_transcurrido >= ejecutando->tiempo_max) {
            ejecutando->ejecutar();  // 🔹 Calcula el resultado

            cout << "\nProceso " << ejecutando->id_programa << " terminado. Resultado: ";
            cout << (isnan(ejecutando->resultado) ? "ERROR" : to_string(ejecutando->resultado)) << endl;
            mySleep(2);  // 🔹 Pausa para mostrar el resultado

            ejecutando->estado = TERMINADO;
            ejecutando->tiempo_finalizacion = reloj_global;
            terminados.push_back(*ejecutando);

            ejecutando = nullptr;  // 🔹 Evita que se use memoria no válida

            // Si hay procesos en la cola de listos, ejecutar el siguiente
            if (!listos.empty()) {
                ejecutando = &listos.front();  // 🔹 Se apunta al primer proceso de la cola
                listos.pop_front();
            }
        }
    } else {
        cout << "Ningun proceso en ejecucion\n";
    }

    // Cola de bloqueados
    cout << "\n--- Cola de Bloqueados ---\n";
    for (const auto& p : bloqueados) {
        cout << "ID: " << p.id_programa
             << " | T. Bloqueado: " << p.tiempo_bloqueado << "/8" << endl;
    }

    // Procesos terminados
    cout << "\n--- Procesos Terminados ---\n";
    for (const auto& p : terminados) {
        cout << "ID: " << p.id_programa
             << " | Operacion: " << p.datos[0] << p.operacion << p.datos[1]
             << " | Resultado: ";
        cout << (isnan(p.resultado) ? "ERROR" : to_string(p.resultado)) << endl;
    }

    mySleep(1);  // 🔹 Pausa después de mostrar la interfaz
}


void Simulador::ejecutar() {
    while (!nuevos.empty() || !listos.empty() || ejecutando || !bloqueados.empty()) {
        actualizarEstados();
        mostrarInterfaz();

        //mySleep(1); // Mostrar la interfaz pausadamente cada segundo

        char tecla = getKey();
        if (tecla != 0) {
            if ((tecla == 'E' || tecla == 'I') && ejecutando) { 
                procesarTeclas(tecla);
            } else if (tecla != 'E' && tecla != 'I') {  
                procesarTeclas(tecla);
            }            
            continue;
        }

        if (ejecutando) {
            ejecutando->tiempo_transcurrido++;
            ejecutando->tiempo_servicio++;
            reloj_global++;

            if (ejecutando->tiempo_transcurrido >= ejecutando->tiempo_max) {
                ejecutando->ejecutar();

                cout << "\nProceso " << ejecutando->id_programa << " terminado. Resultado: "
                     << (isnan(ejecutando->resultado) ? "ERROR" : to_string(ejecutando->resultado))
                     << endl;
                mySleep(2);

                ejecutando->estado = TERMINADO;
                ejecutando->tiempo_finalizacion = reloj_global;
                terminados.push_back(*ejecutando);
                ejecutando = nullptr;
            }
        } else {
            mySleep(1);
            reloj_global++;
        }

        for (auto& p : listos) {
            p.tiempo_espera++;
        }

        for (auto it = bloqueados.begin(); it != bloqueados.end();) {
            it->tiempo_bloqueado++;
            if (it->tiempo_bloqueado >= 8) {
                it->estado = LISTO;
                listos.push_back(*it);
                it = bloqueados.erase(it);
            } else {
                ++it;
            }
        }
    }

    mostrarInterfaz();
    cout << "\n=== SIMULACION TERMINADA ===\n";
    cout << "Tiempo total: " << reloj_global << " unidades de tiempo\n";

    cout << "\n--- Procesos Terminados ---\n";
    for (const auto& p : terminados) {
        cout << "ID: " << p.id_programa 
             << " | Operacion: " << p.datos[0] << p.operacion << p.datos[1]
             << " | T. Ejecucion: " << p.tiempo_servicio
             << " | Resultado: " << (isnan(p.resultado) ? "ERROR" : to_string(p.resultado))
             << endl;
    }

    cout << "\nPresione cualquier tecla para salir...";
    getKey();
}
