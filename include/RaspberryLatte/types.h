#ifndef TYPES
#define TYPES

#include <cstdint>
#include <chrono>

namespace RaspLatte{
  typedef uint8_t PinIndex;
  typedef uint32_t SPIBaud;

  typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>> TimePoint;
  typedef std::chrono::duration<double> Duration;

  /*  typedef struct TempPair_{
    double brew;
    double steam;
  } TempPair;*/

  template <typename T>
  struct ModePair{
    T brew;
    T steam;
  };
  
  typedef ModePair<double> TempPair;
  
  enum MachineMode {OFF, BREW, STEAM};
}
#endif
