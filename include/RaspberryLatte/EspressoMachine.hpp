#ifndef ESPRESSO_MACHINE
#define ESPRESSO_MACHINE

#include "Boiler.hpp"
#include "BinarySensor.hpp"
#include "MAX31855.hpp"
#include "pins.h"
#include "types.h"
#include "strings.h"
#include "CPUThermometer.hpp"

#include <iostream>
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

    WINDOW * header_win_;
    WINDOW * general_win_;
    WINDOW * pid_win_;

    MachineMode current_mode_;

    int header_str_idx_ = 0;
    int last_setpoint_slider_loc_ = 1;

    CPUThermometer cpu_thermo_;
    
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
      }
      else if (current_mode_==STEAM) {
	temps_.steam += increment;
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
      gpioWrite(LIGHT_PIN_PWR, current_mode_ != OFF);
      gpioWrite(LIGHT_PIN_PMP, ((current_mode_ == BREW) & atSetpoint()));
      gpioWrite(LIGHT_PIN_STM, ((current_mode_ == STEAM) & atSetpoint()));
      return;
    }

    void updateGeneralInfoWin(bool init = true){
      if (init){
	// Clear, border and title
	wclear(general_win_);
	wborder(general_win_, '#', '#', '-','=','#','#','#','#');
	mvwaddstr(general_win_, 0, 29, " General Information ");

	// Status line
	mvwaddstr(general_win_, 2, 8, "Power - ");
	mvwaddstr(general_win_, 2, 35, "Mode - ");
	mvwaddstr(general_win_, 2, 61, "Pump - ");

	mvwaddstr(general_win_, 5, 10, "|-----------------------------|-----------------------------|");
      }
      
      if(current_mode_ == OFF) mvwaddstr(general_win_, 2, 16, "Off");
      else mvwaddstr(general_win_, 2, 16, "On ");

      if(current_mode_ == STEAM) mvwaddstr(general_win_, 2, 42, "Steam");
      else mvwaddstr(general_win_, 2, 42, "Brew ");
      
      if(pump_switch_.read()) mvwaddstr(general_win_, 2, 68, "On ");
      else mvwaddstr(general_win_, 2, 68, "Off");

      //Temp line
      if(current_mode_ == OFF){
	mvwprintw(general_win_, 4, 33, "Setpoint - NA", setpoint());
      } else {
	int display_range = (((int)(0.15 * setpoint())/5)+1)*5;
	mvwprintw(general_win_, 4, 9, "%0.0fC ", setpoint() - display_range);
	mvwprintw(general_win_, 4, 33, "Setpoint - %0.2fC", setpoint());
	mvwprintw(general_win_, 4, 68, "%0.0fC ", setpoint() + display_range);
	mvwprintw(general_win_, 6, last_setpoint_slider_loc_, "         ");

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
	  last_setpoint_slider_loc_ = 10 + offset;
	}
      }
      mvwprintw(general_win_, 7, 20, "%0.2fC", cpu_thermo_.getTemp());
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
							  boiler_(&boiler_temp_sensor_, &temps_, &current_mode_, PWM_BOILER), pwr_switch_(SWITCH_PIN_PWR, false, true),
							  pump_switch_(SWITCH_PIN_PMP, true), steam_switch_(SWITCH_PIN_STM, true){
      current_mode_ = OFF; // Keep machine off until run() is called
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
      updateGeneralInfoWin();
      
      boiler_. updatePIDWin(pid_win_);
      
      wrefresh(header_win_);
      wrefresh(general_win_);

      wtimeout(general_win_, 500);
      int key_press;
      while((key_press = wgetch(general_win_)) != 'q'){
	updateMode();
	updateLights();
	updateGeneralInfoWin(false);
	if (current_mode_ != OFF){
	  handleKeyPress(key_press);
	  boiler_. updatePIDWin(pid_win_, false);
	  if (pump_switch_.read()){
	    boiler_.update(128);
	  }
	  else {
	    boiler_.update();
	  }
	}
      }
    }

    ~EspressoMachine(){
      gpioWrite(LIGHT_PIN_PWR, 0);
      gpioWrite(LIGHT_PIN_PMP, 0);
      gpioWrite(LIGHT_PIN_STM, 0);
      endwin();
    }
  };
}

#endif
