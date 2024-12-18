#include <WiFi.h>
#include <PubSubClient.h>
#include <CustomMatrix.h>

const char* ssid = "";  // Substituír polo SSID da nosa rede WiFi
const char* password = "";   // Substituír polo password da nosa rede WiFi
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
const int buttonPin = 4;

bool countdownCheck = false;

WiFiClient espClient;
PubSubClient client(espClient);

Matrix myMatrix(21, 22);

void setup() {
  Serial.begin(115200);

  while (!Serial) {
  }

  myMatrix.begin(0x70);
  pinMode(buttonPin, INPUT);

  WiFi.begin(ssid, password);
  Serial.println("...................................");
  Serial.print("Conectando á WiFi.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connectado á rede WiFi!");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(onMqttReceived);

  while (!client.connected()) {
    Serial.println("Conectando ao broker MQTT...");
    if (client.connect("nodonapiotmartin", mqttUser, mqttPassword))
      Serial.println("Conectado ao broker MQTT!");
    else {
      Serial.print("Erro ao conectar co broker: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  client.subscribe("devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/countdown/action");
}

void loop() {
  // Verifica se o cliente está conectado
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (countdownCheck) {
    countdown();
    countdownCheck = false;
  }

  if (digitalRead(buttonPin)) {
    char str[16];
    sprintf(str, "%u", "pressed");

    Serial.println("Botón pulsado");
    client.publish("devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/button/action", "press");
  }
  delay(500);
}

// Iniciar contador cando se recibe mensaxe con "start"
void onMqttReceived(char* topic, byte* payload, unsigned int length) {
  String content = "";
  for (size_t i = 0; i < length; i++) {
    content.concat((char)payload[i]);
  }
  if (content == "start") {
    Serial.println("Iniciando contador...");
    countdownCheck = true;
  }
}

// Modificar estado matriz
void changeMatrix(uint8_t valor[8]) {
  for (int i = 0; i < 8; i++) {
    LEDArray[i] = valor[i];
    for (int j = 7; j >= 0; j--) {
      if ((LEDArray[i] & 0x01) > 0)
        myMatrix.drawPixel(j, i, 1);
      LEDArray[i] = LEDArray[i] >> 1;
    }
    myMatrix.writeDisplay();
  }
}

// Mostrar temporizador
void countdown() {
  for (int m = 0; m < 12; m++) {
    myMatrix.clear();
    changeMatrix(estados[m]);
    delay(1000);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar a broker MQTT...");
    if (client.connect("nodonapiotmartin")) {
      Serial.println("conectado!");
      client.subscribe("devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/countdown/action");
      Serial.println("Subscrito ao topic");
    } else {
      Serial.print("erro na conexión, erro=");
      Serial.print(client.state());
      Serial.println(" probando de novo en 5 segundos");
      delay(5000);
    }
  }
}