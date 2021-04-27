#include "../../include/RaspberryLatte/EspressoMachine.hpp"
#include <iostream>

int main(void){
  RaspLatte::EspressoMachine gaggia_classic(95, 150);
  gaggia_classic.run();
  return 0;
}
  
