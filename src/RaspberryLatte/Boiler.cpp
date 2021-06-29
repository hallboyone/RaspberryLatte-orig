#include "../../include/RaspberryLatte/Boiler.hpp"

#include <iostream>
#include <pigpio.h>

namespace RaspLatte{  
  Boiler::Boiler(Sensor<double> * temp_sensor, double setpoint, const PID::PIDGains * pid_gains, PinIndex heater_pin_idx,
		 double min_setpoint, double max_setpoint):
    temp_sensor_(temp_sensor), setpoint_(setpoint), ctrl_(*pid_gains, &setpoint_, temp_sensor_), heater_pin_(heater_pin_idx),
    active_(false), setpoint_clamp_(min_setpoint, max_setpoint){
    if (gpioInitialise() < 0) throw "Could not start GPIO!";

    // Set defaults
    ctrl_.setMinUpdateTimeSec(0.20); //Don't update the PID faster than 5Hz
    ctrl_.setIntegralSumLimits(0, 100);
    ctrl_.setInputLimits(0, 255);
    ctrl_.setSlopePeriodSec(1.1);

    gpioSetPWMfrequency(heater_pin_, 20); //Set the Pwm to operate at 20Hz
    
    //Assume not active until first update called
    gpioPWM(heater_pin_, 0);
    current_pwm_setting_ = 0;
  }

  void Boiler::turnOn(){
    active_ = true;
    ctrl_.reset();
    update();
  }

  void Boiler::turnOff(){
    active_ = false;
    gpioPWM(heater_pin_, 0);
    current_pwm_setting_ = 0;
  }
  
  double Boiler::updateSetpoint(double setpoint, const PID::PIDGains * gains){
    if (gains != NULL) ctrl_.setGains(*gains);
    setpoint_clamp_.clamp(setpoint);
    setpoint_ = setpoint;
    return setpoint;
  }
  
  void Boiler::update(int feed_forward){
    //If machine is on, get input and apply to heater
    if(active_){
      unsigned int pwm_output = 0;//ctrl_.update(feed_forward);
      if(pwm_output != current_pwm_setting_){ // Only update PWM setting if value changed.
	gpioPWM(heater_pin_, pwm_output);
	current_pwm_setting_ = pwm_output;
      }
    }
  }

  double Boiler::currentTemp(){ return temp_sensor_->read(); }
  
  Boiler::~Boiler(){
    gpioPWM(heater_pin_, 0);
  }
}
