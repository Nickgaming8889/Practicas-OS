import os
import sys
import time
import random
import math
from enum import Enum
from collections import deque
import tty
import termios
import fcntl

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
        self.terminal_configurado = False
    
    def configurar_terminal(self):
        """Configura el terminal para lectura de teclas sin bloqueo"""
        if not self.terminal_configurado:
            self.fd = sys.stdin.fileno()
            self.old_settings = termios.tcgetattr(self.fd)
            tty.setraw(self.fd)
            self.old_flags = fcntl.fcntl(self.fd, fcntl.F_GETFL)
            fcntl.fcntl(self.fd, fcntl.F_SETFL, self.old_flags | os.O_NONBLOCK)
            self.terminal_configurado = True
    
    def restaurar_terminal(self):
        """Restaura la configuración original del terminal"""
        if self.terminal_configurado:
            termios.tcsetattr(self.fd, termios.TCSADRAIN, self.old_settings)
            fcntl.fcntl(self.fd, fcntl.F_SETFL, self.old_flags)
            self.terminal_configurado = False
    
    def get_key(self):
        """Lee una tecla sin bloquear la ejecución"""
        try:
            return sys.stdin.read(1)
        except:
            return ''
    
    def clear_screen(self):
        """Limpia la pantalla del terminal"""
        os.system('clear')
    
    def agregar_proceso_automatico(self):
        """Crea un nuevo proceso con valores aleatorios"""
        tiempo_max = random.randint(7, 18)
        opcion = random.randint(0, 5)
        data = [random.randint(0, 20), random.randint(1, 20)]  # Evitar división por cero
        op = ['+', '-', '*', '/', '%', '^'][opcion]
        self.nuevos.append(Proceso(self.id_counter, op, data, tiempo_max, self.reloj_global))
        self.id_counter += 1
    
    def agregar_proceso_manual(self):
        """Permite al usuario crear un proceso manualmente"""
        print("\nAgregar nuevo proceso:")
        while True:
            operacion = input("Operación (+, -, *, /, %, ^): ").strip()
            if operacion in ['+', '-', '*', '/', '%', '^']:
                break
            print("Operación no válida. Intente nuevamente.")
        
        while True:
            try:
                dato1 = int(input("Primer operando: "))
                dato2 = int(input("Segundo operando: "))
                if operacion == '/' and dato2 == 0:
                    print("No se puede dividir por cero. Intente nuevamente.")
                    continue
                break
            except ValueError:
                print("Entrada no válida. Intente nuevamente.")
        
        while True:
            try:
                tiempo_max = int(input("Tiempo máximo estimado (7-18): "))
                if 7 <= tiempo_max <= 18:
                    break
                print("El tiempo debe estar entre 7 y 18.")
            except ValueError:
                print("Entrada no válida. Intente nuevamente.")
        
        self.nuevos.append(Proceso(self.id_counter, operacion, [dato1, dato2], tiempo_max, self.reloj_global))
        self.id_counter += 1
    
    def modo_automatico(self):
        """Genera procesos automáticamente"""
        while True:
            try:
                n = int(input("Ingrese el número de procesos: "))
                if n > 0:
                    break
                print("Debe ingresar un número positivo.")
            except ValueError:
                print("Entrada no válida. Intente nuevamente.")
        
        for _ in range(n):
            self.agregar_proceso_automatico()
    
    def modo_manual(self):
        """Permite al usuario ingresar procesos manualmente"""
        while True:
            try:
                n = int(input("Ingrese el número de procesos: "))
                if n > 0:
                    break
                print("Debe ingresar un número positivo.")
            except ValueError:
                print("Entrada no válida. Intente nuevamente.")
        
        for _ in range(n):
            self.agregar_proceso_manual()
    
    def actualizar_estados(self):
        """Actualiza los estados de los procesos"""
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
        
        # Asignar nuevo proceso a ejecución
        if not self.ejecutando and self.listos:
            self.ejecutando = self.listos.popleft()
            self.ejecutando.estado = Estado.EJECUCION
            if self.ejecutando.tiempo_respuesta == -1:
                self.ejecutando.tiempo_respuesta = self.reloj_global - self.ejecutando.tiempo_llegada
    
    def procesar_tecla(self, tecla):
        """Procesa la tecla presionada por el usuario"""
        tecla = tecla.upper()
        if tecla == 'I' and self.ejecutando:  # Interrupción E/S
            self.ejecutando.estado = Estado.BLOQUEADO
            self.ejecutando.tiempo_bloqueado = 0
            self.bloqueados.append(self.ejecutando)
            self.ejecutando = None
            return True
        elif tecla == 'E' and self.ejecutando:  # Error
            self.ejecutando.estado = Estado.TERMINADO
            self.ejecutando.resultado = float('nan')
            self.ejecutando.tiempo_finalizacion = self.reloj_global
            self.terminados.append(self.ejecutando)
            self.ejecutando = None
            return True
        elif tecla == 'N':  # Nuevo proceso
            self.agregar_proceso_automatico()
            return True
        elif tecla == 'P':  # Pausa
            self.mostrar_mensaje("\nSimulación pausada. Presione 'C' para continuar...")
            while True:
                key = self.get_key()
                if key and key.upper() == 'C':
                    break
                time.sleep(0.1)
            return True
        elif tecla == 'B':  # Mostrar tabla de procesos
            self.mostrar_tabla_procesos()
            return True
        return False
    
    def mostrar_mensaje(self, mensaje):
        """Muestra un mensaje en la interfaz"""
        self.clear_screen()
        print(mensaje)
    
    def mostrar_tabla_procesos(self):
        """Muestra la tabla completa de procesos"""
        self.clear_screen()
        print("\n=== TABLA DE PROCESOS (BCP) ===")
        print("-" * 90)
        print("| ID | Estado     | Operación | T.Llegada | T.Finaliz | T.Retorno | T.Espera | T.Servicio | Resultado |")
        print("-" * 90)
        
        # Función auxiliar para mostrar un proceso
        def linea_proceso(p):
            operacion = f"{p.datos[0]} {p.operacion} {p.datos[1]}"
            estado = p.get_estado_string()
            resultado = "ERROR" if math.isnan(p.resultado) else f"{p.resultado:.2f}" if p.estado == Estado.TERMINADO else "N/A"
            finalizacion = p.tiempo_finalizacion if p.estado == Estado.TERMINADO else "N/A"
            retorno = p.tiempo_finalizacion - p.tiempo_llegada if p.estado == Estado.TERMINADO else "N/A"
            
            return f"| {p.id_programa:2} | {estado:10} | {operacion:9} | {p.tiempo_llegada:9} | {finalizacion:9} | {retorno:9} | {p.tiempo_espera:7} | {p.tiempo_servicio:8} | {resultado:9} |"
        
        # Mostrar todos los procesos
        for p in sorted(self.nuevos, key=lambda x: x.id_programa):
            print(linea_proceso(p))
        for p in sorted(self.listos, key=lambda x: x.id_programa):
            print(linea_proceso(p))
        if self.ejecutando:
            print(linea_proceso(self.ejecutando))
        for p in sorted(self.bloqueados, key=lambda x: x.id_programa):
            print(linea_proceso(p))
        for p in sorted(self.terminados, key=lambda x: x.id_programa):
            print(linea_proceso(p))
        
        print("-" * 90)
        print("\nPresione cualquier tecla para continuar...")
        self.get_key()
    
    def mostrar_interfaz(self):
        """Muestra la interfaz principal del simulador"""
        self.clear_screen()
        print("=== SIMULADOR DE PROCESOS ===")
        print(f"Reloj Global: {self.reloj_global} u.t.")
        print("Teclas: I (Interrupción), E (Error), P (Pausa), N (Nuevo), B (Tabla)\n")
        
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
            print("ERROR" if math.isnan(p.resultado) else f"{p.resultado:.2f}")
    
    def ejecutar(self):
        """Ejecuta el ciclo principal del simulador"""
        try:
            self.configurar_terminal()
            
            while self.nuevos or self.listos or self.ejecutando or self.bloqueados:
                self.actualizar_estados()
                self.mostrar_interfaz()
                
                # Procesar teclas con timeout
                start_time = time.time()
                key_pressed = False
                
                while time.time() - start_time < 1.0:  # Esperar hasta 1 segundo
                    key = self.get_key()
                    if key:
                        if self.procesar_tecla(key):
                            key_pressed = True
                            break
                    time.sleep(0.05)
                
                if key_pressed:
                    continue
                
                # Avanzar tiempo de ejecución
                if self.ejecutando:
                    self.ejecutando.tiempo_transcurrido += 1
                    self.ejecutando.tiempo_servicio += 1
                    self.reloj_global += 1
                    
                    if self.ejecutando.tiempo_transcurrido >= self.ejecutando.tiempo_max:
                        self.ejecutando.ejecutar()
                        self.mostrar_mensaje(f"\nProceso {self.ejecutando.id_programa} terminado. Resultado: {'ERROR' if math.isnan(self.ejecutando.resultado) else f'{self.ejecutando.resultado:.2f}'}")
                        
                        self.ejecutando.estado = Estado.TERMINADO
                        self.ejecutando.tiempo_finalizacion = self.reloj_global
                        self.terminados.append(self.ejecutando)
                        self.ejecutando = None
                        time.sleep(1)
                else:
                    self.reloj_global += 1
                
                # Actualizar tiempos de espera
                for p in self.listos:
                    p.tiempo_espera += 1
            
            # Mostrar resultados finales
            self.mostrar_mensaje("\n=== SIMULACIÓN TERMINADA ===")
            print(f"Tiempo total: {self.reloj_global} unidades de tiempo\n")
            
            # Mostrar estadísticas
            if self.terminados:
                total_procesos = len(self.terminados)
                avg_retorno = sum(p.tiempo_finalizacion - p.tiempo_llegada for p in self.terminados) / total_procesos
                avg_espera = sum(p.tiempo_espera for p in self.terminados) / total_procesos
                avg_servicio = sum(p.tiempo_servicio for p in self.terminados) / total_procesos
                
                print("=== ESTADÍSTICAS FINALES ===")
                print(f"Procesos completados: {total_procesos}")
                print(f"Tiempo promedio de retorno: {avg_retorno:.2f}")
                print(f"Tiempo promedio de espera: {avg_espera:.2f}")
                print(f"Tiempo promedio de servicio: {avg_servicio:.2f}")
            
            print("\nPresione cualquier tecla para salir...")
            self.get_key()
        
        except KeyboardInterrupt:
            print("\nSimulación interrumpida por el usuario")
        finally:
            self.restaurar_terminal()

def main():
    sim = Simulador()
    
    print("=== SIMULADOR DE PROCESOS ===")
    print("1. Modo automático")
    print("2. Modo manual")
    
    while True:
        opcion = input("Seleccione el modo (1/2): ").strip()
        if opcion == '1':
            sim.modo_automatico()
            break
        elif opcion == '2':
            sim.modo_manual()
            break
        else:
            print("Opción no válida. Intente nuevamente.")
    
    sim.ejecutar()

if __name__ == "__main__":
    main()