#include <WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "iPhone de Diego (4)";
const char* password = "NIKO2016";
const char* mqtt_server = "192.168.137.73"; 
const int mqtt_port = 1883; 
const char* mqtt_user = "";   
const char* mqtt_password = ""; 
const char* motor_data_topic = "motor/data";
const char* motor_control_topic = "motor/control";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const int msgInterval = 120;

float vel_motor = 10.0;
float vel_sped = 5.0;
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

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("\n[MQTT] Recibido en tópico: ");
  Serial.println(topic);
  Serial.println(message);

  // Controlar el tractor, enviando a STM
  if (String(topic) == motor_control_topic) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
      Serial.print("Error al registrar JSON de control: ");
      Serial.println(error.c_str());
      return;
    }

    // Extraer datos del JSON
    int accel = doc["acelerador"] | 0;
    bool freno = doc["freno"] | false;
    bool cont_rem = doc["control_remoto"] | false;

    // Formatear y enviar
    String comando = "A," + String(accel) +",B," + String(freno) + ",R," + String(cont_rem) + "\n";

    Serial1.print(comando);

    Serial.print("[Serial] Enviado a STM32: ");
    Serial.println(comando);
  }
}

void setup() {
  Serial.begin(115200); 
  delay(3000); 
  randomSeed(analogRead(0));
  setup_wifi(); 
  client.setServer(mqtt_server, mqtt_port); 
  client.setCallback(callback);
  Serial1.begin(115200, SERIAL_8N1, 4, 5); 
}

void loop() {
  if (!client.connected()) { 
    reconnect(); 
  } 
  client.loop(); 
  

  while (Serial1.available()) {
    String incomingData = Serial1.readStringUntil('\n');
    incomingData.trim(); 

    // Variables temporales para guardar lo extraído
    float temp_thr = 0.0;
    float temp_vel = 0.0;
    float temp_rpm = 0.0;
    int temp_gear = 0;


    int leidos = sscanf(incomingData.c_str(), "T: %f | S: %f | R: %f | G: %d", &temp_thr, &temp_vel, &temp_rpm, &temp_gear);

    if (leidos == 4) {
      vel_motor = temp_thr;             // Thr -> motor
      vel_sped = temp_vel;              //Velocity -> sped
      vel_veh   = temp_rpm;             // RPM -> vl
      marchaS   = String(temp_gear);    // Gear -> marcha (Convertido a String para el JSON)
    }
  }


  long now = millis(); 
  if (now - lastMsg > msgInterval) { 
    lastMsg = now; 
    
    JsonDocument jsonDocument; 
    jsonDocument["motor"] = vel_motor; 
    jsonDocument["spd"] = vel_sped;
    jsonDocument["vl"] = vel_veh; 
    jsonDocument["marcha"] = marchaS;
 
    String jsonString; 
    serializeJson(jsonDocument, jsonString); 
    
    Serial.print("Publicando datos: "); 
    Serial.println(jsonString); 
    client.publish(motor_data_topic, jsonString.c_str()); 
  }
}
