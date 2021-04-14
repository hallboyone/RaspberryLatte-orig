#include "../../include/RaspberryLatte/BinarySensor.hpp"
#include "../../include/RaspberryLatte/MAX31855.hpp"
#include "../../include/RaspberryLatte/PID.hpp"
#include "../../include/RaspberryLatte/Boiler.hpp"
#include <iostream>


int main(void){
  RaspLatte::PinIndex gpio_num = 0;
  bool pull_down = false;
  bool invert = false;
  RaspLatte::BinarySensor sensor1(gpio_num, pull_down, invert);

  RaspLatte::MAX31855 boiler_temp_sensor(0);
  
  RaspLatte::Boiler boiler1(&boiler_temp_sensor, 4, 95, 140);
  boiler1.activateBrew();
  
  double temp = boiler_temp_sensor.read();
  if(temp == MAX31855_TEMP_UNAVALIBLE){
    boiler_temp_sensor.printError();
  }
  else{
    std::cout<<"Thermo_temp = "<<boiler_temp_sensor.read()<<std::endl;;
  }

  while(1){
    boiler1.update();
  }
    
  return 1;
}
  
