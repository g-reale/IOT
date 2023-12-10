#include "Logger.hpp"
#include "Samplers.hpp"

void setup() {
  Serial.begin(9600);
}

Logger temp(sampleBmpTemp,NC,1000);
// Logger press(sampleBmpPress,NC,1000);
// Logger alt(sampleBmpAlt,NC,1000);
Logger hum(sampleHumidity,13,1000);

char names[10][10] = {
  "\ttmp",
  "\tpre",
  "\talt",
  "\thum"
};

void loop() {
  uint8_t id;
  uint8_t event;
  if(event = Logger::logAll(id)){
    Serial.print(Logger::getData(0));
    Serial.print("\t");
    Serial.println(Logger::getData(1));
    // Serial.print("event: ");
    // Serial.print((int)event);
    // Serial.print("\tid: ");
    // Serial.print((int)id);
    // Serial.print("\tdata: ");
    // Serial.println(Logger::getData(id));
    // Serial.println(names[id]);
  }
}
