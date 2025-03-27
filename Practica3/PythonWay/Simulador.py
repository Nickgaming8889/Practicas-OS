import os
import sys
import time
import random
import math
from enum import Enum
from collections import deque
import platform
import msvcrt  # Para Windows
try:
    import tty
    import termios
except ImportError:
    pass  # Para sistemas que no son Unix

class Estado(Enum):
    NUEVO = 1
    LISTO = 2
    EJECUCION = 3
    BLOQUEADO = 4
    TERMINADO = 5

class Proceso:
    def __init__(self, id_programa, operacion, datos, tiempo_max, tiempo_llegada):
        self.id_programa = id_programa
        self.operacion = operacion
        self.datos = datos
        self.tiempo_max = tiempo_max
        self.tiempo_transcurrido = 0
        self.tiempo_bloqueado = 0
        self.resultado = float('nan')
        self.estado = Estado.NUEVO
        self.tiempo_llegada = tiempo_llegada
        self.tiempo_finalizacion = 0
        self.tiempo_respuesta = -1
        self.tiempo_espera = 0
        self.tiempo_servicio = 0
    
    def ejecutar(self):
        try:
            if self.operacion == '+':
                self.resultado = self.datos[0] + self.datos[1]
            elif self.operacion == '-':
                self.resultado = self.datos[0] - self.datos[1]
            elif self.operacion == '*':
                self.resultado = self.datos[0] * self.datos[1]
            elif self.operacion == '/':
                self.resultado = self.datos[0] / self.datos[1]
            elif self.operacion == '%':
                self.resultado = self.datos[0] % self.datos[1]
            elif self.operacion == '^':
                self.resultado = self.datos[0] ** self.datos[1]
        except:
            self.resultado = float('nan')
    
    def get_estado_string(self):
        return self.estado.name

def clear_screen():
    os.system('cls' if platform.system() == 'Windows' else 'clear')

def get_key():
    if platform.system() == 'Windows':
        if msvcrt.kbhit():
            return msvcrt.getch().decode()
        return ''
    else:
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch

class Simulador:
    def __init__(self):
        self.nuevos = deque()
        self.listos = deque()
        self.bloqueados = deque()
        self.terminados = []
        self.ejecutando = None
        self.reloj_global = 0
        self.id_counter = 1
        self.MAX_PROCESOS_EN_MEMORIA = 3
    
    def agregar_proceso_automatico(self):
        tiempo_max = random.randint(7, 18)
        opcion = random.randint(0, 5)
        data = [random.randint(0, 20), random.randint(4, 20)]
        op = ['+', '-', '*', '/', '%', '^'][opcion]
        self.nuevos.append(Proceso(self.id_counter, op, data, tiempo_max, self.reloj_global))
        self.id_counter += 1
    
    def agregar_proceso_manual(self):
        print("\nAgregar nuevo proceso:")
        operacion = input("Operación (+, -, *, /, %, ^): ")
        dato1 = int(input("Primer operando: "))
        dato2 = int(input("Segundo operando: "))
        tiempo_max = int(input("Tiempo máximo estimado (7-18): "))
        self.nuevos.append(Proceso(self.id_counter, operacion, [dato1, dato2], tiempo_max, self.reloj_global))
        self.id_counter += 1
    
    def automatic(self):
        n = int(input("Ingrese el número de procesos: "))
        for _ in range(n):
            self.agregar_proceso_automatico()
    
    def manual(self):
        n = int(input("Ingrese el número de procesos: "))
        for _ in range(n):
            self.agregar_proceso_manual()
    
    def actualizar_estados(self):
        # Mover procesos de nuevos a listos si hay espacio
        while self.nuevos and len(self.listos) + (1 if self.ejecutando else 0) + len(self.bloqueados) < self.MAX_PROCESOS_EN_MEMORIA:
            p = self.nuevos.popleft()
            p.estado = Estado.LISTO
            self.listos.append(p)
        
        # Actualizar procesos bloqueados
        for p in list(self.bloqueados):
            p.tiempo_bloqueado += 1
            if p.tiempo_bloqueado >= 8:
                p.estado = Estado.LISTO
                self.listos.append(p)
                self.bloqueados.remove(p)
        
        # Asignar nuevo proceso a ejecución si hay listos y no hay proceso ejecutando
        if not self.ejecutando and self.listos:
            self.ejecutando = self.listos.popleft()
            self.ejecutando.estado = Estado.EJECUCION
            if self.ejecutando.tiempo_respuesta == -1:
                self.ejecutando.tiempo_respuesta = self.reloj_global - self.ejecutando.tiempo_llegada
    
    def procesar_teclas(self, tecla):
        tecla = tecla.upper()
        if tecla == 'I' and self.ejecutando:  # Interrupción E/S
            self.ejecutando.estado = Estado.BLOQUEADO
            self.ejecutando.tiempo_bloqueado = 0
            self.bloqueados.append(self.ejecutando)
            self.ejecutando = None
        elif tecla == 'E' and self.ejecutando:  # Error
            self.ejecutando.estado = Estado.TERMINADO
            self.ejecutando.resultado = float('nan')
            self.ejecutando.tiempo_finalizacion = self.reloj_global
            self.terminados.append(self.ejecutando)
            self.ejecutando = None
        elif tecla == 'N':  # Nuevo proceso
            self.agregar_proceso_automatico()
        elif tecla == 'P':  # Pausa
            print("\nSimulación pausada. Presione 'C' para continuar...")
            while True:
                key = get_key()
                if key.upper() == 'C':
                    break
                time.sleep(0.1)
        elif tecla == 'B':  # Mostrar tabla de procesos
            self.mostrar_tabla_procesos()
    
    def mostrar_tabla_procesos(self):
        clear_screen()
        print("\n=== TABLA DE PROCESOS (BCP) ===")
        print("-" * 80)
        
        # Procesos nuevos
        for p in list(self.nuevos):
            self.mostrar_bcp(p)
        
        # Procesos listos
        for p in self.listos:
            self.mostrar_bcp(p)
        
        # Proceso en ejecución
        if self.ejecutando:
            self.mostrar_bcp(self.ejecutando)
        
        # Procesos bloqueados
        for p in self.bloqueados:
            self.mostrar_bcp(p)
        
        # Procesos terminados
        for p in self.terminados:
            self.mostrar_bcp(p)
        
        print("-" * 80)
        print("Presione 'C' para continuar...")
        while get_key().upper() != 'C':
            time.sleep(0.1)
    
    def mostrar_bcp(self, p):
        print(f"ID: {p.id_programa} | Estado: {p.get_estado_string()}")
        print(f"Operación: {p.datos[0]} {p.operacion} {p.datos[1]}", end="")
        
        if p.estado == Estado.TERMINADO:
            print(" | Resultado: ", end="")
            if math.isnan(p.resultado):
                print("ERROR", end="")
            else:
                print(p.resultado, end="")
        print()
        
        print(f"T. Llegada: {p.tiempo_llegada}", end="")
        if p.estado == Estado.TERMINADO:
            print(f" | T. Finalización: {p.tiempo_finalizacion} | T. Retorno: {p.tiempo_finalizacion - p.tiempo_llegada}", end="")
        print()
        
        print(f"T. Espera: {p.tiempo_espera} | T. Servicio: {p.tiempo_servicio}", end="")
        
        if p.estado != Estado.TERMINADO:
            print(f" | T. Restante: {p.tiempo_max - p.tiempo_transcurrido}", end="")
        
        if p.tiempo_respuesta != -1:
            print(f" | T. Respuesta: {p.tiempo_respuesta}", end="")
        
        if p.estado == Estado.BLOQUEADO:
            print(f" | T. Bloqueado restante: {8 - p.tiempo_bloqueado}", end="")
        
        print("\n" + "-" * 80)
    
    def mostrar_interfaz(self):
        clear_screen()
        print("=== SIMULADOR DE PROCESOS ===")
        print(f"Reloj Global: {self.reloj_global} u.t.")
        print("Teclas: I (Interrupción), E (Error), P (Pausa), C (Continuar), N (Nuevo), B (Tabla)\n")
        
        # Procesos nuevos
        print(f"Procesos nuevos pendientes: {len(self.nuevos)}")
        
        # Cola de listos
        print("\n--- Cola de Listos ---")
        for p in self.listos:
            print(f"ID: {p.id_programa} | TME: {p.tiempo_max} | TR: {p.tiempo_max - p.tiempo_transcurrido}")
        
        # Proceso en ejecución
        print("\n--- Proceso en Ejecución ---")
        if self.ejecutando:
            print(f"ID: {self.ejecutando.id_programa}")
            print(f"Operación: {self.ejecutando.datos[0]} {self.ejecutando.operacion} {self.ejecutando.datos[1]}")
            print(f"T. Ejecutado: {self.ejecutando.tiempo_transcurrido} | T. Restante: {self.ejecutando.tiempo_max - self.ejecutando.tiempo_transcurrido}")
            
            # Barra de progreso
            progress = (self.ejecutando.tiempo_transcurrido * 20) // self.ejecutando.tiempo_max
            print("Progreso: [" + "=" * progress + " " * (20 - progress) + f"] {progress * 5}%")
        else:
            print("Ningún proceso en ejecución")
        
        # Cola de bloqueados
        print("\n--- Cola de Bloqueados ---")
        for p in self.bloqueados:
            print(f"ID: {p.id_programa} | T. Bloqueado: {p.tiempo_bloqueado}/8")
        
        # Procesos terminados
        print("\n--- Procesos Terminados ---")
        for p in self.terminados:
            print(f"ID: {p.id_programa} | Operación: {p.datos[0]} {p.operacion} {p.datos[1]} | Resultado: ", end="")
            if math.isnan(p.resultado):
                print("ERROR")
            else:
                print(p.resultado)
    
    def ejecutar(self):
        try:
            while self.nuevos or self.listos or self.ejecutando or self.bloqueados:
                self.actualizar_estados()
                self.mostrar_interfaz()
                
                # Procesar teclas
                key = get_key()
                if key:
                    self.procesar_teclas(key)
                    continue
                
                # Avanzar tiempo de ejecución
                if self.ejecutando:
                    time.sleep(1)  # 1 segundo real por unidad de tiempo
                    
                    self.ejecutando.tiempo_transcurrido += 1
                    self.ejecutando.tiempo_servicio += 1
                    self.reloj_global += 1
                    
                    # Verificar si el proceso ha terminado
                    if self.ejecutando.tiempo_transcurrido >= self.ejecutando.tiempo_max:
                        self.ejecutando.ejecutar()
                        
                        # Mostrar resultado
                        clear_screen()
                        print(f"\nProceso {self.ejecutando.id_programa} terminado. Resultado: ", end="")
                        if math.isnan(self.ejecutando.resultado):
                            print("ERROR")
                        else:
                            print(self.ejecutando.resultado)
                        time.sleep(2)
                        
                        # Mover a terminados
                        self.ejecutando.estado = Estado.TERMINADO
                        self.ejecutando.tiempo_finalizacion = self.reloj_global
                        self.terminados.append(self.ejecutando)
                        self.ejecutando = None
                else:
                    time.sleep(1)
                    self.reloj_global += 1
                
                # Actualizar tiempos de espera
                for p in self.listos:
                    p.tiempo_espera += 1
            
            # Mostrar resultados finales
            clear_screen()
            print("\n=== SIMULACIÓN TERMINADA ===")
            print(f"Tiempo total: {self.reloj_global} unidades de tiempo\n")
            
            # Mostrar tabla completa de procesos
            print("=== RESUMEN FINAL DE PROCESOS ===")
            print("-" * 80)
            print("| ID | Operación   | Estado     | T.Llegada | T.Finaliz | T.Retorno | T.Espera | T.Servicio | Resultado |")
            print("-" * 80)
            
            for p in sorted(self.terminados, key=lambda x: x.id_programa):
                operacion = f"{p.datos[0]} {p.operacion} {p.datos[1]}"
                resultado = "ERROR" if math.isnan(p.resultado) else f"{p.resultado:.2f}"
                
                print(f"| {p.id_programa:2} | {operacion:10} | {p.get_estado_string():10} | "
                      f"{p.tiempo_llegada:9} | {p.tiempo_finalizacion:9} | "
                      f"{p.tiempo_finalizacion - p.tiempo_llegada:9} | "
                      f"{p.tiempo_espera:8} | {p.tiempo_servicio:9} | "
                      f"{resultado:9} |")
            
            print("-" * 80)
            
            # Calcular estadísticas
            if self.terminados:
                avg_retorno = sum(p.tiempo_finalizacion - p.tiempo_llegada for p in self.terminados) / len(self.terminados)
                avg_espera = sum(p.tiempo_espera for p in self.terminados) / len(self.terminados)
                avg_servicio = sum(p.tiempo_servicio for p in self.terminados) / len(self.terminados)
                
                print("\n=== ESTADÍSTICAS ===")
                print(f"Tiempo promedio de retorno: {avg_retorno:.2f}")
                print(f"Tiempo promedio de espera: {avg_espera:.2f}")
                print(f"Tiempo promedio de servicio: {avg_servicio:.2f}")
            
            print("\nPresione cualquier tecla para salir...")
            get_key()
        
        except KeyboardInterrupt:
            print("\nSimulación interrumpida por el usuario")

def main():
    sim = Simulador()
    
    print("=== SIMULADOR DE PROCESOS ===")
    print("1. Modo automático")
    print("2. Modo manual")
    opcion = input("Seleccione el modo (1/2): ")
    
    if opcion == '1':
        sim.automatic()
    else:
        sim.manual()
    
    sim.ejecutar()

if __name__ == "__main__":
    main()