// LIBRERIAS SENSORES
#include <Wire.h>
#include <RTClib.h>
// LIBRERIAS COMUNICACIONES
#include <WiFi.h>
#include <PubSubClient.h>

// CONSTANTES
#define DAY_NIGHT_INTERRUPTION 16
#define GREEN_LED 32
#define YELLOW_LED 25
#define RED_LED 27
#define CAR_STATUS_MQTT_TOPIC "devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/car/status"
#define DAY_NIGTH_MQTT_TOPIC "devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/day"
#define BROKER_FOG "172.16.10.133"
#define CLOUDLET_FOG "172.16.10.112"

const char* ssid = "nome_da_rede";
const char* password = "contrasinal_da_rede";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";

// VARIABLES 
WiFiClient espClient;
PubSubClient client(espClient); 
char* mqttServer = "172.16.10.133";

TaskHandle_t xTaskKeepClientConnected = NULL;
TaskHandle_t xTaskManageDayNight = NULL;
bool dia = true;

RTC_DS3231 rtc;

void onAlarm() {    
  dia = !dia;
}

// FUNCIONES
void TaskManageDayNightCode( void * pvParameters ) {
  while (1) {
    if (!dia) {
      client.publish(DAY_NIGTH_MQTT_TOPIC, "0");
      Serial.println("Es de noche. Me mantengo parpadeando en ambar.");
      rtc.clearAlarm(1);
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      // La alarma 1 se repite cada dia, cuando las horas marquen 9
      if(!rtc.setAlarm1( DateTime(0, 0, 0, 7, 0, 0), DS3231_A1_Hour )) {
          Serial.println("Alarma 1 no configurada");
      } else {
          Serial.println("Alarma 1 configurada correctamente");
      }
    }
    while (!dia) {  // Modo noche
      digitalWrite(YELLOW_LED, HIGH);
      delay(500);
      digitalWrite(YELLOW_LED, LOW);
      delay(500);
    }

    if (dia) {
      rtc.clearAlarm(1);
      // La alarma 1 se repite cada dia, cuando las horas marquen 9
      if(!rtc.setAlarm1( DateTime(0, 0, 0, 19, 0, 0), DS3231_A1_Hour )) {
          Serial.println("Alarma 1 no configurada");
      }else {
          Serial.println("Alarma 1 configurada correctamente");
      }
    }

    client.publish(DAY_NIGTH_MQTT_TOPIC, "1");
    Serial.println("Es de dia. Esperando mensaje MQTT...");
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    
    while (dia) {  // Modo dia
      delay(1000);      
    }
  }
}

void TaskKeepClientConnectedCode ( void * pvParameters ) {
  while (1){
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    delay(10000);
  }
}

void setNetwork() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
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

    if (client.connect("PabloNapiotPrueb2", mqttUser, mqttPassword )) {
      Serial.println("Conectado ao broker MQTT!");
    }
    else {
      Serial.print("Erro ao conectar co broker: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.subscribe("devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/car/status");
  Serial.println("Red lista.");
}

void setRTC() {
  while(!rtc.begin()) {
    Serial.println("Couldn't find DS3231!");
    delay(1000);
  }
  Serial.println("Conectado RTC");
  if(rtc.lostPower()) {
      // this will adjust to the date and time at compilation
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //we don't need the 32K Pin, so disable it
  rtc.disable32K();
  
  // Making it so, that the alarm will trigger an interrupt
  pinMode(DAY_NIGHT_INTERRUPTION, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(DAY_NIGHT_INTERRUPTION),
                                        onAlarm, FALLING);

  // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
  // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);
  rtc.disableAlarm(2);

  // stop oscillating signals at SQW Pin
  // otherwise setAlarm1 will fail
  rtc.writeSqwPinMode(DS3231_OFF);

  dia = (rtc.now().hour() > 6 || rtc.now().hour() < 19);

}

// Función de callback que procesa as mensaxes MQTT recibidas 
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("received message[");
  Serial.print(topic);
  Serial.print("] ");

// // TODO: RETURN CUANDO NOCHE
  if (!dia) {
    return;
  }

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
}

// Reconecta co broker MQTT se se perde a conexión 
void reconnect() {   
  while (!client.connected()) {     
    Serial.print("Intentando conectar a broker MQTT...");          // Inténtase conectar indicando o ID do dispositivo     
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback); 
    //IMPORTANTE: este ID debe ser único!     
    if (client.connect("PabloNapiotPrueba2", mqttUser, mqttPassword)) {       
      Serial.println("conectado!");              // Subscripción ao topic       
      Serial.println("Subscrito ao topic");
    } else { 
      Serial.print("erro na conexión, erro=");       
      Serial.print(client.state());       
      Serial.println(" probando de novo en 5 segundos");       
      delay(5000);     
    }   
    client.subscribe("devices/es/udc/MUIoT-NAPIoT/SmartTrafficLight/car/status");
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
  setRTC();
  
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  
  if (xTaskCreate(TaskManageDayNightCode,
                  "MQTT Car",
                  10000,
                  NULL,
                  1,
                  &xTaskManageDayNight)
      != pdPASS) {
    Serial.println("Error al crear tarea Check.");
  }
  if (xTaskCreate(TaskKeepClientConnectedCode,
                  "Reconnect MQTT",
                  10000,
                  NULL,
                  1,
                  &xTaskKeepClientConnected)
      != pdPASS) {
    Serial.println("Error al crear tarea Check.");
  }
}

void loop() {
  
}