#ifndef SAMPLERSHPP
#define SAMPLERSHPP

#include "MovingAvg.hpp"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define NC 255 //not connected
#define NTR 0  //nothing to report

Adafruit_BMP280 bmp;

// Events
#define EVENT_IRRIGATION_ON 2
#define EVENT_IRRIGATION_OFF 3
#define EVENT_ENVIRONMENT_CONTROL_ON 4
#define EVENT_ENVIRONMENT_CONTROL_OFF 5
#define EVENT_AUTOMATIC_COVER_ON 6
#define EVENT_AUTOMATIC_COVER_OFF 7
#define NOT_RAINING 8  
#define RAINING 9

// limits
#define TEMPERATURE_HIGH_THRESHOLD 30.0  
#define TEMPERATURE_LOW_THRESHOLD 15.0
#define SOIL_HUMIDITY_LOW_THRESHOLD 380
#define LUMINOSITY_HIGH_THRESHOLD 20.0
#define PRESSURE_LOW_THRESHOLD 1000.0

const char event_names[][30] = {
  "NOTHING_TO_REPORT",
  "-\-",
  "IRRIGATION_ON",
  "IRRIGATION_OFF",
  "ENVIROMENT_CONTROL_ON",
  "ENVIROMENT_CONTROL_OFF",
  "AUTOMATIC_COVER_ON",
  "AUTOMATIC_COVER_OFF",
  "NOT_RAINING",
  "RAINING"
};

uint8_t sampleBmpTemp(float& output, uint8_t pin){
  static MovingAvg avg_temp;
  static int first_call = 1;

  if (!bmp.begin(0x76) && first_call){ 
    return NTR;
  }

  if(first_call)
    first_call = 0;

  output = avg_temp.filter(bmp.readTemperature());

  if (output > TEMPERATURE_HIGH_THRESHOLD) { 
    // Condition 3: If the temperature is too high
    return EVENT_ENVIRONMENT_CONTROL_ON;
  } else if (output < TEMPERATURE_LOW_THRESHOLD) { 
    // Condition 5: If the temperature is too low
    return EVENT_ENVIRONMENT_CONTROL_ON;
  } else {
    // Condition 6: If the temperature is at a level considered normal
    return EVENT_ENVIRONMENT_CONTROL_OFF;
  }
}

uint8_t sampleBmpPress(float& output, uint8_t pin){
  static MovingAvg avg_temp;
  static int first_call = 1;

  if (!bmp.begin(0x76) && first_call){ 
    return NTR;
  }

  if(first_call)
    first_call = 0;

  output = avg_temp.filter(bmp.readPressure());

  if (output > PRESSURE_LOW_THRESHOLD) { 
    // Condition 2: If atmospheric pressure is significantly lower than average, as this indicates possible imminent rain
    return EVENT_IRRIGATION_OFF;
  }
  
  //event handling
  return NTR;
}

uint8_t sampleBmpAlt(float& output, uint8_t pin){
  static MovingAvg avg_temp;
  static int first_call = 1;

  if (!bmp.begin(0x76) && first_call){ 
    return NTR;
  }

  if(first_call)
    first_call = 0;

  output = avg_temp.filter(bmp.readAltitude(1013.25));

  //event handling
  return NTR;
}

uint8_t sampleHumidity(float& output, uint8_t pin){
  static MovingAvg avg_humidity;
  float sample = 100 * analogRead(pin) / 4096;
  output = avg_humidity.filter(sample);

  if (output < SOIL_HUMIDITY_LOW_THRESHOLD) { 
    // Condition 1: If soil moisture is below the desired level
    return EVENT_IRRIGATION_ON;
  } else {
    // Condition 2: If soil moisture is normal or above the desired level
    return EVENT_IRRIGATION_OFF;
  }
}

uint8_t sampleLuminosity(float& output, uint8_t pin){
  static MovingAvg avg_luminosity;
  float sample = 100 * analogRead(pin) / 4096;
  output = avg_luminosity.filter(sample);

  
  if (output > LUMINOSITY_HIGH_THRESHOLD) { 
    // Condition 4: If the light is too intense
    return EVENT_AUTOMATIC_COVER_ON;
  } else {
    // Condition 8: If the luminosity is not above the upper threshold
    return EVENT_AUTOMATIC_COVER_OFF;
  }
}

uint8_t sampleRain(float& output, uint8_t pin){
  if(digitalRead(pin)){
    output = 0;
    return NOT_RAINING;
  }
  output = 1;
  return RAINING;
}

#endif