#ifndef RASPBERRY_LATTE_UI
#define RASPBERRY_LATTE_UI

#include <curses.h>

#include "CPUThermometer.hpp"

namespace RaspLatte{
  class EspressoMachine;
  class Boiler;
  
  class RaspberryLatteUI{
  private:
    EspressoMachine * machine_;
    Boiler * boiler_;

    WINDOW * header_win_;
    WINDOW * general_win_;
    WINDOW * boiler_win_;

    int last_setpoint_slider_loc_ = 0;
    CPUThermometer cpu_thermo_;
    
    void updateGeneralWindow(bool init = true);
    void updateBoilerWindow(bool init = true);
  public:
    RaspberryLatteUI(EspressoMachine * machine, Boiler * boiler);

    void init();
    /**
     * Wait 0.25 sec for key press and then update windows and return key
     */
    int refresh();
  };
}
#endif
