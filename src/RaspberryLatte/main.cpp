#include "../../include/RaspberryLatte/EspressoMachine.hpp"
#include <iostream>

int main(void){
  RaspLatte::EspressoMachine gaggia_classic(95, 140);
  gaggia_classic.run();
  return 0;
}
  
