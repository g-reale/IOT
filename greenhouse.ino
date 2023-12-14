#include "Logger.hpp"
#include "Samplers.hpp"
#include <WiFi.h>
#include <PubSubClient.h>

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
  LOCK,
  LOCK_1,
  ON
};

const char *SSID = SSID;
const char *PWD = PWD;

const char *SERVER = "test.mosquitto.org";
const int PORT = 1883;

WiFiClient wifi;
PubSubClient mqttClient(wifi);

uint8_t id;
uint8_t event;
uint8_t state = OFF;

bool red = false;
bool yellow = false;
bool green = false;
bool rain = false;
bool notRain = false;

String message;
String sensor;

Logger temp(sampleBmpTemp, NC, 10000, TMP);
Logger press(sampleBmpPress, NC, 10000, PRE);
Logger alt(sampleBmpAlt, NC, 10000, ALT);
Logger hum(sampleHumidity, 13, 10000, HUM);
Logger lum(sampleLuminosity, 12, 10000, LUM);
Logger ran(sampleRain, 19, 10000, RAN);

Logger *sensors[] = {&temp, &press, &alt, &hum, &lum, &ran};

void CallbackMqtt(char *topic, byte *payload, unsigned int length);
void handleManualControl();
void handleAutomaticControl();
void connectToWiFi();
void connectToMQTTBroker();
void setupLEDs();
void publishLEDStatus(bool red, bool yellow, bool green);
void updateAndPublishAllSensors();

void setup()
{
  Serial.begin(115200);
  connectToWiFi();
  connectToMQTTBroker();
  setupLEDs();
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
    publishLEDStatus(red, yellow, green);
  }
  delay(1000);
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

void setupLEDs()
{
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
}

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

void updateAndPublishAllSensors()
{
  for (Logger *sensor : sensors)
  {
    if (event = sensor->logAll(id))
    {
      String sensorName = String(PATH) + String(sensor->getName(id));

      rain = (strcmp(event_names[event], "RAINING") == 0);
      notRain = (strcmp(event_names[event], "NOT_RAINING") == 0);

      if (rain)
        message = String("CHOVENDO");
      else if (notRain)
        message = String("SEM CHUVA");
      else
        message = String(sensor->getData(id));

      mqttClient.publish(sensorName.c_str(), message.c_str());
    }
  }
}

void handleManualControl()
{
  Serial.println("MANUAL");
  updateAndPublishAllSensors();

  mqttClient.publish((String(PATH) + String("REDLED")).c_str(), String(0).c_str());
  mqttClient.publish((String(PATH) + String("YELLOWLED")).c_str(), String(0).c_str());
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);

  if (MANUAL_IRRIGATION)
  {
    mqttClient.publish((String(PATH) + String("REDLED")).c_str(), String(1).c_str());
    digitalWrite(RED_LED_PIN, HIGH);
    delay(1500);
    mqttClient.publish((String(PATH) + String("REDLED")).c_str(), String(0).c_str());
    digitalWrite(RED_LED_PIN, LOW);
    MANUAL_IRRIGATION = false;
  }

  if (MANUAL_COVER)
  {
    mqttClient.publish((String(PATH) + String("GREENLED")).c_str(), String(1).c_str());
    digitalWrite(GREEN_LED_PIN, HIGH);
  }
  else if (!MANUAL_COVER)
  {
    mqttClient.publish((String(PATH) + String("GREENLED")).c_str(), String(0).c_str());
    digitalWrite(GREEN_LED_PIN, LOW);
  }

  if (MANUAL_COLD || MANUAL_WARM)
  {
    mqttClient.publish((String(PATH) + String("YELLOWLED")).c_str(), String(1).c_str());
    digitalWrite(YELLOW_LED_PIN, HIGH);
    delay(1500);
    mqttClient.publish((String(PATH) + String("YELLOWLED")).c_str(), String(0).c_str());
    digitalWrite(YELLOW_LED_PIN, LOW);
    MANUAL_COLD = false;
    MANUAL_WARM = false;
  }
}

void handleAutomaticControl()
{
  Serial.println("AUTOMATICO");
  updateAndPublishAllSensors();

  switch (state)
  {
  case OFF:
    red = false;
    digitalWrite(RED_LED_PIN, LOW);
    state = event == LOW_HUM ? ON : state;
    state = event == LOW_PRESS ? LOCK : state;
    state = event == RAINING ? LOCK_1 : state;
    break;

  case LOCK:
    state = event == HIGH_PRESS ? OFF : state;
    break;

  case LOCK_1:
    state = event == NOT_RAINING ? OFF : state;
    break;

  case ON:
    red = true;
    digitalWrite(RED_LED_PIN, HIGH);
    state = event == HIGH_HUM ? OFF : state;
    state = event == LOW_PRESS ? OFF : state;
    state = event == RAINING ? OFF : state;
    break;
  }

  switch (event)
  {

  case EVENT_ENVIRONMENT_CONTROL_ON:
    yellow = true;
    digitalWrite(YELLOW_LED_PIN, HIGH);
    break;

  case EVENT_ENVIRONMENT_CONTROL_OFF:
    yellow = false;
    digitalWrite(YELLOW_LED_PIN, LOW);
    break;

  case EVENT_AUTOMATIC_COVER_ON:
    green = true;
    digitalWrite(GREEN_LED_PIN, HIGH);
    break;

  case EVENT_AUTOMATIC_COVER_OFF:
    green = false;
    digitalWrite(GREEN_LED_PIN, LOW);
    break;
  }

  publishLEDStatus(red, yellow, green);
}

void publishLEDStatus(bool red, bool yellow, bool green)
{
  mqttClient.publish((String(PATH) + String("REDLED")).c_str(), String(red).c_str());
  mqttClient.publish((String(PATH) + String("YELLOWLED")).c_str(), String(yellow).c_str());
  mqttClient.publish((String(PATH) + String("GREENLED")).c_str(), String(green).c_str());

  Serial.print("RED_LED_PIN");
  Serial.print("\t");
  Serial.println(red);

  Serial.print("YELLOW_LED_PIN");
  Serial.print("\t");
  Serial.println(yellow);

  Serial.print("GREEN_LED_PIN");
  Serial.print("\t");
  Serial.println(green);

  Serial.println("-------------------------------------\n\n");
  delay(1000);
}
