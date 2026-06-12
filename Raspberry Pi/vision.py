import cv2
import time
from datetime import datetime
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

# ==========================================
# CONFIGURACION INFLUXDB (Para Grafana)
# ==========================================
TOKEN = "xAKRNJwnsAdMfaXWQ_5bSYyi81wWYIlWZcenpu8TxNe4d10XWQQ9Z1g0ayp9pngJswZQgzEBIvttTvupjw13Wg==" 
ORG = "TE2003B"
BUCKET = "reto"
URL = "http://192.168.137.73:8086"

influx_client = InfluxDBClient(url=URL, token=TOKEN, org=ORG)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)


def registrar_alerta_influx(estado_num, descripcion):
    """
    Guarda el estado del conductor en la base de datos.
    estado_num: 0 (Normal), 1 (Distraccion), 2 (Fatiga)
    """
    try:
        punto = Point("monitoreo_cabina") \
            .tag("id_maquina", "tractor_01") \
            .field("estado_alerta", estado_num) \
            .field("descripcion", descripcion)
            
        write_api.write(bucket=BUCKET, org=ORG, record=punto)
        print(f"[{datetime.now().strftime('%H:%M:%S')}] OK Guardado en DB -> Estado: {descripcion}")
        
    except Exception as e:
        print(f"\n[ERROR CRITICO InfluxDB]: {e}")
# ==========================================
# EJECUCION PRINCIPAL DE VISION
# ==========================================
def main():
    # Cargar los clasificadores pre-entrenados de Haar Cascades
    face_cascade = cv2.CascadeClassifier('/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml')
    eye_cascade = cv2.CascadeClassifier('/usr/share/opencv4/haarcascades/haarcascade_eye.xml')

    # Iniciar la camara
    cap = cv2.VideoCapture(0)

    # Variables de estado y contadores de frames
    frames_sin_rostro = 0
    frames_ojos_cerrados = 0

    # Umbrales 
    UMBRAL_DISTRACCION = 20  
    UMBRAL_FATIGA = 15       

    # Control de tiempo para no saturar InfluxDB
    ultimo_envio = 0
    COOLDOWN_ALERTA = 2.0  # Segundos de espera entre envio de alertas
    COOLDOWN_NORMAL = 5.0  # Segundos para reportar estado "Normal" y mantener la grafica viva

    print("Iniciando monitoreo de cabina... (Presiona 'q' para salir)")

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Error al acceder a la camara.")
            break

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        rostros = face_cascade.detectMultiScale(gray, scaleFactor=1.3, minNeighbors=5, minSize=(100, 100))

        estado_actual = 0  # 0 = Normal, 1 = Distraccion, 2 = Fatiga
        texto_alerta = "Normal"

        # LOGICA DE DISTRACCION
        if len(rostros) == 0:
            frames_sin_rostro += 1
            if frames_sin_rostro > UMBRAL_DISTRACCION:
                estado_actual = 1
                texto_alerta = "Distraccion"
                cv2.putText(frame, "ALERTA: DISTRACCION!", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)
        else:
            frames_sin_rostro = 0 # Reiniciar contador si se ve el rostro

            for (x, y, w, h) in rostros:
                cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)
                
                # Recortar solo la mitad superior del rostro para buscar los ojos
                roi_gray_superior = gray[y:y+int(h/2), x:x+w]
                roi_color_superior = frame[y:y+int(h/2), x:x+w]

                ojos = eye_cascade.detectMultiScale(roi_gray_superior, scaleFactor=1.1, minNeighbors=20, minSize=(25, 25))

                # LOGICA DE FATIGA
                if len(ojos) == 0:
                    frames_ojos_cerrados += 1
                    if frames_ojos_cerrados > UMBRAL_FATIGA:
                        estado_actual = 2
                        texto_alerta = "Fatiga"
                        cv2.putText(frame, "ALERTA: FATIGA (Ojos cerrados)!", (50, 100), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 165, 255), 3)
                else:
                    frames_ojos_cerrados = 0 
                    for (ex, ey, ew, eh) in ojos:
                        # Dibujar el rectangulo en el frame original usando las coordenadas ajustadas
                        cv2.rectangle(frame, (x+ex, y+ey), (x+ex+ew, y+ey+eh), (0, 255, 0), 2)

        # ==========================================
        # ENVIO DE DATOS A INFLUXDB
        # ==========================================
        tiempo_actual = time.time()
        
        if estado_actual != 0 and (tiempo_actual - ultimo_envio > COOLDOWN_ALERTA):
            registrar_alerta_influx(estado_actual, texto_alerta)
            ultimo_envio = tiempo_actual
            
        elif estado_actual == 0 and (tiempo_actual - ultimo_envio > COOLDOWN_NORMAL):
            registrar_alerta_influx(0, "Normal")
            ultimo_envio = tiempo_actual

        # Mostrar el frame procesado
        cv2.imshow('Monitoreo de Cabina - Operador', frame)

        # Salir con la tecla 'q'
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Liberar recursos
    cap.release()
    cv2.destroyAllWindows()
    influx_client.close()
if __name__ == "__main__":
    main()

