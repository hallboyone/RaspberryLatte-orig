#ifndef ESPRESSO_MACHINE
#define ESPRESSO_MACHINE

#include <mqtt/client.h>

#include "Boiler.hpp"
#include "BinarySensor.hpp"
#include "MAX31855.hpp"
#include "pins.h"
#include "types.h"
#include "CPUThermometer.hpp"

namespace RaspLatte{
  typedef BinarySensor Switch;

  class EspressoMachine{
  private:
    /** Struct containing the current brew and steam temperature setpoints*/
    TempPair temps_;

    MAX31855 boiler_temp_sensor_;
    Boiler boiler_;
    
    Switch pwr_switch_;
    Switch pump_switch_;
    Switch steam_switch_;

    MachineMode current_mode_;

    const ModePair<PID::PIDGains> K_ = {.brew = {.p = 100, .i = 0.25, .d = 250},
					.steam = {.p = 100, .i = 0., .d = 250}};
    
    const char * ADDRESS_ = "tcp://localhost:1883";
    const char * CLIENT_ID_ = "Gaggia_Classic";
    const char * TOPIC_ = "RaspberryLatte/state";
    const char * SUB_TOPIC_ = "RaspberryLatte/settings";
    
    mqtt::client client_;
    mqtt::connect_options con_ops_;

    const CPUThermometer cpu_thermo_ = CPUThermometer();

    /*
     * Use the current state of the power and steam switch to get the curreent mode of the system
     */
    void updateMode();

    /*
     * Use the current mode and tempurature to turn on and off the lights
     */
    void updateLights();

    /**
     * Creates a char array with the current machine state and publishes it to MQTT broker.
     */
    void sendMachineStateMQTT();

    /**
     * If avalible, this will read and update the settings from a MQTT broker under RaspberryLatte/settings
     */
    void getMachineSettingsMQTT();
  public:
    EspressoMachine(double brew_temp, double steam_temp);

    void MQTTConnect();
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
