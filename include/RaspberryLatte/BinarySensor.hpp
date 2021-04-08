#ifndef BINARY_SENSOR
#define BINARY_SENSOR

#include "Sensor.hpp"
#include "types.h"
#include <pigpio.h>

namespace RaspLatte{
  class BinarySensor : public Sensor<bool>{
    /**
     * A sensor class that detects a binary signal from a GPIO pin. The class should have several features
     * - Implement the read function that returns the sensor's state
     * - A way to attach a callback function to be run when the sensor changes state (i.e. a user presses a button) (FUTURE WORK)
     * - A debounce mechenism that limits the rate of switching (FUTURE WORK)
     */
  public:
    BinarySensor(const PinIndex p, const bool invert = false, const bool pull_down = false): p_(p), invert_(invert){
      if (gpioInitialise() < 0){
	throw "Could not start GPIO!";
      }
      
      gpioSetMode(p_, PI_INPUT);
      if (pull_down){
	gpioSetPullUpDown(p_, PI_PUD_DOWN);
      } else {
	gpioSetPullUpDown(p_, PI_PUD_UP);
      }
    }

    virtual bool read() const{
      int sensor_val = gpioRead(p_);
      if (sensor_val==PI_BAD_GPIO){
	throw "Bad GPIO pin!";
      }
      if (invert_){
	return (sensor_val==0);
      } else {
	return (sensor_val==1);
      }
    }
    
  private:
    const PinIndex p_;
    const bool invert_; 
  };
}
#endif
