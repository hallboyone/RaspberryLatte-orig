#ifndef SENSOR
#define SENSOR

namespace RaspLatte{
  template <typename RETURN_TYPE>
  class Sensor{
  public:
    virtual RETURN_TYPE read() = 0;
    virtual ~Sensor(){};
  };
}
#endif
