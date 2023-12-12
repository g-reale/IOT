#include "Logger.hpp"
#include "Samplers.hpp"

// LEDs
#define RED_LED_PIN 14
#define YELLOW_LED_PIN 27
#define GREEN_LED_PIN 26

void setup() {
  Serial.begin(115200);
  
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW); 
}

Logger temp(sampleBmpTemp,NC,5000);
Logger press(sampleBmpPress,NC,5000);
Logger alt(sampleBmpAlt,NC,5000);
Logger hum(sampleHumidity,13,5000);
Logger lum(sampleLuminosity,7,5000);
Logger ran(sampleRain,19,5000);

char names[10][10] = {
  "\ttmp",
  "\tpre",
  "\talt",
  "\thum",
  "\tlum",
  "\tran"
};

void loop() {
  uint8_t id;
  uint8_t event;
  if(event = Logger::logAll(id)){
    Serial.print("event: ");
    Serial.print(event_names[event]);
    Serial.print("\tid: ");
    Serial.print((int)id);
    Serial.print("\tdata: ");
    Serial.print(Logger::getData(id));
    Serial.print("\tname: ");
    Serial.println(names[id]);

  // Executes action corresponding to the event
    switch (event) {
      case EVENT_IRRIGATION_ON:
        //Activate automatic irrigation system
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
  }
}
