#ifndef PID_CLASS
#define PID_CLASS

#include "Sensor.hpp"

#include <vector>
#include <chrono>

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

  class Clamper{
  public:
    Clamper(){}
    Clamper(double min, double max): min_(min), max_(max){}    
    void clamp(double & num){
      if(num < min_) num = min_;
      else if(num > max_) num = max_;
    }
    
  private:
    double min_ = -1000000;
    double max_ = 1000000;
  };
  
  class PID{
  private:
    typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>> TimePoint;
    typedef std::chrono::duration<double> Duration;
    
    class DIntegral{
      /** A discrete integral class to handle the error sum in a PID controller*/
    public:
      DIntegral(){
	prev_time_ = std::chrono::steady_clock::now();
	prev_val_ = 0;
      }
      
      DIntegral(TimePoint t, double v): prev_time_(t), prev_val_(v){}
      
      /** Add a point to the integral. Assume a linear change from the previous v to current*/
      void addPoint(TimePoint t, double v){
	Duration delta_t = t - prev_time_;
	double avg_v = (v+prev_val_)/2.0;

	area_ += (delta_t.count() * avg_v);

	if(clamping_) clamp_.clamp(area_);
	
	prev_val_ = v;
	prev_time_ = t;
      }

      void applyClamp(double min, double max){
	clamping_ = true;
	clamp_ = Clamper(min, max);
	clamp_.clamp(area_);
      }

      double area(){
	return area_;
      }
      void resetArea(){
	area_ = 0;
      }
    private:
      TimePoint prev_time_;
      double prev_val_;
      Clamper clamp_;
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
      
      double addPoint(TimePoint t, double v){
	vals_.insert(vals_.begin(), v);
	times_.insert(times_.begin(), t);
	
	// Dump points older than the period
	cleanPoints();
	updateSlope();
	return slope_;
      }

      void setPeriod(double p){
	if (p<0) period_ = Duration(0);
	else period_ = Duration(p);
	updateSlope();
      }

      void reset(){
	vals_ = std::vector<double>();
        times_ = std::vector<TimePoint>();
	slope_ = 0;
      }
      
      double slope(){ return slope_; }
    private:
      std::vector<TimePoint> times_;
      std::vector<double> vals_;
      Duration period_;
      double slope_;
      
      void cleanPoints(){
	while (times_.front() - times_.back() > period_){
	  times_.pop_back();
	  vals_.pop_back();
	}
      }

      void updateSlope(){
	// Can't get slope off one point.
	if (vals_.size() <= 1){
	  slope_ = 0;
	  return;
	}

	// Find the average error and time
	double avg_err = 0;
	Duration avg_t(0);
	for(unsigned int i = 0; i<vals_.size(); i++){
	  avg_err += vals_[i];
	  avg_t += times_[i].time_since_epoch();
	}
      
	avg_err /= vals_.size();
	avg_t /= vals_.size();
	
	//Find and return the slope
	double num = 0;
	double dem = 0;
	for(unsigned int i = 0; i<vals_.size(); i++){
	  double sqrt_dem = (times_[i].time_since_epoch() - avg_t).count();
	  num += sqrt_dem * (vals_[i] - avg_err);
	  dem +=  sqrt_dem * sqrt_dem;
	}
	
	slope_ = num/dem;
      }
    };

    
  public:
    typedef struct PIDGains_{
      double p;
      double i;
      double d;
    } PIDGains;


    // ========================= Constructors =========================
    PID(Sensor<double> * sensor_ptr, PIDGains gains, double set_point = 0): sensor_(sensor_ptr), K_(gains), set_point_(set_point){
      min_t_between_updates_ = Duration(0.001);
      
      last_update_time_ = std::chrono::steady_clock::now();
      
      // Init the slope and integral terms
      double err = sensor_->read()-set_point_;
      slope_ = DDerivative(last_update_time_, err);
      int_sum_ = DIntegral(last_update_time_, err);
    }

    // ============================ Set up ===========================
    void setIntegralSumLimits(double min, double max){
      int_sum_.applyClamp(min, max);
    }
    void setInputLimits(double min, double max){
      if (min > max) input_clamper_ = Clamper(0,0);
      else input_clamper_ = Clamper(min, max); 
    }
    void setSlopePeriodSec(double period){
      slope_.setPeriod(period);
    }
    void setSetpoint(double set_point){
      set_point_ = set_point;
      int_sum_.resetArea();
      slope_.reset();
    }
    void setGains(PIDGains gains){
      K_ = gains;
    }
    void setP(double Kp){
      K_.p = Kp;
    }
    void setI(double Ki){
      K_.i = Ki;
    }
    void setD(double Kd){
      K_.d = Kd;
    }
    void setMinUpdateTimeSec(double t){
      min_t_between_updates_ = Duration(t);
    }
    
    // ======================== Operation ============================
    double update(){
      TimePoint current_time = std::chrono::steady_clock::now();
      if (current_time - last_update_time_ < min_t_between_updates_) return u_;

      last_update_time_ = current_time;
      double err = sensor_->read()-set_point_;
      

      int_sum_.addPoint(current_time, err);
      slope_.addPoint(current_time, err);
     
      u_ = K_.p * err + K_.i * int_sum_.area()+ K_.d * slope_.slope();
      input_clamper_.clamp(u_);

      return u_;
    }

    double u(){
      return u_;
    }
    
  private:  
    Sensor<double> * sensor_;
    PIDGains K_;
    double set_point_;

    Duration min_t_between_updates_;
    TimePoint last_update_time_;

    DDerivative slope_;
    DIntegral int_sum_;
    
    double u_ = 0;
    Clamper input_clamper_;
  };
}
#endif
