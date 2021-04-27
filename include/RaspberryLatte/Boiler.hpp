#ifndef BOILER
#define BOILER

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
    MachineMode current_mode_;
    
    const ModePair<PID::PIDGains> K_ = {.brew = {.p = 100, .i = 0.25, .d = 250},
					.steam = {.p = 35, .i = 0, .d = 200}};
    
    PID ctrl_;

    Clamp<double> setpoint_clamp_;

    unsigned int current_pwm_setting_ = 0;

    void switchMode();
  public:
    Boiler(Sensor<double> * temp_sensor, TempPair * setpoints, MachineMode * mode, PinIndex heater_pin);
    void update(int feed_forward = 0);
    void updatePIDWin(WINDOW * pid_win, bool init = true);
    ~Boiler();
  };
}
#endif
