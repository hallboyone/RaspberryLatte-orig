#ifndef SENSOR
#define SENSOR

namespace RaspLatte{
  template <typename RETURN_TYPE>
  class Sensor{
    virtual RETURN_TYPE read() const = 0;
  };
}
#endif
