#ifndef BOILER
#define BOILER

#include "PID.hpp"
#include "Clamp.hpp"
#include "types.h"
#include "PWMOutput.hpp"

namespace RaspLatte{
  /**
   * The Boiler class represents a single boiler within an espresso machine. 
   * Every boiler has the following components
   * (a) A temp sensor (Sensor<double> * )
   * (b) A heating element (A PWM output)
   * (c) A controller (PID)
   *
   * The Boiler class is tasked with tracking a setpoint set using a function 
   * call. It is also responsible for ensuring the setpoint is within the 
   * physical bounds of the boiler.
   */
  
  class Boiler{
  private:
    Sensor<double> * const temp_sensor_; /** The sensor measuring the boiler's temp */
    double setpoint_; /** The setpoint being tracked by the boiler when active */
    PID ctrl_; /** A PID controller regulating the PWM output */
    PWMOutput heater_;
    bool active_; /** A boolean indicating if the heater is on */
    Clamp<double> setpoint_clamp_; /** A clamp object that clips the setpoint within reasionable bounds */

  public:
    Boiler(Sensor<double> * const temp_sensor, double setpoint, const PID::PIDGains * pid_gains, PinIndex heater_pin_idx,
	   double min_setpoint = 0, double max_setpoint = 160);

    void turnOn();
    void turnOff();
    double updateSetpoint(double setpoint, const PID::PIDGains * new_gains = NULL);
    
    void update(int feed_forward = 0);

    double temp();
    double currentPWM(){ return heater_.currentDutyCycle(); }
    double setpoint(){ return ctrl_.setpoint(); }
    double errorSlope() { return ctrl_.slope(); }
    double errorSum() { return ctrl_.errorSum(); }
  };
}
#endif
