#include "Logger.hpp"
#include "Samplers.hpp"

// LEDs
#define RED_LED_PIN 8 
#define YELLOW_LED_PIN 9
#define GREEN_LED_PIN 10

// Events
#define EVENT_IRRIGATION_ON 2
#define EVENT_IRRIGATION_OFF 3
#define EVENT_ENVIRONMENT_CONTROL_ON 4
#define EVENT_ENVIRONMENT_CONTROL_OFF 5
#define EVENT_AUTOMATIC_COVER_ON 6
#define EVENT_AUTOMATIC_COVER_OFF 7

void setup() {
  Serial.begin(9600);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
}

Logger temp(sampleBmpTemp,NC,1000);
Logger press(sampleBmpPress,NC,1000);
Logger alt(sampleBmpAlt,NC,1000);
Logger hum(sampleHumidity,13,1000);
Logger lum(sampleLuminosity,7,1000);

char names[10][10] = {
  "\ttmp",
  "\tpre",
  "\talt",
  "\thum",
  "\tlum"
};

void loop() {
  uint8_t id;
  uint8_t event;
  if(event == Logger::logAll(id)){
    Serial.print("event: ");
    Serial.print((int)event);
    Serial.print("\tid: ");
    Serial.print((int)id);
    Serial.print("\tdata: ");
    Serial.println(Logger::getData(id));

    // Executes action corresponding to the event
    switch (event) {
      case EVENT_IRRIGATION_ON:
        // Activate automatic irrigation system
        digitalWrite(RED_LED_PIN, HIGH);
        break;
      
      case EVENT_IRRIGATION_OFF:
        // Deactivate automatic irrigation system
        digitalWrite(RED_LED_PIN, LOW);
        break;

      case EVENT_ENVIRONMENT_CONTROL_ON:
        // Activate environment control system
        digitalWrite(YELLOW_LED_PIN, HIGH);
        break;
      
      case EVENT_ENVIRONMENT_CONTROL_OFF:
        // Deactivate environment control system
        digitalWrite(YELLOW_LED_PIN, LOW);
        break;

      case EVENT_AUTOMATIC_COVER_ON:
        // Activate automatic coverage system
        digitalWrite(GREEN_LED_PIN, HIGH);
        break;
      
      case EVENT_AUTOMATIC_COVER_OFF:
        // Deactivate automatic coverage system
        digitalWrite(GREEN_LED_PIN, LOW);
        break;

      default:
      // Disables all systems
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, LOW);
        break;
    }

    Serial.println(names[id]);
  }
}
