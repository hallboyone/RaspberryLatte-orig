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

    const ModePair<PID::PIDGains> K_ = {.brew = {.p = 100, .i = 0.25, .d = 250},
					.steam = {.p = 100, .i = 0., .d = 250}};
    
    /*
     * Update the current mode's setpoint by the increment. If mode is off, do nothing
     */
    void updateSetpoint(double increment);

    /*
     * Use the current state of the power and steam switch to get the curreent mode of the system
     */
    void updateMode();

    /*
     * Use the current mode and tempurature to turn on and off the lights
     */
    void updateLights();

    /*
     * Update internal paarameters based on the value of key
     */
    void handleKeyPress(int key);
    
  public:
    EspressoMachine(double brew_temp, double steam_temp);

    /*
     * Runs a loop where the UI is refreshed, any keys are handled, and the 
     * boiler is updated until 'q' is pressed. 
     */
    void run();

    /*
     * Getters
     */
    MachineMode currentMode();
    bool pumpOn();
    double setpoint();
    bool atSetpoint();

    
    ~EspressoMachine();
  };
}

#endif
