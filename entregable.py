import random
import math
import csv 
import time
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt  

# Delimitación de valores aleatorios para las variables
min_radius = 0.6
max_radius = 1.37

min_speed = 7.0
max_speed = 16.0

min_relacion = 8.5
max_relacion = 11.0

num_results = 100
random_relacion = random.uniform(min_relacion, max_relacion)

# Donde se guardan los datos
output_file_path = 'values.csv'

# Títulos de filas de encabezado para el .csv 
head = ["Velocidad Angular", "Radio de la rueda (m)", "Relacion de Transmision", "RPM"]
data = [head]

# Generar aleatoriamente y almacenar valores de RPM
rpm_values = []
iteraciones = []

# Gráfico interactivo
plt.ion()
fig, ax = plt.subplots(figsize=(8, 5))
line, = ax.plot([], [], marker='o', linestyle='-', color='b', label='RPM')

ax.set_xlim(0, num_results)
ax.set_ylim(0, 30)
ax.set_title('Valores de RPM Generados Aleatoriamente')
ax.set_xlabel('Número de Iteración')
ax.set_ylabel('Revoluciones por Minuto (RPM)')
ax.grid(True)
ax.legend()

for i in range(num_results):
    random_radius = random.uniform(min_radius, max_radius)
    random_speed = random.uniform(min_speed,  max_speed)
    r_RPM= (random_speed * 60) / (2 * math.pi * random_radius * random_relacion)
    print(f"radio: {random_radius: .2f} m")
    print(f"Velocidad: {random_speed: .2f} rad/s")
    print(f"Relacion: {random_relacion: .2f}")
    print(f"RPM: {r_RPM: .2f} RPM")
    data_val = [round(random_speed, 2), round(random_radius, 2), round(random_relacion, 2), round(r_RPM, 2)]
    data.append(data_val)

    rpm_values.append(r_RPM)
    iteraciones.append(i)

    # Generar gráficas a partir de los datos obtenidos
    line.set_data(iteraciones, rpm_values)
    
    # Ajuste automático de los ejes si los valores se salen del rango
    if r_RPM > ax.get_ylim()[1]:
        ax.set_ylim(0, r_RPM + 10)
    
    plt.draw()

    plt.pause(0.5)

with open(output_file_path, mode = 'w', newline = '') as csv_file:
    # Guardar datos obtenidos en el .csv
    writer = csv.writer(csv_file)
    writer.writerows(data)

plt.ioff()
plt.show()