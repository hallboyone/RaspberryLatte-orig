#ifndef BOILER
#define BOILER

#include "PID.hpp"
#include "Clamp.hpp"
#include "types.h"
#include <iostream>
#include <pigpio.h>

namespace RaspLatte{
  class Boiler{
  private:
    Sensor<double> * temp_sensor_;
    PinIndex pwm_pin_;
    
    TempPair setpoints_;
    const PID::PIDGains K_brew_ = {.p = 15, .i = 0.1, .d = 50};
    const PID::PIDGains K_steam_ = {.p = 15, .i = 0.1, .d = 50};
    
    PID ctrl_;

    Clamp<double> setpoint_clamp_;

    bool active_;
    unsigned int current_pwm_setting_ = 128;
    
  public:
    Boiler(Sensor<double> * temp_sensor, PinIndex pwm_pin, TempPair setpoints):      
      temp_sensor_(temp_sensor), pwm_pin_(pwm_pin),
      setpoints_(setpoints), K_brew_{15, 0.1, 50}, K_steam_{15, 0.1, 50},
      ctrl_(temp_sensor_, K_brew_, setpoints_.brew), setpoint_clamp_(0,160){
	if (gpioInitialise() < 0){
	  throw "Could not start GPIO!";
	}

	active_ = false;
	
	ctrl_.setMinUpdateTimeSec(0.20); //Don't update the PID faster than 5Hz
	gpioSetPWMfrequency(pwm_pin, 20); //Set the Pwm to operate at 20Hz
      }

    bool brewActive(){
      return ctrl_.setPoint()==setpoints_.brew;
    }
    bool steamActive(){
      return ctrl_.setPoint()==setpoints_.steam;
    }
    
    void setBrewSetpoint(double setpoint){
      setpoint_clamp_.clamp(setpoint);
      setpoints_.brew = setpoint;
      if (brewActive()){
	ctrl_.setSetpoint(setpoint);
	update();
      }
    }
    void setSteamSetpoint(double setpoint){
      setpoint_clamp_.clamp(setpoint);
      setpoints_.steam = setpoint;
      if (steamActive()){
	ctrl_.setSetpoint(setpoint);
	update();
      }
    }
    
    void update(){
      if (!active_){
	gpioPWM(pwm_pin_, 0);
	current_pwm_setting_ = 0;
	return;
      }
      unsigned int pwm_output = ctrl_.update();
      if(pwm_output != current_pwm_setting_){ // Only update PWM setting if value changed.
	std::cout<<"Boiler temp = "<<temp_sensor_->read()<<std::endl;
	std::cout<<pwm_output<<" to boiler over pin #"<<(int)pwm_pin_<<std::endl;
	gpioPWM(pwm_pin_, pwm_output);
	current_pwm_setting_ = pwm_output;
      }
    }
    
    void activateBrew(){
      std::cout<<"Activating brew settings."<<std::endl;
      active_ = true;
      ctrl_.setSetpoint(setpoints_.brew);
      ctrl_.setGains(K_brew_);
      update();
    }
    void activateSteam(){
      std::cout<<"Activating steam settings."<<std::endl;
      active_ = true;
      ctrl_.setSetpoint(setpoints_.steam);
      ctrl_.setGains(K_steam_);
      update();
    }
    void deactivate(){
      std::cout<<"Turning off boiler\n";
      active_ = false;
      update();
    }
  };
}
#endif
