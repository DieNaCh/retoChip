import random
import math
import csv 
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

with open(output_file_path, mode = 'w', newline = '') as csv_file:
    # Guardar datos obtenidos en el .csv
    writer = csv.writer(csv_file)
    writer.writerows(data)

    # Generar gráficas a partir de los datos obtenidos
    plt.figure(figsize=(8, 5))
    plt.plot(range(1, num_results + 1), rpm_values, marker='o', linestyle='-', color='b', label='RPM')

    plt.title('Valores de RPM Generados Aleatoriamente')
    plt.xlabel('Numero de Iteración / Prueba')
    plt.ylabel('Revoluciones Por Minuto (RPM)')
    plt.grid(True)
    plt.legend()

    plt.show()
