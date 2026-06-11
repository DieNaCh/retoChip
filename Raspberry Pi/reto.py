import paho.mqtt.client as mqtt 
import json
from datetime import datetime

from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

from flask import Flask, request, jsonify
from flask_cors import CORS
import threading

TOKEN = "xAKRNJwnsAdMfaXWQ_5bSYyi81wWYIlWZcenpu8TxNe4d10XWQQ9Z1g0ayp9pngJswZQgzEBIvttTvupjw13Wg==" 
ORG = "TE2003B"
BUCKET = "reto"
URL = "http://localhost:8086"

MQTT_BROKER = "192.168.137.73"  
MQTT_PORT = 1883 
MQTT_TOPIC = "motor/data"

# ==========================================
# FLASK API (Para comandos de Grafana a STM32)
# ==========================================
app = Flask(__name__)
CORS(app)

@app.route('/control', methods=['POST'])
def receive_grafana_command():
    data = request.json
    print(f"[API] Comando recibido desde Grafana: {data}")
    
    # Publicar a tópico
    mqtt_client.publish("motor/control", json.dumps(data))
    
    return jsonify({"status": "Comando enviado"}), 200

def run_flask():
    # Corre el servidor
    app.run(host='0.0.0.0', port=5000, debug=False, use_reloader=False) 


# ==========================================
# CSV
# ==========================================

def write_to_csv(timestamp, rpm, vl, speed, marcha):
    with open(CSV_FILENAME, 'a', newline='') as csvfile:
        writer = csv.writer(csvfile)

        # Escribir encabezado solo si el archivo esta vacio
        if csvfile.tell() == 0:
            writer.writerow(CSV_HEADER)

        writer.writerow([
            timestamp,
            f"{rpm:.2f}",
            f"{vl:.2f}",
            marcha,
            f"{speed:.2f}",
        ])

# ==========================================
# INFLUXDB
# ==========================================
influx_client = InfluxDBClient(url=URL, token=TOKEN, org=ORG)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

def guardar_en_influx(motor, vl, marcha, speed):
    try:
        motor_float = float(motor)
        vl_float = float(vl)        
        speed_float = float(speed)
        marcha_int = int(float(marcha))
        
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        punto = Point("monitoreo_tractor") \
            .tag("id_maquina", "tractor_01") \
            .tag("sensor_set", "motor_v1") \
            .field("thr", motor_float) \
            .field("rpm", vl_float) \
            .field("speed", speed_float) \
            .field("marcha", marcha_int)
            
        write_api.write(bucket=BUCKET, org=ORG, record=punto)
        
        #write_to_csv(timestamp, motor_float, vl_float, speed_float, marcha_int)

        
        print(f"[OK] Guardado en DB -> Thr: {motor_float:.2f} | Spd: {speed_float:.1f} | RPM: {vl_float:.1f} | Gear: {marcha_int}")
        
    except Exception as e:
        print(f"\n[ERROR CRITICO InfluxDB]: {e}")

# ==========================================
# FUNCIONES MQTT
# ==========================================
def on_connect(client, userdata, flags, reason_code, properties): 
    if reason_code == 0: 
        print(f"[OK] Conectado al broker MQTT en {MQTT_BROKER}") 
        client.subscribe(MQTT_TOPIC)
    else: 
        print(f"[ERROR] Fallo al conectar, codigo: {reason_code}") 

def on_message(client, userdata, msg):
    try:
        mensaje = msg.payload.decode()
        data = json.loads(mensaje)

        
        motor_val = data["motor"]
        vl_val = data["vl"]
        sped_val = data["spd"]
        marcha_val = data["marcha"]
        
        guardar_en_influx(motor_val, vl_val, marcha_val, sped_val)

    except json.JSONDecodeError:
        print("[ERROR] El mensaje recibido no es un JSON valido.")
    except KeyError as e:
        print(f"[ERROR] Falta la clave {e} en el JSON recibido.")

# ==========================================
# EJECUCION PRINCIPAL
# ==========================================
mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2) 
mqtt_client.on_connect = on_connect 
mqtt_client.on_message = on_message

print("Iniciando sistema de recepcion...")
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)  

api_thread = threading.Thread(target=run_flask)
api_thread.daemon = True
api_thread.start()

try: 
    mqtt_client.loop_forever()

except KeyboardInterrupt: 
    print("\n--- Desconectando del sistema ---") 
    mqtt_client.loop_stop() 
    mqtt_client.disconnect() 
    influx_client.close() 
