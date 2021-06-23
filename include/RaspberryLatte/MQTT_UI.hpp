#ifndef RASPBERRY_LATTE_MQQT_CLIENT
#define RASPBERRY_LATTE_MQQT_CLIENT

#include <mqtt/client.h>
#include <iostream>

namespace RaspLatte{
  class MQTTClient{
  private:
    const char * ADDRESS_ = "tcp://localhost:1883";
    const char * CLIENT_ID_ = "Gaggia_Classic";
    const char * TOPIC_ = "RaspberryLatte/state";

    mqtt::client client_;
    mqtt::connect_options con_ops_;
    
  public:

    MQTTClient(): client_(ADDRESS_, CLIENT_ID_){
      con_ops_.set_keep_alive_interval(20);
      con_ops_.set_clean_session(true);
    }

    void connect(){
      try{
	std::cout<<"Connection...";
	client_.connect(con_ops_);
	std::cout<<"\nDone. Sending...";
	client_.publish(TOPIC_, "Hello World", 12);
	std::cout<<"\nDone. Sending message 2...";
	client_.publish(TOPIC_, "How are you?", 13);
	std::cout<<"Done!\n";
      }
      catch (const mqtt::exception& exc) {
	std::cerr << exc.what() << std::endl;
      }
    }
    
  };
}

#endif
