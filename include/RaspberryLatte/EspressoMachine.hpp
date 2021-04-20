#ifndef ESPRESSO_MACHINE
#define ESPRESSO_MACHINE

#include "Boiler.hpp"
#include "BinarySensor.hpp"
#include "MAX31855.hpp"
#include "pins.h"
#include "types.h"

#include <iostream>
#include <sstream>
#include <curses.h>

namespace RaspLatte{
  typedef BinarySensor Switch;
 
  class EspressoMachine{
  private:
    TempPair temps_;
    
    MAX31855 boiler_temp_sensor_;
    
    Boiler boiler_;

    Switch pwr_switch_;
    Switch pump_switch_;
    Switch steam_switch_;

    int output_len_;

    WINDOW * header_win_;
    WINDOW * general_win_;
    WINDOW * pid_win_;

    const char * header_str_ =
      "################################################################################"
      "#       )   (           ___    _    ___  ___  ___  ___  ___  ___ __   __       #"
      "#      (    )  )       | _ \\  /_\\  / __|| _ \\| _ )| __|| _ \\| _ \\\\ \\ / /       #"
      "#       )  (  (        |   / / _ \\ \\__ \\|  _/| _ \\| _| |   /|   / \\ V /        #"
      "#     ________)_       |_|_\\/_/ \\_\\|___/|_|  |___/|___||_|_\\|_|_\\  |_|         #"
      "#   .-'---------|               _       _  _____  _____  ___                   #"
      "#  ( c|/\\/\\/\\/\\/|              | |     /_\\|_   _||_   _|| __|                  #"
      "#   '-./\\/\\/\\/\\/|              | |__  / _ \\ | |    | |  | _|                   #"
      "#     '_________'              |____|/_/ \\_\\|_|    |_|  |___|                  #"
      "#      '-------'                                                               #"
      "#==============================================================================#";

    static void printOnOff(bool on){
      if (on) std::cout<<"ON";
      else std::cout<<"OFF";
    }

    bool atSetpoint(){
      if (steam_switch_.read()){
	return (boiler_temp_sensor_.read() < 1.05*temps_.steam & boiler_temp_sensor_.read() > .95*temps_.steam);
      } else {
	return (boiler_temp_sensor_.read() < 1.05*temps_.brew & boiler_temp_sensor_.read() > .95*temps_.brew);
      }
    }
    
    void setLightStates(){
      gpioWrite(LIGHT_PIN_PWR, pwr_switch_.read());
      gpioWrite(LIGHT_PIN_PMP, (!steam_switch_.read()) & atSetpoint());
      gpioWrite(LIGHT_PIN_STM, (steam_switch_.read()) & atSetpoint());
      return;
    }

    //          1         2         3         4         5         6         7       
    /*01234567890123456789012345678901234567890123456789012345678901234567890123456789
     *#_____________________________General Information______________________________#
     *#                                                                              #
     *#       Power - On                 Mode - Steam              Pump - Off        #
     *#                                                                              #
     *#        80°C                    Setpoint - 95°C                   110°C       #
     *#         |-----------------------------|-----------------------------|        #
     *#                                                                              #
     *#------------------------------------------------------------------------------#
    */
    void updateGeneralInfo(){
      wborder(general_win_, '#', '#', '_','-','#','#','#','#');
      mvwaddstr(general_win_, 0, 30, "General Information");
      mvwaddstr(general_win_, 2, 8, "Power - ");
      if(pwr_switch_.read()) waddstr(general_win_, "On");
      else waddstr(general_win_, "Off");
      mvwaddstr(general_win_, 2, 35, "Mode - ");
      if(steam_switch_.read()) waddstr(general_win_, "Steam");
      else waddstr(general_win_, "Brew");
      mvwaddstr(general_win_, 2, 61, "Pump - ");
      if(pump_switch_.read()) waddstr(general_win_, "On");
      else waddstr(general_win_, "Off");
      wrefresh(general_win_);
    }
    
  public:
    EspressoMachine(double brew_temp, double steam_temp): temps_{.brew=brew_temp, .steam=steam_temp}, boiler_temp_sensor_(CS_THERMO),
							  boiler_(&boiler_temp_sensor_, PWM_BOILER, temps_), pwr_switch_(SWITCH_PIN_PWR, false, true),
							  pump_switch_(SWITCH_PIN_PMP, true), steam_switch_(SWITCH_PIN_STM, true){
    }

    void run(){
      //Set up stuff for ncurses
      initscr();
      cbreak();
      noecho();
      keypad(stdscr, TRUE);
      
      header_win_ = newwin(11, 80, 0, 0);
      general_win_ = newwin(8, 80, 11, 0);
      pid_win_ = newwin(8, 80, 19, 0);

      mvwaddstr(header_win_, 0, 0, header_str_);
      updateGeneralInfo();
      wrefresh(header_win_);
      wrefresh(general_win_);

      timeout(50);
      while(getch() != 'q'){
	setLightStates();
	updateGeneralInfo();
	//refresh();
      }
    }
    
    void update(){
      updateGeneralInfo();
      refresh();
    }
    
    void printStatus(){
      /*
       *  $Power: ON , Pump: OFF, Targer Temp: 140°C, Current Temp: 135°C 
       */
      std::ostringstream output;  
      output<<"Power: ";
      if (pwr_switch_.read()) output<<"ON ";
      else output<<"OFF";
      output<<", Pump: ";
      if (pump_switch_.read()) output<<"ON ";
      else output<<"OFF";
      output<<", Target Temp: ";
      if(steam_switch_.read()) output<<temps_.steam<<"°C";
      else output<<temps_.brew<<"°C";
      if(boiler_temp_sensor_.read() == MAX31855_TEMP_UNAVALIBLE){
	boiler_temp_sensor_.printError();
	std::cout<<"\n";
      }
      else output<<", Current Temp: "<<boiler_temp_sensor_.read()<<"°C\n";
      output_len_ = output.str().length();
      std::cout<<output.str();
      output_len_ += boiler_.printStatus();
    }

    ~EspressoMachine(){
      endwin();
    }
  };
}

#endif
