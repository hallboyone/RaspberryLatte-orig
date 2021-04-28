#ifndef ESPRESSO_MACHINE
#define ESPRESSO_MACHINE

#include "Boiler.hpp"
#include "BinarySensor.hpp"
#include "MAX31855.hpp"
#include "pins.h"
#include "types.h"
#include "RaspberryLatteUI.hpp"

namespace RaspLatte{
  typedef BinarySensor Switch;

  class EspressoMachine{
  private:
    
    TempPair temps_;
    
    MAX31855 boiler_temp_sensor_;
    
    Boiler boiler_;

    RaspberryLatteUI ui_;
    
    Switch pwr_switch_;
    Switch pump_switch_;
    Switch steam_switch_;

    MachineMode current_mode_;
        
    bool atSetpoint();
    void updateSetpoint(double increment);
    void updateMode();
    void updateLights();
    void handleKeyPress(int key);
    
  public:
    EspressoMachine(double brew_temp, double steam_temp);
    void run();
    MachineMode currentMode();
    bool pumpOn();
    double setpoint();
    ~EspressoMachine();
  };
}

#endif
