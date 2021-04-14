#ifndef BOILER
#define BOILER

#include "PID.hpp"
#include "types.h"
#include <iostream>

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
    
  public:
    Boiler(Sensor<double> * temp_sensor, PinIndex pwm_pin, double set_point_brew, double set_point_steam):
      temp_sensor_(temp_sensor), pwm_pin_(pwm_pin),
      set_point_brew_(set_point_brew), set_point_steam_(set_point_steam), K_brew_{15, 0.01, 50}, K_steam_{15, 0.01, 50},
      ctrl_(temp_sensor_, K_brew_, set_point_brew_), set_point_clamp_(0,160){
	active_ = false;
      }

    bool brewActive(){
      return ctrl_.setPoint()==set_point_brew_;
    }
    bool steamActive(){
      return ctrl_.setPoint()==set_point_steam_;
    }
    
    void setBrewSetpoint(double set_point){
      set_point_brew_ = set_point;
      if (brewActive()){
	ctrl_.setSetpoint(set_point);
      }
    }
    void setSteamSetpoint(double set_point){
      set_point_steam_ = set_point;
      if (steamActive()){
	ctrl_.setSetpoint(set_point);
      }
    }
    
    void update(){
      if (!active_) return;
      double pwm_output = ctrl_.update();
      std::cout<<pwm_output<<" to pi #"<<pwm_pin_<<std::endl;
      // Output PWM
    }
    void activateBrew(){
      active_ = true;
      ctrl_.setSetpoint(set_point_brew_);
      ctrl_.setGains(K_brew_);
    }
    void activateSteam(){
      active_ = true;
      ctrl_.setSetpoint(set_point_steam_);
      ctrl_.setGains(K_steam_);
    }
    void deactivate(){
      active_ = false;
    }
  };
}
#endif
