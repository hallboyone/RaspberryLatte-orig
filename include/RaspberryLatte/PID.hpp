#ifndef PID_CLASS
#define PID_CLASS

#include "Sensor.hpp"
#include "Clamp.hpp"
#include "types.h"

#include <vector>
#include <chrono>
#include <curses.h>

namespace RaspLatte{
  /**
   * PID - Implements a PID controller. The sensor data comes from a Sensor<double> object and is used to set
   * an internal varable u_ (retreived with u()). The gains are provided using an internally defined struct
   * called PIDGains. The following table gives the units of each (E is the error units, U the input, and s is seconds)
   * along with any related settings
   *                                     GAIN | UNITS |    Other Terms
   *                                     =====|=======|==================
   *                                      Kp  |  U/E  |       None
   *                                      Ki  | U/(Es)|   Windup limits
   *                                      Kd  |  Us/E | Slope time range
   *
   * In addition to the gains and their related fields, there is a field for the setpoint (default 0) and input range
   */
  
  class PID{
  private:
    class DIntegral{
      /** A discrete integral class to handle the error sum in a PID controller*/
    public:
      DIntegral(){
	// Starts with a data point at (t0,0)
	prev_time_ = std::chrono::steady_clock::now();
	prev_val_ = 0;
      }
      
      DIntegral(TimePoint t, double v): prev_time_(t), prev_val_(v){}
      
      /** Add a point to the integral. Assume a linear change from the previous v to current*/
      void addPoint(TimePoint t, double v);
      void setClamp(double min, double max);
      double area();
      void resetArea();
      
    private:
      TimePoint prev_time_;
      double prev_val_;
      Clamp<double> clamp_;
      bool clamping_ = false;
      double area_ = 0;
    };

    /**
     * Fits a slope to the data points taken within the last period_ seconds.
     */
    class DDerivative{
    public:
      DDerivative(){
	period_ = Duration(0.001);
      }
      DDerivative(TimePoint t, double v): times_(1,t), vals_(1,v){
	period_ = Duration(0.001);
      }
      
      double addPoint(TimePoint t, double v);
      void setPeriod(double p);
      void reset();
      double slope();
      
    private:
      std::vector<TimePoint> times_;
      std::vector<double> vals_;
      Duration period_;
      double slope_;
      
      void cleanPoints();
      void updateSlope();
    };

    
  public:
    typedef struct PIDGains_{
      double p;
      double i;
      double d;
    } PIDGains;

    // ========================= Constructors =========================
    PID(PIDGains gains, double * setpoint, Sensor<double> * const sensor_ptr);
    
    // ============================ Setters  ===========================
    void setIntegralSumLimits(double min, double max);
    void setInputLimits(double min, double max);
    void setSlopePeriodSec(double period);
    void setGains(PIDGains gains);
    void setMinUpdateTimeSec(double t);
    
    // ======================== Operation ============================
    void reset();
    double update(int feed_forward = 0);

    // ========================= Getters ==============================
    double setpoint();
    double u();
    double errorSum();
    double slope();
    
  private:  
    Sensor<double> * const sensor_;
    PIDGains K_;
    double * setpoint_;
    double prev_setpoint_; // Used to check if value changed
    
    Duration min_t_between_updates_;
    TimePoint last_update_time_;

    DDerivative slope_;
    DIntegral int_sum_;
    
    double u_ = 0;
    Clamp<double> input_clamper_;
  };
}
#endif
