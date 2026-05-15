#include <WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "iPhone de Diego (4)";
const char* password = "NIKO2016";
const char* mqtt_server = "192.168.137.62"; 
const int mqtt_port = 1883; 
const char* mqtt_user = "";   
const char* mqtt_password = ""; 
const char* motor_data_topic = "motor/data";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const int msgInterval = 2000;

float vel_motor = 10.0;
float vel_veh = 5.0;
String marchaS = "";

void setup_wifi() { 
  delay(10); 
  Serial.println(); 
  Serial.print("Conectando a "); 
  Serial.println(ssid); 
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  } 
  Serial.println(""); 
  Serial.println("WiFi conectado"); 
  Serial.println("Dirección IP: "); 
  Serial.println(WiFi.localIP()); 
} 

void reconnect() { 
  while (!client.connected()) { 
    Serial.print("Intentando conectar a MQTT..."); 
    if (client.connect("ESP32_Tractor_Client", mqtt_user, mqtt_password)) { 
      Serial.println("conectado"); 
    } else { 
      Serial.print("falló, rc="); 
      Serial.print(client.state()); 
      Serial.println(" intentando de nuevo en 5 segundos"); 
      delay(5000); 
    } 
  } 
} 

void setup() {
  Serial.begin(115200); 
  delay(3000); // Tiempo para que el ESP32-C6 abra el puerto serial
  randomSeed(analogRead(0));
  setup_wifi(); 
  client.setServer(mqtt_server, mqtt_port); 
  Serial1.begin(115200, SERIAL_8N1, 4, 5); 
}

void loop() {
  if (!client.connected()) { 
    reconnect(); 
  } 
  client.loop(); 
  
  // =================================================================
  // 1. LEER DATOS DEL STM32 CONSTANTEMENTE (Fuera del temporizador)
  // =================================================================
  while (Serial1.available()) {
    // Leemos la línea hasta el salto de línea y limpiamos espacios o retornos de carro (\r)
    String incomingData = Serial1.readStringUntil('\n');
    incomingData.trim(); 

    // Variables temporales para guardar lo extraído
    float temp_thr = 0.0;
    float temp_rpm = 0.0;
    int temp_gear = 0;

    // MAGIA DE EXTRACCIÓN: sscanf busca el texto exacto y saca los valores numéricos
    // %f es para float, %d es para enteros (int)
    int leidos = sscanf(incomingData.c_str(), "Thr: %f | RPM: %f | Gear: %d", &temp_thr, &temp_rpm, &temp_gear);

    // Si sscanf encontró exactamente los 3 valores, actualizamos las variables globales
    if (leidos == 3) {
      vel_motor = temp_thr;             // Thr -> motor
      vel_veh   = temp_rpm;             // RPM -> vl
      marchaS   = String(temp_gear);    // Gear -> marcha (Convertido a String para el JSON)
      
      // Opcional: Imprimir en consola para verificar que se parseó bien
      // Serial.printf("Extraído -> Motor: %.2f, Vl: %.1f, Marcha: %s\n", vel_motor, vel_veh, marchaS.c_str());
    }
  }

  // =================================================================
  // 2. PUBLICAR POR MQTT CADA 2 SEGUNDOS
  // =================================================================
  long now = millis(); 
  if (now - lastMsg > msgInterval) { 
    lastMsg = now; 
    
    // Crear el documento JSON con los DATOS REALES
    JsonDocument jsonDocument; 
    jsonDocument["motor"] = vel_motor; 
    jsonDocument["vl"] = vel_veh; 
    jsonDocument["marcha"] = marchaS;
 
    // Convertir el documento JSON a una cadena 
    String jsonString; 
    serializeJson(jsonDocument, jsonString); 
    
    // Publicar el mensaje JSON MQTT 
    Serial.print("Publicando datos: "); 
    Serial.println(jsonString); 
    client.publish(motor_data_topic, jsonString.c_str()); 
  }
}