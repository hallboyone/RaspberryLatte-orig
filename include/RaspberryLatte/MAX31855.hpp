#ifndef MAX_31855
#define MAX_31855

#include <pigpio.h>
#include <iostream>

#include "Sensor.hpp"
#include "types.h"

#define MAX31855_ERR_OPEN_CIRCUIT 1
#define MAX31855_ERR_GND_SHORT 2
#define MAX31855_ERR_VCC_SHORT 4
#define MAX31855_TEMP_UNAVALIBLE -1000
namespace RaspLatte{
  class MAX31855 : public Sensor<double> {
    /**
     * Class that abstracts the SPI interface and setup of the MAX31855 thermocouple-
     * to-digital breakout board from Adafruit.
     */
  public:
    MAX31855(int spi_select_pin){
      handle_ = spiOpen(spi_select_pin, 1000000, 0);
      if (handle_ < 0){
	throw "Error: Could not open SPI to MAX31855.";
      }
    }

    double read(){
      updateData();
      if (err_){
	return MAX31855_TEMP_UNAVALIBLE;
      }
      return thermo_temp_;
    }

    double readChipTemp(){
      updateData();
      if (err_){
	return MAX31855_TEMP_UNAVALIBLE;
      }
      return chip_temp_;
    }
    
    double readThermoTemp(){
      return read();
    }

    uint8_t readError(){
      return err_;
    }
    
  private:
    int handle_;

    float thermo_temp_;
    float chip_temp_;
    uint8_t err_;
    
    void updateData(){
      /*   A      B     C    D    E      F    G   H   I   J
         | 31 | 30-18 | 17 | 16 | 15 | 14-4 | 3 | 2 | 1 | 0
	 
	 A - Sign of thermo temp
	 B - Thermo temp. 0 bit at 20
	 C - Reserved
	 D - Fault bit
	 E - Sign of chip temp
	 F - Chip temp. 0 bit at 8
	 G - Reserved
	 H - Short to Vcc
	 I - Short to Gnd
	 J - Open circuit
      */

      char c_buf[4] = {0,0,0,0};
      
      if (spiRead(handle_, c_buf, 4) < 0){
	throw "Error: Could not read data from MAX31855 over SPI";
      }

      int32_t buf = (c_buf[3]<<24) | (c_buf[2] << 16) | (c_buf[1] << 8) | c_buf[0];
      
      // Errors
      err_ = (buf & 0x7);
      if (err_) return; // Nothing else to do. Error is set

      buf >>= 4; // Dump buttom 4 bits (error bits and reserved bit)

      // Read the lower 11 bits remaining in buf. This is the unsigned ship temp
      if (buf & 0x800) { //negative chip temp
	// Convert to negative value by extending sign and casting to signed type.
	int16_t tmp = 0xF800 | (buf & 0x7FF);
	chip_temp_ = tmp;
      } else {
	chip_temp_ = (buf & 0x7FF);
      }
      chip_temp_ *= 0.0625;
	
      buf >>= 14; // Dump the next 14 bits (chip temp, reserved, and fault bit)

      if (buf & 0x200) {
	// Negative value, drop the lower 18 bits and explicitly extend sign bits.
	thermo_temp_ = 0xFFFFC000 | buf;
      } else {
	// Positive value, just drop the lower 18 bits.
        thermo_temp_ = buf;
      }
      thermo_temp_ *= 0.25;
    }
  };
}
#endif
