// Implementación de cliente MQTT SUBSCRIBIR
#include <WiFi.h>
#include <PubSubClient.h>

// Credenciais da rede Wifi
const char* ssid = "Nina Note 10 Pro"; // Substituir polo nome da nosa rede Wifi
const char* password = "ninasanz0311*"; // Substituir por password da nosa rede Wifi
// Configuración do broker MQTT
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/pedestrian/status";

const char* mqttUser = "";
const char* mqttPassword = "";

const int redpin = 0;           //select the pin for the red LED
const int bluepin = 2;          // select the pin for the blue LED
const int greenpin = 4;         // select the pin for the green LED
const int PinSensorLum = 34;    // lectura led luminosidad

int valueLum = 0;

WiFiClient espClient;
PubSubClient client(espClient);

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

  // iniciar semaforo apagado     
  Serial.println("Semaforo apagado");
  setColor(0, 0, 0);  // semaforo off
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
    
    // Controlamos o estado do LED en fucnión da mensaxe
    if (message == "0") {
      Serial.println("Semaforo Verde");
      setColor(0, 255, 0);  // green color  
    } else if (message == "2") {
      Serial.println("Semaforo Rojo");
      setColor(255, 0, 0);  // red color  
    } else if (message == "3"){  // apagado
      Serial.println("Semaforo apagado");
      setColor(0, 0, 0);  // semaforo off
    }
}

// Función para establecer un color RGB
// como el led RGB es tipo anodo comun, los pines catodos se activan con cero, por eso 255-color
void setColor(int red, int green, int blue) {
  analogWrite(redpin, 255-red);
  analogWrite(greenpin, 255-green);
  analogWrite(bluepin, 255-blue);
}

// Reconecta co broker MQTT se se perde a conexión
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar a broker MQTT...");
    // Inténtase conectar indicando o ID do dispositivo
    //IMPORTANTE: este ID debe ser único!
    if (client.connect("NODO5MUIoT-NAPIoT")) {
      Serial.println("conectado!");
      // Subscripción ao topic
      client.subscribe(mqtt_topic);
      Serial.println("Subscrito a topic");
    } else {
      Serial.print("error en la conexión, erro=");
      Serial.print(client.state());
      Serial.println(" probando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}
void setup() {
  // Configuración do pin do LED
 // pinMode(ledPin, OUTPUT);
 // digitalWrite(ledPin, LOW);
  // Configuración do porto serie
  Serial.begin(115200);
  // Conexión coa WiFi
  setup_wifi();
  // Configuración de MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  }
  void loop() {
  // Verifica se o cliente está conectado
  if (!client.connected()) {
  reconnect();
  }
  client.loop();

  // lectura sensor de luminosidad
  valueLum = analogRead(PinSensorLum);
  Serial.println(valueLum);
  delay(100); //only here to slow down the output so it is easier to read

  client.loop();
  char str[16];
  sprintf(str, "%u", valueLum); //Con esto simulamos a xeración do valor dun sensor
  client.publish("devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/intensity", str); // Usar o mesmo topic que en Node-RED
  Serial.println(str);
  delay(5000);
}