#ifndef ESPRESSO_MACHINE
#define ESPRESSO_MACHINE

#include "Boiler.hpp"
#include "BinarySensor.hpp"
#include "MAX31855.hpp"
#include "pins.h"
#include "types.h"
#include "strings.h"

#include <iostream>
#include <sstream>
#include <curses.h>

namespace RaspLatte{
  typedef BinarySensor Switch;

  
  class EspressoMachine{
  private:
    enum MachineMode {BREW, STEAM, OFF};
    
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

    MachineMode current_mode_;

    /*
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
    */
    int header_str_idx_ = 0;
    
    double setpoint(){
      if (current_mode_ == STEAM){
	return temps_.steam;
      } else {
	return temps_.brew;
      }
    }
    bool atSetpoint(){
      return ((boiler_temp_sensor_.read() < 1.05*setpoint()) & (boiler_temp_sensor_.read() > .95*setpoint()));
    }

    void updateSetpoint(double increment){
      if (current_mode_==BREW) {
	temps_.brew += increment;
	boiler_.setBrewSetpoint(temps_.brew);
      }
      else if (current_mode_==STEAM) {
	temps_.steam += increment;
	boiler_.setSteamSetpoint(temps_.steam);
      }
    }
    
    void updateMode(){
      if(pwr_switch_.read()){
	if(steam_switch_.read()){
	  current_mode_ = STEAM;
	} else {
	  current_mode_ = BREW;
	}
      } else {
	current_mode_ = OFF;
      }
    }
    
    void updateLights(){
      gpioWrite(LIGHT_PIN_PWR, pwr_switch_.read());
      gpioWrite(LIGHT_PIN_PMP, ((current_mode_ == BREW) & atSetpoint()));
      gpioWrite(LIGHT_PIN_STM, ((current_mode_ == STEAM) & atSetpoint()));
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
     *#              /\ 83.25°C                                                      #
     *#                                                                              #
     *#------------------------------------------------------------------------------#
    */

    void updateGeneralInfo(){
      // Clear, border and title
      wclear(general_win_);
      wborder(general_win_, '#', '#', '-','-','#','#','#','#');
      mvwaddstr(general_win_, 0, 29, " General Information ");

      // Status line
      mvwaddstr(general_win_, 2, 8, "Power - ");
      if(current_mode_ == OFF) waddstr(general_win_, "Off");
      else waddstr(general_win_, "On");
      mvwaddstr(general_win_, 2, 35, "Mode - ");
      if(current_mode_ == STEAM) waddstr(general_win_, "Steam");
      else waddstr(general_win_, "Brew");
      mvwaddstr(general_win_, 2, 61, "Pump - ");
      if(pump_switch_.read()) waddstr(general_win_, "On");
      else waddstr(general_win_, "Off");

      //Temp line
      mvwaddstr(general_win_, 5, 10, "|-----------------------------|-----------------------------|");
      if(current_mode_ == OFF){
	mvwprintw(general_win_, 4, 33, "Setpoint - NA", setpoint());
      } else {
	int display_range = (((int)(0.15 * setpoint())/5)+1)*5;
	mvwprintw(general_win_, 4, 9, "%0.0fC", setpoint() - display_range);
	mvwprintw(general_win_, 4, 33, "Setpoint - %0.2fC", setpoint());
	mvwprintw(general_win_, 4, 68, "%0.0fC", setpoint() + display_range);

	// Current temp pointer
	double current_temp = boiler_temp_sensor_.read();
	if(current_temp == MAX31855_TEMP_UNAVALIBLE){
	  mvwaddch(general_win_, 6, 10, ACS_UARROW);
	  wprintw(general_win_, " NA C");
	} else {
	  double delta_t = 2*display_range/60.;
	  double err = current_temp - (setpoint()-display_range);
	  int offset = err/delta_t;
	  offset = (offset < 0 ? 0 : offset);
	  offset = (offset > 60 ? 60 : offset);
	  mvwaddch(general_win_, 6, 10+offset, ACS_UARROW);
	  wprintw(general_win_, " %0.2fC", current_temp, offset);
	}
      }
      wrefresh(general_win_);
    }

    void handleKeyPress(int key){
      switch(key){
      case KEY_UP:
	updateSetpoint(1);
	break;
      case KEY_DOWN:
	updateSetpoint(-1);
	break;
      case KEY_LEFT:
	updateSetpoint(-0.25);
	break;
      case KEY_RIGHT:
	updateSetpoint(0.25);
	break;
      default:
	break;
      }
    }
    
  public:
    EspressoMachine(double brew_temp, double steam_temp): temps_{.brew=brew_temp, .steam=steam_temp}, boiler_temp_sensor_(CS_THERMO),
							  boiler_(&boiler_temp_sensor_, PWM_BOILER, temps_), pwr_switch_(SWITCH_PIN_PWR, false, true),
							  pump_switch_(SWITCH_PIN_PMP, true), steam_switch_(SWITCH_PIN_STM, true){
      updateMode();
      updateLights();
    }

    void run(){
      //Set up stuff for ncurses
      initscr();
      cbreak();
      noecho();
      curs_set(0);
      
      // Create the windows for the header, general info, and PID
      header_win_ = newwin(11, 80, 0, 0);
      general_win_ = newwin(8, 80, 11, 0);
      pid_win_ = newwin(8, 80, 19, 0);

      keypad(general_win_, TRUE);
      
      //Init the screens and refresh
      mvwaddstr(header_win_, 0, 0, HEADER_STR[header_str_idx_]);
      updateGeneralInfo();
      wrefresh(header_win_);
      wrefresh(general_win_);

      wtimeout(general_win_, 250);
      int itr = 0;
      int key_press;
      while((key_press = wgetch(general_win_)) != 'q'){
	handleKeyPress(key_press);
	if (current_mode_ != OFF){
	  itr++;
	  if (itr == 2){
	    itr = 0;
	    if(header_str_idx_ == 4) header_str_idx_ = 0;
	    else header_str_idx_++;
	    mvwaddstr(header_win_, 0, 0, HEADER_STR[header_str_idx_]);
	    wrefresh(header_win_);
	  }
	}
	updateLights();
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
