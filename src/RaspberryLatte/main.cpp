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

  RaspLatte::PID::PIDGains K = {.p = 15, .i = 0.001, .d = 40};
  RaspLatte::PID ctrl(&tempSensor, K);
  ctrl.setInputLimits(0, 255);
  ctrl.setIntegralSumLimits(-50, 50);
  ctrl.setSlopePeriodSec(1);
  for(int i = 0; i<100000; i++){
    ctrl.update();
  }
    
  return 1;
}
  
