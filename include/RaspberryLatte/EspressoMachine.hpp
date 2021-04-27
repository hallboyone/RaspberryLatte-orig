#ifndef ESPRESSO_MACHINE
#define ESPRESSO_MACHINE

#include "Boiler.hpp"
#include "BinarySensor.hpp"
#include "MAX31855.hpp"
#include "pins.h"
#include "types.h"
#include "RaspberryLatteUI.hpp"

#include <iostream>

namespace RaspLatte{
  typedef BinarySensor Switch;

  class EspressoMachine{
  private:
    
    TempPair temps_;
    
    MAX31855 boiler_temp_sensor_;
    
    Boiler boiler_;

    RaspberryLatteUI ui_;
    
    Switch pwr_switch_;
    Switch pump_switch_;
    Switch steam_switch_;

    MachineMode current_mode_;
        
    bool atSetpoint(){
      return ((boiler_temp_sensor_.read() < 1.05*setpoint()) & (boiler_temp_sensor_.read() > .95*setpoint()));
    }

    void updateSetpoint(double increment){
      if (current_mode_==BREW) {
	temps_.brew += increment;
      }
      else if (current_mode_==STEAM) {
	temps_.steam += increment;
      }
    }
    
    void updateMode(){
      if(pwr_switch_.read()){
	if(steam_switch_.read()){
	  current_mode_ = STEAM;
	} else {
	  current_mode_ = BREW;
	}
      } else {
	current_mode_ = OFF;
      }
    }
    
    void updateLights(){
      gpioWrite(LIGHT_PIN_PWR, current_mode_ != OFF);
      gpioWrite(LIGHT_PIN_PMP, ((current_mode_ == BREW) & atSetpoint()));
      gpioWrite(LIGHT_PIN_STM, ((current_mode_ == STEAM) & atSetpoint()));
      return;
    }
    
    void handleKeyPress(int key){
      switch(key){
      case KEY_UP:
	updateSetpoint(1);
	break;
      case KEY_DOWN:
	updateSetpoint(-1);
	break;
      case KEY_LEFT:
	updateSetpoint(-0.25);
	break;
      case KEY_RIGHT:
	updateSetpoint(0.25);
	break;
      default:
	break;
      }
    }
    
  public:
    EspressoMachine(double brew_temp, double steam_temp): temps_{.brew=brew_temp, .steam=steam_temp}, boiler_temp_sensor_(CS_THERMO),
							  boiler_(&boiler_temp_sensor_, &temps_, &current_mode_, PWM_BOILER),
							  ui_(this, &boiler_), pwr_switch_(SWITCH_PIN_PWR, false, true),
							  pump_switch_(SWITCH_PIN_PMP, true), steam_switch_(SWITCH_PIN_STM, true){
      current_mode_ = OFF; // Keep machine off until run() is called
    }

    void run(){
      int key_press;
      while((key_press = ui_.refresh()) != 'q'){
	updateMode();
	updateLights();
	if (current_mode_ != OFF){
	  handleKeyPress(key_press);
	  if (pump_switch_.read()){
	    boiler_.update(128);
	  }
	  else {
	    boiler_.update();
	  }
	}
      }
    }

    MachineMode currentMode(){ return current_mode_; }
    bool pumpOn() { return pump_switch_.read(); }
    double setpoint(){
      if (current_mode_ == STEAM){
	return temps_.steam;
      } else {
	return temps_.brew;
      }
    }
    
    ~EspressoMachine(){
      gpioWrite(LIGHT_PIN_PWR, 0);
      gpioWrite(LIGHT_PIN_PMP, 0);
      gpioWrite(LIGHT_PIN_STM, 0);
      endwin();
    }
  };
}

#endif
