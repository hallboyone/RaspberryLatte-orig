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
    class EspressoMachineSettings{
    private:
      TempPair temp_;
      char * encoded_data_;

      void encode(){
	int tmp_temp = 100*temp_.brew;
	encoded_data_[0] = tmp_temp      & 0xff;
	encoded_data_[1] = (tmp_temp>>8) & 0xff;
	tmp_temp = 100*temp_.steam;
	encoded_data_[2] = tmp_temp      & 0xff;
	encoded_data_[3] = (tmp_temp>>8) & 0xff;
      }
    public:
      EspressoMachineSettings(double brew_temp, double steam_temp): temp_{.brew=brew_temp, .steam=steam_temp} {
	encoded_data_ = new char [4];
      }
      
      double brewTemp(){ return temp_.brew; }
      double steamTemp(){ return temp_.steam; }
      
      const char * encodedData(){
	encode();
	return encoded_data_;
      }

      void decodeData(const char * data){
	int tmp_temp = data[1];
	tmp_temp = (tmp_temp<<8) | data[0];
	temp_.brew = (double)tmp_temp/100.;
	tmp_temp = data[3];
	tmp_temp = (tmp_temp<<8) | data[2];
	temp_.steam = (double)tmp_temp/100.;
      }

      ~EspressoMachineSettings(){
	delete(encoded_data_);
      }
    };
      
    /** Struct containing the current brew and steam temperature setpoints*/
    TempPair temps_;
    EspressoMachineSettings settings_;
    
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
