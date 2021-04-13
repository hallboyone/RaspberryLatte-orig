#include "../../include/RaspberryLatte/BinarySensor.hpp"
#include "../../include/RaspberryLatte/MAX31855.hpp"
#include "../../include/RaspberryLatte/PID.hpp"
#include <iostream>


int main(void){
  RaspLatte::PinIndex gpio_num = 0;
  bool pull_down = false;
  bool invert = false;
  RaspLatte::BinarySensor sensor1(gpio_num, pull_down, invert);
  RaspLatte::MAX31855 tempSensor(0);

  tempSensor.printError();
  
  double temp = tempSensor.read();
  if(temp == MAX31855_TEMP_UNAVALIBLE){
    tempSensor.printError();
  }
  else{
    std::cout<<"Thermo_temp = "<<tempSensor.read()<<std::endl;;
  }
  
  std::cout<<"Sensor 1: "<<sensor1.read()<<std::endl;

  RaspLatte::PID::PIDGains K = {.p = 1, .i = 0.001, .d = 0};
  RaspLatte::PID ctrl(&tempSensor, K);
  for(int i = 0; i<100; i++){
    ctrl.update();
    std::cout<<ctrl.u()<<std::endl;
  }
    
  return 1;
}
  
