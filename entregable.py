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
velocidad_angular = []
radio_rueda = []

# Gráfico interactivo
plt.ion()
fig, (rpmgraph, radgrad, velgrad) = plt.subplots(3, 1, figsize=(15, 20))
fig.tight_layout(pad=4.0)

line_rpm, = rpmgraph.plot([], [], marker='o', color='b', label='RPM')
rpmgraph.set_xlim(0, num_results)
rpmgraph.set_ylim(0,30)
rpmgraph.set_title('Valores de RPM')
rpmgraph.set_ylabel('RPM')
rpmgraph.grid(True)

line_radio, = radgrad.plot([], [], marker='o', color='g', label='radio')
radgrad.set_xlim(0, num_results)
radgrad.set_ylim(min_radius - 0.5, max_radius + 0.5)
radgrad.set_title('Valores del radio')
radgrad.set_ylabel('m')
radgrad.grid(True)

line_vel, = velgrad.plot([], [], marker='o', color='r', label='vel')
velgrad.set_xlim(0, num_results)
velgrad.set_ylim(min_speed - 3, max_speed + 3)
velgrad.set_title('Valores de velocidad')
velgrad.set_ylabel('rad/s')
velgrad.grid(True)

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
    velocidad_angular.append(random_speed)
    radio_rueda.append(random_radius)
    iteraciones.append(i)

    # Generar gráficas a partir de los datos obtenidos
    line_radio.set_data(iteraciones, radio_rueda)
    line_rpm.set_data(iteraciones, rpm_values)
    line_vel.set_data(iteraciones, velocidad_angular)
    # Ajuste automático de los ejes si los valores se salen del rango
    if r_RPM > rpmgraph.get_ylim()[1]:
        rpmgraph.set_ylim(0, r_RPM + 10)
    
    plt.draw()

    plt.pause(0.5)

with open(output_file_path, mode = 'w', newline = '') as csv_file:
    # Guardar datos obtenidos en el .csv
    writer = csv.writer(csv_file)
    writer.writerows(data)

plt.ioff()
plt.show()