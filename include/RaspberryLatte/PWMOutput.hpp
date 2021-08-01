#ifndef PWM_OUTPUT
#define PWM_OUTPUT

#include "types.h"
#include "pigpio.h"

namespace RaspLatte{
  class PWMOutput{
  public:
    PWMOutput(const PinIndex p, const int freq = 20): p_(p), current_power_(0){
      if (gpioInitialise() < 0) throw "Could not start GPIO!";
      gpioSetPWMfrequency(p, freq);
      gpioPWM(p, 0);
    }

    void setPowerTo(uint8_t pwr){
      if(pwr != current_power_){
	gpioPWM(p_, pwr);
	current_power_ = pwr;
      }
    }

    int currentDutyCycle(){ return current_power_; };

    ~PWMOutput(){
      gpioPWM(p_, 0);
    }
    
  private:
    const PinIndex p_;
    uint8_t current_power_;
  };
}
#endif
