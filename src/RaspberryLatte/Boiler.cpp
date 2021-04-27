#include "../../include/RaspberryLatte/Boiler.hpp"

#include <iostream>
#include <pigpio.h>

namespace RaspLatte{
  void Boiler::updateMode(){
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
    
  Boiler::Boiler(Sensor<double> * temp_sensor, TempPair * setpoints, MachineMode * mode, PinIndex heater_pin):      
    temp_sensor_(temp_sensor), heater_pin_(heater_pin),
    setpoints_(setpoints), setpoint_(setpoints_->brew), mode_(mode),
    ctrl_(K_brew_, &setpoint_, temp_sensor_), setpoint_clamp_(0,160)
  {
    if (gpioInitialise() < 0) throw "Could not start GPIO!";

    // Set defaults
    ctrl_.setMinUpdateTimeSec(0.20); //Don't update the PID faster than 5Hz
    ctrl_.setIntegralSumLimits(0, 100);
    ctrl_.setInputLimits(0, 255);
      
    gpioSetPWMfrequency(heater_pin, 20); //Set the Pwm to operate at 20Hz
      
    //Assume not active until first update called
    previous_mode_ = OFF;
    gpioPWM(heater_pin_, 0);
    current_pwm_setting_ = 0;
  }

  void Boiler::update(int feed_forward){
    //If machine is off, disable pwm
    if(*mode_ == OFF){
      gpioPWM(heater_pin_, 0);
      current_pwm_setting_ = 0;
      previous_mode_ = OFF;
      return;
    }

    //Update internal parameters based on current mode.
    updateMode();
      
    unsigned int pwm_output = ctrl_.update(feed_forward);
    if(pwm_output != current_pwm_setting_){ // Only update PWM setting if value changed.
      gpioPWM(heater_pin_, pwm_output);
      current_pwm_setting_ = pwm_output;
    }
  }

  void Boiler::updatePIDWin(WINDOW * pid_win, bool init){
    ctrl_.updateStatusWin(pid_win, init);
  }

  Boiler::~Boiler(){
    gpioPWM(heater_pin_, 0);
  }
}
