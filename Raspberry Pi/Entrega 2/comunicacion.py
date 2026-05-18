import paho.mqtt.client as mqtt 

import time 

 

MQTT_BROKER = "192.168.137.62"  

MQTT_PORT = 1883 

MQTT_TOPIC = "motor/data" 

 

def on_connect(client, userdata, flags, reason_code, properties): 

    if reason_code == 0: 

        print("Conectado al broker MQTT") 
        client.subscribe(MQTT_TOPIC)

    else: 

        print(f"Error al conectar, código: {reason_code}") 

def on_message(client, userdata, msg):
    mensaje = msg.payload.decode()
    print(f"Mensaje recibido: {mensaje}")

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2) 

client.on_connect = on_connect 

client.on_message = on_message
 

client.connect(MQTT_BROKER, MQTT_PORT, 60)  

try: 

    while True: 

        client.loop_forever()

except KeyboardInterrupt: 

    print("Desconectando...") 

    client.loop_stop() 

    client.disconnect() 
