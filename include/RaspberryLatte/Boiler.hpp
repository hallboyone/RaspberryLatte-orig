#ifndef BOILER
#define BOILER

#include <iostream>
#include <pigpio.h>
#include <sstream>

#include "PID.hpp"
#include "Clamp.hpp"
#include "types.h"


namespace RaspLatte{
  
  class Boiler{
  private:
    Sensor<double> * temp_sensor_;
    PinIndex heater_pin_;
    
    TempPair * setpoints_;
    double setpoint_;
    MachineMode * mode_;
    MachineMode previous_mode_;
    
    const PID::PIDGains K_brew_ = {.p = 15, .i = 0.1, .d = 50};
    const PID::PIDGains K_steam_ = {.p = 15, .i = 0.1, .d = 50};
    
    PID ctrl_;

    Clamp<double> setpoint_clamp_;

    unsigned int current_pwm_setting_ = 128;

    void updateMode(){
      if (previous_mode_ == OFF) ctrl_.reset();
      switch(*mode_){
      case STEAM:
	setpoint_ = setpoints_->steam;
	if(previous_mode_ != *mode_){
	  ctrl_.setGains(K_steam_);
	}
	break;
      case BREW:
	setpoint_ = setpoints_->brew;
	if(previous_mode_ != *mode_){
	  ctrl_.setGains(K_brew_);
	}
	break;
      case OFF:
	break;
      }
      previous_mode_ = *mode_;
    }
    
  public:
    Boiler(Sensor<double> * temp_sensor, TempPair * setpoints, MachineMode * mode, PinIndex heater_pin):      
      temp_sensor_(temp_sensor), heater_pin_(heater_pin),
      setpoints_(setpoints), setpoint_(setpoints_->brew), mode_(mode),
      ctrl_(K_brew_, &setpoint_, temp_sensor_), setpoint_clamp_(0,160)
    {
      if (gpioInitialise() < 0) throw "Could not start GPIO!";

      // Set defaults
      ctrl_.setMinUpdateTimeSec(0.20); //Don't update the PID faster than 5Hz
      gpioSetPWMfrequency(heater_pin, 20); //Set the Pwm to operate at 20Hz
      
      //Assume not active until first update called
      previous_mode_ = OFF;
      gpioPWM(heater_pin_, 0);
      current_pwm_setting_ = 0;
    }

    void update(){
      //If machine is off, disable pwm
      if(*mode_ == OFF){ 
	gpioPWM(heater_pin_, 0);
	current_pwm_setting_ = 0;
	previous_mode_ = OFF;
	return;
      }

      //Update internal parameters based on current mode.
      updateMode();
      
      unsigned int pwm_output = ctrl_.update();
      if(pwm_output != current_pwm_setting_){ // Only update PWM setting if value changed.
	gpioPWM(heater_pin_, pwm_output);
	current_pwm_setting_ = pwm_output;
      }
    }

    void updatePIDWin(WINDOW * pid_win){
      ctrl_.updateStatusWin(pid_win);
    }
  };
}
#endif
