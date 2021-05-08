#include "../../include/RaspberryLatte/RaspberryLatteUI.hpp"
#include "../../include/RaspberryLatte/EspressoMachine.hpp"
#include "../../include/RaspberryLatte/Boiler.hpp"
#include "../../include/RaspberryLatte/strings.h"

namespace RaspLatte{    
  void RaspberryLatteUI::updateGeneralWindow(bool init){
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
      
    if(machine_->currentMode() == OFF) mvwaddstr(general_win_, 2, 16, "Off");
    else mvwaddstr(general_win_, 2, 16, "On ");

    if(machine_->currentMode() == STEAM) mvwaddstr(general_win_, 2, 42, "Steam");
    else mvwaddstr(general_win_, 2, 42, "Brew ");
      
    if(machine_->pumpOn()) mvwaddstr(general_win_, 2, 68, "On ");
    else mvwaddstr(general_win_, 2, 68, "Off");

    //Temp line
    if(machine_->currentMode() == OFF){
      mvwprintw(general_win_, 4, 33, "Setpoint - NA   ", machine_->setpoint());
    } else {
      int display_range = (((int)(0.15 * machine_->setpoint())/5)+1)*5;
      mvwprintw(general_win_, 4, 9, "%0.0fC ", machine_->setpoint() - display_range);
      mvwprintw(general_win_, 4, 33, "Setpoint - %0.2fC", machine_->setpoint());
      mvwprintw(general_win_, 4, 68, "%0.0fC ", machine_->setpoint() + display_range);
      mvwprintw(general_win_, 6, last_setpoint_slider_loc_, "         ");

      // Current temp pointer
      double current_temp = boiler_->currentTemp();
      if(current_temp == MAX31855_TEMP_UNAVALIBLE){
	mvwaddch(general_win_, 6, 10, ACS_UARROW);
	wprintw(general_win_, " NA C");
      } else {
	double delta_t = 2*display_range/60.;
	double err = current_temp - (machine_->setpoint()-display_range);
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
    
  void RaspberryLatteUI::updateBoilerWindow(bool init){
    if (init){
      // Clear, border and title
      wclear(boiler_win_);
      wborder(boiler_win_, '#', '#', '-','=','#','#','#','#');
      mvwaddstr(boiler_win_, 0, 34, " PID Status ");

      // PWM output line
      mvwprintw(boiler_win_, 1, 32, "PWM Output - %0.0f  ", boiler_->currentPWM());
      mvwprintw(boiler_win_, 3, 7, "Setpoint - %0.2f    ", boiler_->setpoint());
      mvwprintw(boiler_win_, 3, 32, "Current - %0.2f    ", boiler_->currentTemp());
      mvwprintw(boiler_win_, 3, 58, "Error - %0.2f    ", boiler_->setpoint() - boiler_->currentTemp());
      mvwprintw(boiler_win_, 4, 7, "Error Sum - %0.2f    ", boiler_->errorSum());
      mvwprintw(boiler_win_, 4, 32, "Slope - %0.2f    ", boiler_->errorSlope());
    }
    else {
      mvwprintw(boiler_win_, 1, 45, "%0.0f   ", boiler_->currentPWM());
      mvwprintw(boiler_win_, 3, 18, "%0.2f    ", boiler_->setpoint());
      mvwprintw(boiler_win_, 3, 42, "%0.2f    ", boiler_->currentTemp());
      mvwprintw(boiler_win_, 3, 66, "%0.2f    ", boiler_->setpoint() - boiler_->currentTemp());
      mvwprintw(boiler_win_, 4, 19, "%0.2f    ", boiler_->errorSum());
      mvwprintw(boiler_win_, 4, 40, "%0.2f    ", boiler_->errorSlope());
    }
      
    wrefresh(boiler_win_);
  }
    
  RaspberryLatteUI::RaspberryLatteUI(EspressoMachine * machine, Boiler * boiler): machine_(machine), boiler_(boiler){}

  void RaspberryLatteUI::init(){
    //Set up stuff for ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);
      
    // Create the windows for the header, general info, and PID
    header_win_ = newwin(11, 80, 0, 0);
    general_win_ = newwin(8, 80, 11, 0);
    boiler_win_ = newwin(8, 80, 19, 0);

    keypad(general_win_, TRUE);

    wtimeout(general_win_,500);
      
    //Init the screens and refresh
    mvwaddstr(header_win_, 0, 0, HEADER_STR[0]);
    wrefresh(header_win_);

    updateGeneralWindow();
    updateBoilerWindow();
  }
    
  int RaspberryLatteUI::refresh(){
    int key_press = wgetch(general_win_);
    updateGeneralWindow(false);
    updateBoilerWindow(false);
    return key_press;
  }
}
