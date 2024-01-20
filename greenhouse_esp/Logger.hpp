#ifndef LOGGERHPP
#define LOGGERHPP

/*
  class to manage the sensors
  reads a sensor peridically and registers it's output and events
  
  this is not a dinamic implementation, the number of sensors has to be know at compile time.
  once a sensor is registered it cannot be removed. 
*/

#include <Arduino.h>
#define MAX_SAMPLERS 3

class Logger{
  private:
    
    inline static Logger * loggers[MAX_SAMPLERS];
    inline static uint8_t id_head = 0;
    
    uint8_t id;
    uint8_t pin;
    uint32_t ttl;
    uint32_t timestamp; 
    uint8_t (*sampler)(float&,uint8_t);
    char * name;
    float data;

    
    // runs the sampling function every tts milliseconds
    // saves the result in the data variable and returns a event if it has ocurred 
    inline uint8_t log(){
      if(millis() > timestamp + ttl){
        timestamp = millis();
        return sampler(data,pin); 
      }

      return 0;
    }

  public:

    Logger(uint8_t (*new_sampler)(float&,uint8_t), 
           uint8_t new_pin,
           uint32_t new_ttl,
           char * new_name)
    {
      name = new_name;
      sampler = new_sampler;
      pin = new_pin;
      ttl = new_ttl;
      timestamp = 0;
     
      id = id_head;
      id_head++;

      loggers[id] = this;
      Serial.println((int)ttl);
    }

    inline uint8_t getId(){
      return id;
    }

    inline uint32_t getTtl(){
     return ttl;
    }
  
    // returns the data of the sensor acording to it's id
    static inline float getData(uint8_t id){
      return loggers[id%id_head]->data;
    }

    // returns the human readable name for the sensor
    static inline char * getName(uint8_t id){
      return loggers[id%id_head]->name;
    }

    // returns the number of the event if one has ocurred
    // stores the id of the logger in the input parameter 
    static inline uint8_t logAll(uint8_t& id){
      static uint8_t idx = 0;
      idx = (idx + 1) % id_head;
      id = idx;
      return loggers[idx]->log();
    }
};

#endif