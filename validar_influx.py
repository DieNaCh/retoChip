import time
import random
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS
# CONFIGURACION (agregar sus datos de configuracion en InfluxDB)
TOKEN = "xAKRNJwnsAdMfaXWQ_5bSYyi81wWYIlWZcenpu8TxNe4d10XWQQ9Z1g0ayp9pngJswZQgzEBIvttTvupjw13Wg==" # Obtenido de http://localhost:8086
ORG = "TE2003B"
BUCKET = "reto"
URL = "http://localhost:8086"
def validar_conexion():
	# 1. Creamos el cliente de conexion
	client = InfluxDBClient(url=URL, token=TOKEN, org=ORG)
	# 2. Configuramos la API de escritura (Modo Sincrono para validar errores al instante)
	write_api = client.write_api(write_options=SYNCHRONOUS)
	print(f"--- Iniciando Validacion de InfluxDB ---")
	print(f"Conectando a {URL}...")
	try:
		while True:
			# SIMULACIoN DE DATOS DEL TRACTOR
			# Imaginemos que leemos sensores reales
			rpm_actual = random.randint(1500, 5000)
			vel_actual = round(random.uniform(10.5, 12.8), 2)
			# 3. CONSTRUCCION DEL PUNTO (DATA POINT)
			# measurement: nombre de la tabla
			# tag: metadatos indexados (fijos)
			# field: valores numericos (dinamicos)
			punto = Point("monitoreo_tractor") \
			.tag("id_maquina", "tractor_01") \
			.tag("sensor_set", "motor_v1") \
			.field("rpm", rpm_actual) \
			.field("velocidad", vel_actual) \
			# 4. ENVIO A LA BASE DE DATOS
			write_api.write(bucket=BUCKET, org=ORG, record=punto)
			print(f"[OK] Datos enviados -> RPM: {rpm_actual} | Vel: {vel_actual} km/h")
			# Esperamos 1 segundo entre envios
			time.sleep(1)
	except KeyboardInterrupt:
		print("\n--- Validacion Finalizada por el Usuario ---")
	except Exception as e:
		print(f"\n[ERROR CRITICO]: {e}")
		print("Revisa si el TOKEN, la ORG o el BUCKET son correctos.")
	finally:
		client.close()
if __name__ == "__main__":
	validar_conexion()
