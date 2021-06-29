#ifndef CPUTEMP
#define CPUTEMP
#include <stdio.h>

class CPUThermometer{
public:
  float getTemp() const{
    float millideg;
    FILE *thermal;

    thermal = fopen("/sys/class/thermal/thermal_zone0/temp","r");
    fscanf(thermal,"%f",&millideg);
    fclose(thermal);
    return millideg / 1000;
  }
};
#endif
