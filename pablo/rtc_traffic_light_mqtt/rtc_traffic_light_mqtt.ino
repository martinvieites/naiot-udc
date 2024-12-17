// LIBRERIAS SENSORES
#include <Wire.h>
#include <RTClib.h>
// LIBRERIAS COMUNICACIONES
#include <WiFi.h>
#include <PubSubClient.h>

// CONSTANTES
#define DAY_NIGHT_INTERRUPTION 12
#define GREEN_LED 32
#define YELLOW_LED 25
#define RED_LED 27
const char* ssid = "LMDG";
const char* password = "pabloRivada ";
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";

// VARIABLES 
WiFiClient espClient;
PubSubClient client(espClient); 

TaskHandle_t xTaskupdateMqtt = NULL;
TaskHandle_t xTaskCheckMqtt = NULL;
bool dia = true;
int estado = 0;

// FUNCIONES
void TaskupdateMqttCode( void * pvParameters ) {
  while (1) {
    delay(10000);
    // dia = true;
    Serial.println("updateMqtt.Es de dia.");
    client.publish("devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/dia", "1"); 
    delay(10000);

    // dia = false;
    Serial.println("updateMqtt.Es de noche.");
    client.publish("devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/dia", "0"); 
  }
}
// FUNCIONES
void TaskCheckMqttCode( void * pvParameters ) {
  while (1) {
        
    // if (!client.connected()) {
    //   reconnect();   
    // }   
    client.loop();
    
    if (!dia) {  // Modo noche
      Serial.println("Es de noche. Me mantengo parpadeando en ambar");
      digitalWrite(YELLOW_LED, HIGH);
      delay(500);
      digitalWrite(YELLOW_LED, LOW);
      delay(500);
    }
  
  }
}

void setNetwork() {
  WiFi.mode(WIFI_STA);
  WiFi.begin("LMDG", "pabloRivada");
  Serial.print(F("Esperando red"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(F("."));
  }
  Serial.println("WiFi listo.");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback); 
  while (!client.connected()) {
    Serial.println("Conectando ao broker MQTT...");
    if (client.connect(
        "PabloNapiotPrueba1",
         mqttUser, mqttPassword ))
      Serial.println("Conectado ao broker MQTT!");
    else {
      Serial.print("Erro ao conectar co broker: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.subscribe("devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/car/status");
  Serial.println("Red lista.");
}


// Función de callback que procesa as mensaxes MQTT recibidas 
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("received message[");
  Serial.print(topic);
  Serial.print("] ");

  // TODO: No hacer print si es de noche
  // if (!dia) {
  //   return;
  // }

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];   
  }   
  Serial.println(message);    // Controlamos o estado do LED en fucnión da mensaxe   
  if (message == "0") {
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(RED_LED, LOW);
        Serial.println("semaforo verde");
        return;
  }
  if (message == "1") {
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(YELLOW_LED, HIGH);
        digitalWrite(RED_LED, LOW);
        Serial.println("semaforo amarillo");
        return;
  }
  if (message == "2") {
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(RED_LED, HIGH);
        Serial.println("semaforo rojo");
        return;
  }
  if (message == "3") {
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(RED_LED, LOW);
        Serial.println("semaforo apagado");
        return;
  }
  Serial.println("Valor no reconocido para semaforo");
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
  
  return;
  // switch (message.toInt()) {
  //   case 0: // verde
  //     {
  //       digitalWrite(GREEN_LED, HIGH);
  //       digitalWrite(YELLOW_LED, LOW);
  //       digitalWrite(RED_LED, LOW);
  //     }
  //   case 1: // amarillo
  //     {
  //       digitalWrite(GREEN_LED, LOW);
  //       digitalWrite(YELLOW_LED, HIGH);
  //       digitalWrite(RED_LED, LOW);
  //     }
  //   case 2: // rojo
  //     {
  //       digitalWrite(GREEN_LED, LOW);
  //       digitalWrite(YELLOW_LED, LOW);
  //       digitalWrite(RED_LED, HIGH);
  //     }
  //   case 3: //apagado
  //     {
  //       digitalWrite(GREEN_LED, LOW);
  //       digitalWrite(YELLOW_LED, LOW);
  //       digitalWrite(RED_LED, LOW);
  //     }
  //   default:
  //   {
  //       digitalWrite(GREEN_LED, HIGH);
  //       digitalWrite(YELLOW_LED, HIGH);
  //       digitalWrite(RED_LED, HIGH);

  //   }
  // }
}

// Reconecta co broker MQTT se se perde a conexión 
void reconnect() {   
  while (!client.connected()) {     
    Serial.print("Intentando conectar a broker MQTT...");          // Inténtase conectar indicando o ID do dispositivo     
    //IMPORTANTE: este ID debe ser único!     
    if (client.connect("PabloNapiotPrueba1", mqttUser, mqttPassword)) {       
      Serial.println("conectado!");              // Subscripción ao topic       
      client.subscribe("devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/car/status");
      Serial.println("Subscrito ao topic");
    } else { 
      Serial.print("erro na conexión, erro=");       
      Serial.print(client.state());       
      Serial.println(" probando de novo en 5 segundos");       
      delay(5000);     
    }   
  } 
}  

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Configuracion de pines
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(RED_LED, HIGH);

  //Conexion WiFi
  setNetwork();
  
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // if (xTaskCreate(TaskupdateMqttCode,
  //                 "Check MQTT",
  //                 10000,
  //                 NULL,
  //                 1,
  //                 &xTaskupdateMqtt)
  //     != pdPASS) {
  //   Serial.println("Error al crear tarea Update.");
  // }
  
  if (xTaskCreate(TaskCheckMqttCode,
                  "Check MQTT",
                  10000,
                  NULL,
                  1,
                  &xTaskCheckMqtt)
      != pdPASS) {
    Serial.println("Error al crear tarea Check.");
  }
}

void loop() {
  
  
}