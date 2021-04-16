#ifndef ESPRESSO_MACHINE
#define ESPRESSO_MACHINE

#include "Boiler.hpp"
#include "BinarySensor.hpp"
#include "MAX31855.hpp"
#include "pins.h"
#include "types.h"

#include <iostream>

namespace RaspLatte{
  typedef BinarySensor Switch;
 
  class EspressoMachine{
  private:
    TempPair temps_;
    
    MAX31855 boiler_temp_sensor_;
    
    Boiler boiler_;

    Switch pwr_switch_;
    Switch pump_switch_;
    Switch steam_switch_;

    void printStatus(){
      std::cout<<"Power: ";
      printOnOff(pwr_switch_.read());
      std::cout<<", Pump: ";
      printOnOff(pump_switch_.read());
      std::cout<<", Steam: ";
      printOnOff(steam_switch_.read());
      std::cout<<"\nCurrent Temp: "<<boiler_temp_sensor_.read()<<"C\n";
    }
    void printOnOff(bool on){
      if (on) std::cout<<"ON";
      else std::cout<<"OFF";
    }

    static void lightSwitch(int gpio, int level, uint32_t tick){
      if (gpio == SWITCH_PIN_PWR) gpioWrite(LIGHT_PIN_PWR, level);
      else if (gpio == SWITCH_PIN_PMP) gpioWrite(LIGHT_PIN_PMP, (!level) & gpioRead(SWITCH_PIN_PWR));
      else if (gpio == SWITCH_PIN_STM) gpioWrite(LIGHT_PIN_STM, (!level) & gpioRead(SWITCH_PIN_PWR));
      return;
    }
    
    void test(int gpio, int level, uint32_t tick){
      std::cout<<"Changed state"<<std::endl;
    }
    
  public:
    EspressoMachine(double brew_temp, double steam_temp): temps_{.brew=brew_temp, .steam=steam_temp}, boiler_temp_sensor_(CS_THERMO),
							  boiler_(&boiler_temp_sensor_, PWM_BOILER, temps_), pwr_switch_(SWITCH_PIN_PWR, false, true),
							  pump_switch_(SWITCH_PIN_PMP, true), steam_switch_(SWITCH_PIN_STM, true){
      std::cout<<"Starting up the espresso machine!\n";
      printStatus();
      gpioSetAlertFunc(SWITCH_PIN_PWR, lightSwitch);
      gpioSetAlertFunc(SWITCH_PIN_PMP, lightSwitch);
      gpioSetAlertFunc(SWITCH_PIN_STM, lightSwitch);
      gpioWrite(LIGHT_PIN_PWR, pwr_switch_.read());
      gpioWrite(LIGHT_PIN_PMP, pump_switch_.read());
      gpioWrite(LIGHT_PIN_STM, steam_switch_.read());
    }
  };
}

#endif
