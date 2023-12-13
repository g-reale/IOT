
#include "Logger.hpp"
#include "Samplers.hpp"

#include <WiFi.h>
#include <PubSubClient.h>

// LEDs
#define RED_LED_PIN 14
#define YELLOW_LED_PIN 27
#define GREEN_LED_PIN 26

//aliases
#define TMP "TMP"
#define PRE "PRE"
#define ALT "ALT"
#define HUM "HUM"
#define LUM "LUM"
#define RAN "RAN"

#define PATH "okidoki/teamo/shida/"

//states
#define OFF 0
#define LOCK 1
#define LOCK_1 2
#define ON 3

#define SSID "HMOlivei1"
#define PWD "helio001"

#define SERVER "test.mosquitto.org"
#define PORT 1883


WiFiClient wifi;
PubSubClient mqttClient(wifi);

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to WiFi ");
  Serial.println("HMOlivei1");
  WiFi.begin(SSID,PWD);
  
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }

  Serial.print("\nConnected to ");
  Serial.println(SSID);
  
  mqttClient.setServer(SERVER,PORT);

  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
    char clientId[100] = "\0";
    sprintf(clientId, "ESP32Client-%04X", random(0xffff));
    Serial.println(clientId);
    if (mqttClient.connect(clientId)){
      Serial.println("Connected to MQTT broker.");
      mqttClient.subscribe("/okidoki/teamo/shida");
    }
  }

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW); 
}

Logger temp(sampleBmpTemp,NC,10000,TMP);
Logger press(sampleBmpPress,NC,10000,PRE);
Logger alt(sampleBmpAlt,NC,10000,ALT);
Logger hum(sampleHumidity,13,10000,HUM);
Logger lum(sampleLuminosity,12,10000,LUM);
Logger ran(sampleRain,19,10000,RAN);

void loop() {
  uint8_t id;
  uint8_t event;

  static bool red = false;
  static bool yellow = false;
  static bool green = false;

  if(event = Logger::logAll(id)){
    
    String sensor = String(PATH) + String(Logger::getName(id));
    mqttClient.publish(sensor.c_str(), 
                       String(Logger::getData(id)).c_str());
  
    Serial.print("event: ");
    Serial.print(event_names[event]);
    Serial.print("\tid: ");
    Serial.print((int)id);
    Serial.print("\tdata: ");
    Serial.print(Logger::getData(id));
    Serial.print("\tname: ");
    Serial.println(sensor.c_str());

    static uint8_t state = OFF;

    switch (state){
      case OFF:
        red = false;
        digitalWrite(RED_LED_PIN, LOW);

        // Serial.print("RED_LED_PIN");
        // Serial.print("\t");
        // Serial.println(LOW);

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

        // Serial.print("RED_LED_PIN");
        // Serial.print("\t");
        // Serial.println(HIGH);

        state = event == HIGH_HUM ? OFF : state;
        state = event == LOW_PRESS ? OFF : state;
        state = event == RAINING ? OFF : state;
      break;
    }

  // Executes action corresponding to the event
    switch (event) {

      case EVENT_ENVIRONMENT_CONTROL_ON:
        // Activate environment control system
        yellow = true;
        digitalWrite(YELLOW_LED_PIN, HIGH);

        // Serial.print("YELLOW_LED_PIN");
        // Serial.print("\t");
        // Serial.println(HIGH);

        break;
      
      case EVENT_ENVIRONMENT_CONTROL_OFF:
        // Deactivate environment control system
        yellow = false;
        digitalWrite(YELLOW_LED_PIN, LOW);

        // Serial.print("YELLOW_LED_PIN");
        // Serial.print("\t");
        // Serial.println(LOW);

        break;

      case EVENT_AUTOMATIC_COVER_ON:
        // Activate automatic coverage system
        green = true;
        digitalWrite(GREEN_LED_PIN, HIGH);

        // Serial.print("GREEN_LED_PIN");
        // Serial.print("\t");
        // Serial.println(HIGH);

        break;
      
      case EVENT_AUTOMATIC_COVER_OFF:
        // Deactivate automatic coverage system
        green = false;
        digitalWrite(GREEN_LED_PIN, LOW);

        // Serial.print("GREEN_LED_PIN");
        // Serial.print("\t");
        // Serial.println(LOW);

        break;
    }
    
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
  }
}
