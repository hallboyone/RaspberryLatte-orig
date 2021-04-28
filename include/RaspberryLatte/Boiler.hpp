#ifndef BOILER
#define BOILER

#include "PID.hpp"
#include "Clamp.hpp"
#include "types.h"


namespace RaspLatte{

  /**
   * The Boiler class represents a single boiler within an espresso machine. 
   * Every boiler has the following components
   * (a) A temp sensor (Sensor<double> object)
   * (b) A heating element
   * (c) A controller
   *
   * The Boiler class is tasked with tracking a setpoint set using a function 
   * call. It is also responsible for ensuring the setpoint is within the 
   * physical bounds of the boiler.
   */
  
  class Boiler{
  private:
    Sensor<double> * temp_sensor_;
    double setpoint_;
    PID ctrl_;
    PinIndex heater_pin_;
    bool active_;
    unsigned int current_pwm_setting_ = 0;
    Clamp<double> setpoint_clamp_;
    
    /*
    TempPair * setpoints_;
    MachineMode * mode_;
    MachineMode current_mode_;
    
    const ModePair<PID::PIDGains> K_ = {.brew = {.p = 100, .i = 0.25, .d = 250},
					.steam = {.p = 100, .i = 0., .d = 250}};
    */

    
    

    //void switchMode();
  public:
    Boiler(Sensor<double> * temp_sensor, TempPair * setpoints, MachineMode * mode, PinIndex heater_pin);
    Boiler(Sensor<double> * temp_sensor, double setpoint, const PID::PIDGains * pid_gains, PinIndex heater_pin_idx,
	   double min_setpoint = 0, double max_setpoint = 160);

    void turnOn();
    void turnOff();
    double updateSetpoint(double setpoint, const PID::PIDGains * new_gains = NULL);
    
    void update(int feed_forward = 0);

    double currentTemp();
    double currentPWM(){ return ctrl_.u(); }
    double setpoint(){ return ctrl_.setpoint(); }
    double errorSlope() { return ctrl_.slope(); }
    double errorSum() { return ctrl_.errorSum(); }
    
    ~Boiler();
  };
}
#endif
