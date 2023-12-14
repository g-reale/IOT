#include "Logger.hpp"
#include "Samplers.hpp"
#include <WiFi.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>

// Buttons
bool SYSTEM_ON = false;
bool MANUALLY = false;
bool MANUAL_IRRIGATION = false;
bool MANUAL_COVER = false;
bool MANUAL_COLD = false;
bool MANUAL_WARM = false;

// LEDs
const int RED_LED_PIN = 14;
const int YELLOW_LED_PIN = 27;
const int GREEN_LED_PIN = 26;

// Aliases
#define TMP "TMP"
#define PRE "PRE"
#define ALT "ALT"
#define HUM "HUM"
#define LUM "LUM"
#define RAN "RAN"

const char *PATH = "IOT/okidoki/";

// States
enum States
{
  OFF = 0,
  LOCK = 1,
  LOCK_1 = 2,
  ON = 3
};

const char *SSID = "FERNANDA";
const char *PWD = "17071996";

const char *SERVER = "test.mosquitto.org";
const int PORT = 1883;

WiFiClient wifi;
PubSubClient mqttClient(wifi);

static bool red = false;
static bool yellow = false;
static bool green = false;
static bool rain = false;
static bool notRain = false;
static bool firstM = true;

String message;
String sensor;

Logger temp(sampleBmpTemp, NC, 10000, TMP);
Logger press(sampleBmpPress, NC, 10000, PRE);
Logger alt(sampleBmpAlt, NC, 10000, ALT);
Logger hum(sampleHumidity, 13, 10000, HUM);
Logger lum(sampleLuminosity, 12, 10000, LUM);
Logger ran(sampleRain, 19, 10000, RAN);

void CallbackMqtt(char *topic, byte *payload, unsigned int length);
void handleManualControl();
void handleAutomaticControl();
void connectToWiFi();
void connectToMQTTBroker();
// void setupLEDs();
void publishLEDStatus(bool red, bool yellow, bool green, bool delayOn);
uint8_t updateAndPublishAllSensors();
void sendCommandToArduino(char command);

void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600);
  connectToWiFi();
  connectToMQTTBroker();
  // setupLEDs();
}

void sendCommandToArduino(char command) {
  Serial.println(">>>>>>>>> Enviando: ");
  Serial.println(command);
  Serial2.write(command);
}

void loop()
{
  mqttClient.loop();

  if (SYSTEM_ON)
  {
    mqttClient.publish((String(PATH) + String("STATUS")).c_str(), String("sistemas ligados").c_str());
    
    if (MANUALLY)
      handleManualControl();
    else
      handleAutomaticControl();
  }
  else
  {
    Serial.println("O sistema está desligado.");
    mqttClient.publish((String(PATH) + String("STATUS")).c_str(), String("sistemas desligados").c_str());
    red = false;
    yellow = false;
    green = false;
    publishLEDStatus(red, yellow, green, false);
  }
  // delay(1000);
}

void connectToWiFi()
{
  Serial.print("Connecting to WiFi ");
  WiFi.begin(SSID, PWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.print("\nConnected to ");
  Serial.println(SSID);
}

void connectToMQTTBroker()
{
  mqttClient.setServer(SERVER, PORT);

  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected())
  {
    char clientId[100] = "\0";
    sprintf(clientId, "ESP32Client-%04X", random(0xffff));
    Serial.println(clientId);
    if (mqttClient.connect(clientId))
    {
      Serial.println("Connected to MQTT broker.");
      mqttClient.subscribe((String(PATH) + String("ON")).c_str());
      mqttClient.subscribe((String(PATH) + String("MANUALLY")).c_str());
      mqttClient.subscribe((String(PATH) + String("IRRIGATION")).c_str());
      mqttClient.subscribe((String(PATH) + String("COVER")).c_str());
      mqttClient.subscribe((String(PATH) + String("ENVIRONMENTCOLD")).c_str());
      mqttClient.subscribe((String(PATH) + String("ENVIRONMENTWARM")).c_str());
      mqttClient.setCallback(CallbackMqtt);
    }
  }
}

// void setupLEDs()
// {
//   pinMode(RED_LED_PIN, OUTPUT);
//   pinMode(YELLOW_LED_PIN, OUTPUT);
//   pinMode(GREEN_LED_PIN, OUTPUT);

//   digitalWrite(RED_LED_PIN, LOW);
//   digitalWrite(YELLOW_LED_PIN, LOW);
//   digitalWrite(GREEN_LED_PIN, LOW);
// }

void CallbackMqtt(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Payload: ");
  String receivedPayload;
  for (int i = 0; i < length; i++)
  {
    receivedPayload += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (receivedPayload.equals("LIGAR"))
    SYSTEM_ON = true;
  else if (receivedPayload.equals("DESLIGAR"))
    SYSTEM_ON = false;
  else if (receivedPayload.equals("MANUAL"))
    MANUALLY = true;
  else if (receivedPayload.equals("AUTOMATICO"))
    MANUALLY = false;
  else if (receivedPayload.equals("ATIVAR_COBERTAS"))
    MANUAL_COVER = true;
  else if (receivedPayload.equals("DESATIVAR_COBERTAS"))
    MANUAL_COVER = false;
  else if (receivedPayload.equals("IRRIGAR"))
    MANUAL_IRRIGATION = true;
  else if (receivedPayload.equals("AQUECER"))
    MANUAL_WARM = true;
  else if (receivedPayload.equals("ESFRIAR"))
    MANUAL_COLD = true;
}

uint8_t updateAndPublishAllSensors()
{
  uint8_t id;
  uint8_t event;

  Serial.print("\t\t > ID: ");
  Serial.println(id);

  if(event = Logger::logAll(id)){
    sensor = String(PATH) + String(Logger::getName(id));

    if (strcmp(event_names[event], "RAINING") == 0) {
      rain = true;
      notRain = false;
      message = "CHOVENDO";
    } else if (strcmp(event_names[event], "NOT_RAINING") == 0) {
      rain = false;
      notRain = true;
      message = "SEM CHUVA";
    } else {
      message = String(Logger::getData(id));
    }

    Serial.print("\t\t > Publicacao: Sensor: ");
    Serial.print(sensor);
    Serial.print(" - Mensagem: ");
    Serial.println(message);

    mqttClient.publish(sensor.c_str(), message.c_str());

    return event;
  }
}

void handleManualControl() {
  Serial.println("MANUAL");

  // Turn off all LEDs
  if (firstM) {
    firstM = false;
    red = false;
    yellow = false;
    green = false;
    publishLEDStatus(red, yellow, green, false);
  }

  // Update sensor values and publish them
  uint8_t event = updateAndPublishAllSensors();

  if (MANUAL_IRRIGATION) {
    // Turn on the red LED for irrigation
    red = true;
    publishLEDStatus(red, yellow, green, true);
    // Turn off the red LED
    red = false;
    publishLEDStatus(red, yellow, green, false);
    MANUAL_IRRIGATION = false;
  }

  if (MANUAL_COVER) {
    // Turn on the green LED for cover
    green = true;
    publishLEDStatus(red, yellow, green, false);
  } else {
    // Turn off the green LED if manual cover is not active
    green = false;
    publishLEDStatus(red, yellow, green, false);
  }

  if (MANUAL_COLD || MANUAL_WARM) {
    // Turn on the yellow LED for cold or warm
    yellow = true;
    publishLEDStatus(red, yellow, green, true);
    // Turn off the yellow LED
    yellow = false;
    publishLEDStatus(red, yellow, green, false);
    MANUAL_COLD = false;
    MANUAL_WARM = false;
  }
  delay(1000);
}


void handleAutomaticControl()
{
  Serial.println("AUTOMATICO");

  static uint8_t state = OFF;
  
  firstM = true; // variable to reset the LEDs when manual control is activated
  
  uint8_t event = updateAndPublishAllSensors();

  switch (state)
  {
  case OFF:
    red = false;
    // digitalWrite(RED_LED_PIN, LOW);
    sendCommandToArduino('B');
    state = (event == LOW_PRESS) ? LOCK : rain ? LOCK_1 : (event == LOW_HUM) ? ON : state;
    break;

  case LOCK:
    state = event == HIGH_PRESS ? OFF : state;
    break;

  case LOCK_1:
    state = notRain ? OFF : state;
    break;

  case ON:
    red = true;
    // digitalWrite(RED_LED_PIN, HIGH);
    sendCommandToArduino('A');
    state = (event == HIGH_HUM || event == LOW_PRESS || event == RAINING) ? OFF : state;
    break;
  }

  switch (event)
  {

  case EVENT_ENVIRONMENT_CONTROL_ON:
    yellow = true;
    // digitalWrite(YELLOW_LED_PIN, HIGH);
    sendCommandToArduino('C');
    break;

  case EVENT_ENVIRONMENT_CONTROL_OFF:
    yellow = false;
    digitalWrite(YELLOW_LED_PIN, LOW);
    sendCommandToArduino('D');
    break;

  case EVENT_AUTOMATIC_COVER_ON:
    green = true;
    digitalWrite(GREEN_LED_PIN, HIGH);
    sendCommandToArduino('E');
    break;

  case EVENT_AUTOMATIC_COVER_OFF:
    green = false;
    digitalWrite(GREEN_LED_PIN, LOW);
    sendCommandToArduino('F');
    break;
  }

  publishLEDStatus(red, yellow, green, true);
}

void publishLEDStatus(bool red, bool yellow, bool green, bool delayOn)
{
  mqttClient.publish((String(PATH) + String("REDLED")).c_str(), String(red).c_str());
  mqttClient.publish((String(PATH) + String("YELLOWLED")).c_str(), String(yellow).c_str());
  mqttClient.publish((String(PATH) + String("GREENLED")).c_str(), String(green).c_str());

  if (delayOn)
    delay(1000);
}
