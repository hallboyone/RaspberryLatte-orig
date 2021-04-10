#include "BinarySensor.hpp"
#include "MAX31855.hpp"
#include <iostream>


int main(void){
  RaspLatte::PinIndex gpio_num = 0;
  bool pull_down = false;
  bool invert = false;
  RaspLatte::BinarySensor sensor1(gpio_num, pull_down, invert);

  RaspLatte::MAX31855 tempSensor(0);
  std::cout<<"Thermo_temp = "<<tempSensor.read()<<std::endl;;
  //while(1){
    std::cout<<"Sensor 1: "<<sensor1.read()<<std::endl;
    //}
  return 1;
}
  
