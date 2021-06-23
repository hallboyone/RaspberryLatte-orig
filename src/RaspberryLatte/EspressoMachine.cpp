#include "../../include/RaspberryLatte/EspressoMachine.hpp"

namespace RaspLatte{
  
  bool EspressoMachine::atSetpoint(){
    return ((boiler_temp_sensor_.read() < 1.05*setpoint()) & (boiler_temp_sensor_.read() > .95*setpoint()));
  }

  void EspressoMachine::updateSetpoint(double increment){
    if (current_mode_==BREW) {
      temps_.brew += increment;
      boiler_.updateSetpoint(temps_.brew);
    }
    else if (current_mode_==STEAM) {
      temps_.steam += increment;
      boiler_.updateSetpoint(temps_.steam);
    }
  }

  void EspressoMachine::updateMode(){
    current_mode_ = currentMode();
    switch(current_mode_){
    case STEAM:
      boiler_.updateSetpoint(temps_.steam, &K_.steam);
      boiler_.turnOn();
      break;
    case BREW:
      boiler_.updateSetpoint(temps_.brew, &K_.brew);
      boiler_.turnOn();
      break;
    case OFF:
      boiler_.turnOff();
    }
  }
    
  void EspressoMachine::updateLights(){
    gpioWrite(LIGHT_PIN_PWR, current_mode_ != OFF);
    gpioWrite(LIGHT_PIN_PMP, ((current_mode_ == BREW) & atSetpoint()));
    gpioWrite(LIGHT_PIN_STM, ((current_mode_ == STEAM) & atSetpoint()));
    return;
  }

  /*
  void EspressoMachine::handleKeyPress(int key){
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
  */
  EspressoMachine::EspressoMachine(double brew_temp, double steam_temp):
    temps_{.brew=brew_temp, .steam=steam_temp}, boiler_temp_sensor_(CS_THERMO),
    boiler_(&boiler_temp_sensor_, temps_.brew, &(K_.brew), PWM_BOILER),
    pwr_switch_(SWITCH_PIN_PWR, false, true),
    pump_switch_(SWITCH_PIN_PMP, true), steam_switch_(SWITCH_PIN_STM, true)
  {
    current_mode_ = OFF; // Keep machine off until run() is called
  }

  void EspressoMachine::run(){
    int key_press;
    while(true){
      if (currentMode() != current_mode_) updateMode();
      updateLights();
      if (current_mode_ != OFF){
	if (pump_switch_.read()){
	  boiler_.update(128);
	}
	else {
	  boiler_.update();
	}
      }
    }
  }

  MachineMode EspressoMachine::currentMode(){
    if(pwr_switch_.read()){
      if(steam_switch_.read()){
	return STEAM;
      } else {
	return BREW;
      }
    }
    else {
      return OFF;
    }
  }
  
  bool EspressoMachine::pumpOn() { return pump_switch_.read(); }
  double EspressoMachine::setpoint(){
    if (current_mode_ == STEAM){
      return temps_.steam;
    } else {
      return temps_.brew;
    }
  }
    
  EspressoMachine::~EspressoMachine(){
    gpioWrite(LIGHT_PIN_PWR, 0);
    gpioWrite(LIGHT_PIN_PMP, 0);
    gpioWrite(LIGHT_PIN_STM, 0);
  }
}
