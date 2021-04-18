#include "../../include/RaspberryLatte/BinarySensor.hpp"
#include "../../include/RaspberryLatte/MAX31855.hpp"
#include "../../include/RaspberryLatte/PID.hpp"
#include "../../include/RaspberryLatte/Boiler.hpp"
#include "../../include/RaspberryLatte/EspressoMachine.hpp"
#include <iostream>


int main(void){
  /*
  RaspLatte::PinIndex gpio_num = 0;
  bool pull_down = false;
  bool invert = false;
  RaspLatte::BinarySensor sensor1(gpio_num, pull_down, invert);
  */
    
  //RaspLatte::MAX31855 boiler_temp_sensor(0);
  //RaspLatte::Sensor<double> * temp_sensor_ptr = &boiler_temp_sensor;
  /*
  double temp = temp_sensor_ptr->read();
  std::cout<<"Thermo_temp = "<<boiler_temp_sensor.read()<<std::endl;;
  temp = temp_sensor_ptr->read();
  std::cout<<"Thermo_temp = "<<temp<<std::endl;;
  temp = temp_sensor_ptr->read();
  std::cout<<"Thermo_temp = "<<temp<<std::endl;;

  
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
  
  */

  RaspLatte::EspressoMachine gaggia_classic(95, 140);
  /*
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  
  WINDOW * win_pid = newwin(20,30,0,0);
  wtimeout(stdscr, 1000);
  while(wgetch(stdscr) != 'q'){
    gaggia_classic.printStatusIn(win_pid);
    //std::cout<<"Thermo_temp = "<<temp_sensor_ptr->read()<<std::endl;;
    //std::cout<<"Thermo_temp = "<<boiler_temp_sensor.read()<<std::endl;;
    
    //gpioSleep(PI_TIME_RELATIVE, 1, 0);
  }
  endwin();
  */
  return 0;
}
  
