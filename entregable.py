import random
import math
import csv 
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt  

# Delimitación de valores aleatorios para las variables
min_radio = 0.6
max_radio = 1.37

min_vel_ang = 7.0
max_vel_ang = 16.0

min_rel_transmision = 8.5
max_rel_transmision = 11.0

num_resultados = 25
random_rel_transmision = random.uniform(min_rel_transmision, max_rel_transmision)

# Donde se guardan los datos
ruta_archivo_datos = 'valores.csv'

# Títulos de filas de encabezado para el .csv 
head = ["Velocidad Angular (rad/s)", "Radio de la rueda (m)", "rel_transmision de Transmisión", "RPM"]
data = [head]

# Listas para almacenar valores obtenidos e iteraciones
rpm_valores = []
velocidad_angular = []
radio_rueda = []
iteraciones = []

# Setup del gráfico interactivo
plt.ion()
fig, (rpm_grafica, radio_grafica, vel_ang_grafica) = plt.subplots(3, 1, figsize=(15, 20))
fig.tight_layout(pad=5.0)
fig.subplots_adjust(hspace=0.5)

# Subplot 1: Gráfico de revoluciones por minuto en el tiempo
line_rpm, = rpm_grafica.plot([], [], marker='o', color='b', label='RPM')
rpm_grafica.set_xlim(0, num_resultados - 1)
rpm_grafica.set_ylim(0, 30)
rpm_grafica.set_title('RPM en el Tiempo')
rpm_grafica.set_xlabel('Tiempo (s)')
rpm_grafica.set_ylabel('Revoluciones por Minuto (RPM)')
rpm_grafica.grid(True)

# Subplot 2: Gráfico de radio de la rueda en el tiempo
line_radio, = radio_grafica.plot([], [], marker='o', color='g', label='Radio')
radio_grafica.set_xlim(0, num_resultados - 1)
radio_grafica.set_ylim(min_radio - 0.5, max_radio + 0.5)
radio_grafica.set_title('Radio de la Rueda en el Tiempo')
radio_grafica.set_xlabel('Tiempo (s)')
radio_grafica.set_ylabel('Radio (m)')
radio_grafica.grid(True)

# Subplot 3: Gráfico de velocidad angular en el tiempo
line_vel, = vel_ang_grafica.plot([], [], marker='o', color='r', label='Velocidad Angular')
vel_ang_grafica.set_xlim(0, num_resultados - 1)
vel_ang_grafica.set_ylim(min_vel_ang - 3, max_vel_ang + 3)
vel_ang_grafica.set_title('Velocidad Angular en el Tiempo')
vel_ang_grafica.set_xlabel('Tiempo (s)')
vel_ang_grafica.set_ylabel('Velocidad Angular (rad/s)')
vel_ang_grafica.grid(True)

# Generación de datos
for i in range(num_resultados):
    # Obtención de valores aleatorios
    random_radio = random.uniform(min_radio, max_radio)
    random_vel_ang = random.uniform(min_vel_ang,  max_vel_ang)

    # Cálculo de RPM, basado en:
    # RPM = (Velocidad angular de la rueda * 60) / (2 * π       * Radio de la rueda * Relación de transmisión)
    r_RPM = (random_vel_ang                * 60) / (2 * math.pi * random_radio      * random_rel_transmision        )

    # Agregar datos a los arreglos correspondientes, redondeándolos a dos decimales
    data_val = [round(random_vel_ang, 2), round(random_radio, 2), round(random_rel_transmision, 2), round(r_RPM, 2)]
    data.append(data_val)

    rpm_valores.append(r_RPM)
    radio_rueda.append(random_radio)
    velocidad_angular.append(random_vel_ang)
    iteraciones.append(i)

    # Ajuste automático de los ejes si los valores se salen del rango
    if r_RPM > rpm_grafica.get_ylim()[1]:
        rpm_grafica.set_ylim(0, r_RPM + 10)
    if random_radio > radio_grafica.get_ylim()[1]:
        radio_grafica.set_ylim(0, random_radio + 10)
    if random_vel_ang > vel_ang_grafica.get_ylim()[1]:
        vel_ang_grafica.set_ylim(0, random_vel_ang + 10)

    # Actualizar gráficas a partir de los datos obtenidos
    line_rpm.set_data(iteraciones, rpm_valores)
    line_radio.set_data(iteraciones, radio_rueda)
    line_vel.set_data(iteraciones, velocidad_angular)

    plt.draw()

    plt.pause(1) # Para simular un valor por segundo

    # Impresión de parámetros (debug)
    print(f"radio: {random_radio: .2f} m")
    print(f"Velocidad: {random_vel_ang: .2f} rad/s")
    print(f"rel_transmision: {random_rel_transmision: .2f}")
    print(f"RPM: {r_RPM: .2f} RPM")

# Guardar datos obtenidos en el .csv
with open(ruta_archivo_datos, mode = 'w', newline = '') as csv_file:
    writer = csv.writer(csv_file)
    writer.writerows(data)

# Terminar interactividad de la gráfica
plt.ioff()
plt.show()