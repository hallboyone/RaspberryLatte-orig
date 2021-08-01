#include "../../include/RaspberryLatte/EspressoMachine.hpp"

#include "../../include/RaspberryLatte/pins.h"
#include "../../include/RaspberryLatte/MAX31855.hpp"
#include <iostream>

int main(void){
  RaspLatte::MAX31855 boiler_thermometer(CS_THERMO);
  RaspLatte::EspressoMachine gaggia_classic(93.5, 157, &boiler_thermometer);
  gaggia_classic.run();
  return 0;
}
  
