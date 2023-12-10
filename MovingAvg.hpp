#ifndef MOVINGAVGHPP
#define MOVINGAVGHPP

#define ORDER 10

class MovingAvg{
  private:
    float avg = 0;
    float samples[ORDER] = {0,0,0,0,0};
    int index = 0;
  public:
    float filter(float sample){
      sample /= ORDER;
      avg += sample - samples[index];
      samples[index] = sample;
      index = (index + 1) % ORDER;
      return avg;
    }
    
    float filter(int sample){
      float aux = sample;
      return filter(aux);
    }
};

#endif