#ifndef RASPBERRY_LATTE_MQQT_UI
#define RASPBERRY_LATTE_MQQT_UI

#include "MQTTClient.h"

#define TIMEOUT     10000L

namespace RaspLatte{
  class RaspberryLatteMQQTUI{
  private:
    const char * ADDRESS_ = "tcp://localhost:1883";
    const char * CLIENTID_ = "Gaggia_Classic";
    const char * TOPIC_ = "RaspberryLatte/state";
    const int QOS_ = 1;

    MQTTClient_connectOptions conn_opts_ = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg_ = MQTTClient_message_initializer;
      
    MQTTClient client_;
  public:
    RaspberryLatteMQQTUI(){
      conn_opts_.keepAliveInterval = 20;
      conn_opts_.cleansession = 1;
      
      //MQTTClient_deliveryToken token;
      int rc;
      
      MQTTClient_create(&client_, ADDRESS_, CLIENTID_,
			MQTTCLIENT_PERSISTENCE_NONE, NULL);
      if ((rc = MQTTClient_connect(client_, &conn_opts_)) != MQTTCLIENT_SUCCESS)
	{
	  printf("Failed to connect, return code %d\n", rc);
	  throw "Could not connect";
	}
    }
  };
}

#endif
