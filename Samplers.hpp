#ifndef SAMPLERSHPP
#define SAMPLERSHPP

#include "MovingAvg.hpp"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define NC 255 //not connected
#define NTR 0  //nothing to report

Adafruit_BMP280 bmp;

uint8_t sampleBmpTemp(float& output, uint8_t pin){
  static MovingAvg avg_temp;
  static int first_call = 1;

  if (!bmp.begin(0x76) && first_call){ 
    return NTR;
  }

  if(first_call)
    first_call = 0;

  output = avg_temp.filter(bmp.readTemperature());

  //event handling
  return NTR;
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
  
  //event handling
  return 1;
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
  return 1;
}

uint8_t sampleHumidity(float& output, uint8_t pin){
  static MovingAvg avg_humidity;
  float sample = 100 * analogRead(pin) / 4096;
  output = avg_humidity.filter(sample);

  //event handling
  return 1;
}

uint8_t sampleLuminosity(float& output, uint8_t pin){
  static MovingAvg avg_luminosity;
  float sample = 100 * analogRead(pin) / 4096;
  output = avg_luminosity.filter(sample);

  //event handling
  return 1;
}

#endif