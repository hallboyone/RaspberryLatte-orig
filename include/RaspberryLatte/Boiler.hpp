#ifndef BOILER
#define BOILER

#include "PID.hpp"
#include "types.h"
#include <iostream>
#include <pigpio.h>

namespace RaspLatte{
  class Boiler{
  private:
    Sensor<double> * temp_sensor_;
    PinIndex pwm_pin_;
    
    double set_point_brew_;
    double set_point_steam_;
    const PID::PIDGains K_brew_ = {.p = 15, .i = 0.1, .d = 50};
    const PID::PIDGains K_steam_ = {.p = 15, .i = 0.1, .d = 50};
    
    PID ctrl_;

    Clamper set_point_clamp_;

    bool active_;
    unsigned int current_pwm_setting_ = 128;
    
  public:
    Boiler(Sensor<double> * temp_sensor, PinIndex pwm_pin, double set_point_brew, double set_point_steam):
      temp_sensor_(temp_sensor), pwm_pin_(pwm_pin),
      set_point_brew_(set_point_brew), set_point_steam_(set_point_steam), K_brew_{15, 0.1, 50}, K_steam_{15, 0.1, 50},
      ctrl_(temp_sensor_, K_brew_, set_point_brew_), set_point_clamp_(-10000,160){
	active_ = false;
	
	ctrl_.setMinUpdateTimeSec(0.20); //Don't update the PID faster than 5Hz
	gpioSetPWMfrequency(pwm_pin, 20); //Set the Pwm to operate at 20Hz
      }

    bool brewActive(){
      return ctrl_.setPoint()==set_point_brew_;
    }
    bool steamActive(){
      return ctrl_.setPoint()==set_point_steam_;
    }
    
    void setBrewSetpoint(double set_point){
      set_point_clamp_.clamp(set_point);
      set_point_brew_ = set_point;
      if (brewActive()){
	ctrl_.setSetpoint(set_point);
	update();
      }
    }
    void setSteamSetpoint(double set_point){
      set_point_clamp_.clamp(set_point);
      set_point_steam_ = set_point;
      if (steamActive()){
	ctrl_.setSetpoint(set_point);
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
      ctrl_.setSetpoint(set_point_brew_);
      ctrl_.setGains(K_brew_);
      update();
    }
    void activateSteam(){
      std::cout<<"Activating steam settings."<<std::endl;
      active_ = true;
      ctrl_.setSetpoint(set_point_steam_);
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
