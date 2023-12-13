#include "Logger.hpp"
#include "Samplers.hpp"

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

//states
#define OFF 0
#define LOCK 1
#define LOCK_1 2
#define ON 3

void setup() {
  Serial.begin(115200);
  
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
    Serial.print("event: ");
    Serial.print(event_names[event]);
    Serial.print("\tid: ");
    Serial.print((int)id);
    Serial.print("\tdata: ");
    Serial.print(Logger::getData(id));
    Serial.print("\tname: ");
    Serial.println(Logger::getName(id));

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
