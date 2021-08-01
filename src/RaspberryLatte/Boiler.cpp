#include "../../include/RaspberryLatte/Boiler.hpp"

#include <iostream>

namespace RaspLatte{  
  Boiler::Boiler(Sensor<double> * const temp_sensor, double setpoint, const PID::PIDGains * pid_gains, PinIndex heater_pin_idx,
		 double min_setpoint, double max_setpoint):
    temp_sensor_(temp_sensor), setpoint_(setpoint), ctrl_(*pid_gains, &setpoint_, temp_sensor_), heater_(heater_pin_idx),
    active_(false), setpoint_clamp_(min_setpoint, max_setpoint){

    // Set defaults
    ctrl_.setMinUpdateTimeSec(0.20); //Don't update the PID faster than 5Hz
    ctrl_.setIntegralSumLimits(0, 100);
    ctrl_.setInputLimits(0, 255);
    ctrl_.setSlopePeriodSec(1.1);
  }

  void Boiler::turnOn(){
    active_ = true;
    ctrl_.reset();
    update();
  }

  void Boiler::turnOff(){
    active_ = false;
    heater_.setPowerTo(0);
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
      heater_.setPowerTo(ctrl_.update(feed_forward));
    }
  }

  double Boiler::temp(){ return temp_sensor_->read(); }
}
