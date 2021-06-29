//#include "../../include/RaspberryLatte/EspressoMachine.hpp"
#include "../../include/RaspberryLatte/MQTT_UI.hpp"
#include <iostream>

int main(void){
  //  RaspLatte::EspressoMachine gaggia_classic(95, 150);
  //gaggia_classic.MQTTConnect();
  //gaggia_classic.run();
  RaspLatte::MQTTClient client;
  client.connect();
  //  client.run();
  return 0;
}
  




