#include <WiFi.h>
#include <PubSubClient.h>

// constantes
#define PEDESTRIAN_GREEN_STATUS "0"
#define PUB_DELAY 5000

// Credenciais da rede Wifi
const char* ssid = "";
const char* password = "";

// Configuración do broker MQTT
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/pedestrian/status";

int buzzerPin = 33;  // buzzer: pin señal a IO33
int inputPin = 27;   // sensor ultrasonidos: pin ECHO a IO27
int outputPin = 25;  // sensor ultrasonidos: pin TRIG a IO25

int buzzerFlag = 0;  // activado:1 desactivado:0

// Led que se acenderá/apagará
/*const int ledPin = LED_BUILTIN;*/

WiFiClient espClient;
PubSubClient client(espClient);

TaskHandle_t xTaskBuzzer = NULL;
TaskHandle_t xTaskUltrasonicMqtt = NULL;


// Función para conectar á WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectada!");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}


// Función de callback que procesa as mensaxes MQTT recibidas
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaxe recibida[");
  Serial.print(topic);
  Serial.print("] ");

  // Imprimese o payload da mensaxe
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // dependiendo del mensaje recibido se activa flag que enciende zumbado
  if (message == PEDESTRIAN_GREEN_STATUS) {
    buzzerFlag = 1;
  } else {
    buzzerFlag = 0;
  }
}

// Reconecta co broker MQTT se se perde a conexión
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar a broker MQTT...");
    // Inténtase conectar indicando o ID do dispositivo
    //IMPORTANTE: este ID debe ser único!
    if (client.connect("NAPIoT-P2-Rec-Clara")) {
      Serial.println("conectado!");
      // Subscripción ao topic
      client.subscribe(mqtt_topic);
      Serial.println("Subscrito ao topic");
    } else {
      Serial.print("erro na conexión, erro=");
      Serial.print(client.state());
      Serial.println(" probando de novo en 5 segundos");
      delay(5000);
    }
  }
}

// TASKS
void taskBuzzer(void* pvParameters) {
  int i;
  while (1) {
    if (buzzerFlag) {
      Serial.println("Se activa buzzer.");
      for (i = 0; i < 80; i++)  // output a frequency sound
      {
        digitalWrite(buzzerPin, HIGH);  // sound
        delay(1);                       // delay1ms
        digitalWrite(buzzerPin, LOW);   // not sound
        delay(1);                       // ms delay
      }
      for (i = 0; i < 100; i++)  // output a frequency sound
      {
        digitalWrite(buzzerPin, HIGH);  // sound
        digitalWrite(buzzerPin, LOW);   // not sound
        delay(2);                       // 2ms delay
      }
    } else {
      digitalWrite(buzzerPin, LOW);
    }
  }
}

void taskUltrasonicMqtt(void* pvParameters) {
  int distance;
  char str[16];
  while (1) {
    // leo el valor del sensor ultrasonidos
    digitalWrite(outputPin, LOW);
    delayMicroseconds(2);
    digitalWrite(outputPin, HIGH);  // Pulse for 10μ s to trigger ultrasonic detection
    delayMicroseconds(10);
    digitalWrite(outputPin, LOW);
    distance = pulseIn(inputPin, HIGH);                 // Read receiver pulse time
    distance = distance / 58;                           // Transform pulse time to distance
    Serial.print("Publicando distancia: ");
    Serial.println(distance);  // Output distance

    // publico en mqtt
    sprintf(str, "%u", distance);
    client.publish("devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/carDistance", str);  // Usar o mesmo topic que en Node-RED

    delay(PUB_DELAY);
  }
}


void setup() {
  // Configuracion pin buzzer
  pinMode(buzzerPin, OUTPUT);  // set digital IO pin pattern, OUTPUT tobe output

  // COnfiguracion pines ultrasonido
  pinMode(inputPin, INPUT);
  pinMode(outputPin, OUTPUT);

  // Configuración do porto serie
  Serial.begin(115200);

  // Conexión coa WiFi
  setup_wifi();

  // Configuración de MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // se crea tarea que lee valor de disancia y lo publica en mqtt
  if (xTaskCreate(taskBuzzer, "Buzzer task", 10000, NULL, tskIDLE_PRIORITY + 1, &xTaskBuzzer) != pdPASS) {
    Serial.println("Error al crear tarea taskBuzzer");
  }
  // se crea tarea que maneja el buzzer
  if (xTaskCreate(taskUltrasonicMqtt, "Ultrasonidos task", 10000, NULL, tskIDLE_PRIORITY + 1, &xTaskUltrasonicMqtt) != pdPASS) {
    Serial.println("Error al crear tarea taskUltrasonicMqtt");
  }
}

void loop() {
  // Verifica se o cliente está conectado
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}