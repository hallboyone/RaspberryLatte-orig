#ifndef PID_CLASS
#define PID_CLASS

#include "Sensor.hpp"
#include "Clamp.hpp"
#include "types.h"

#include <vector>
#include <iostream>
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
      void addPoint(TimePoint t, double v){
	Duration delta_t = prev_time_ - t;
	double avg_v = (v+prev_val_)/2.0;

	area_ += (delta_t.count() * avg_v);

	if(clamping_) clamp_.clamp(area_);
	
	prev_val_ = v;
	prev_time_ = t;
      }

      void setClamp(double min, double max){
	clamping_ = true;
	clamp_.setMin(min);
	clamp_.setMax(max);
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
      double num(){return num_;}
      double den(){return den_;}
    private:
      std::vector<TimePoint> times_;
      std::vector<double> vals_;
      Duration period_;
      double slope_;
      double num_ = 0;
      double den_ = 1;
      
      void cleanPoints(){
	while (times_.size() > 10) {//times_.front() - times_.back() > period_){
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
	double avg_err = vals_[0];
	Duration avg_t = times_[0].time_since_epoch();
	for(unsigned int i = 1; i<vals_.size(); i++){
	  avg_err += vals_[i];
	  avg_t += times_[i].time_since_epoch();
	}
      
	avg_err /= vals_.size();
	avg_t /= vals_.size();
	
	//Find and return the slope
	num_ = 0;
	den_ = 0;
	for(unsigned int i = 0; i<vals_.size(); i++){
	  double sqrt_den = (times_[i].time_since_epoch() - avg_t).count();
	  num_ += sqrt_den * (vals_[i] - avg_err);
	  den_ +=  sqrt_den * sqrt_den;
	}
	
	slope_ = num_/den_;
      }
    };

    
  public:
    typedef struct PIDGains_{
      double p;
      double i;
      double d;
    } PIDGains;

    // ========================= Constructors =========================
    PID(PIDGains gains, double * setpoint, Sensor<double> * sensor_ptr): sensor_(sensor_ptr), K_(gains), setpoint_(setpoint){
      // Default settings
      min_t_between_updates_ = Duration(0.001);
      slope_.setPeriod(2);
      input_clamper_.setMin(0);
      input_clamper_.setMax(255);
      
      last_update_time_ = std::chrono::steady_clock::now();

      // Init the slope and integral terms
      double err = sensor_->read()- *setpoint_;
      slope_ = DDerivative(last_update_time_, err);
      int_sum_ = DIntegral(last_update_time_, err);
      
      prev_setpoint_ = *setpoint;
    }

    // ============================ Set up ===========================
    void setIntegralSumLimits(double min, double max){
      int_sum_.setClamp(min, max);
    }
    
    void setInputLimits(double min, double max){
      input_clamper_.setMin(min);
      input_clamper_.setMax(max);
    }
    
    void setSlopePeriodSec(double period){
      slope_.setPeriod(period);
    }
    
    double setpoint(){ return *setpoint_; }
     
    void setGains(PIDGains gains){
      K_ = gains;
    }
    
    void setMinUpdateTimeSec(double t){
      min_t_between_updates_ = Duration(t);
    }
    
    // ======================== Operation ============================
    void reset(){
      // Reset slope and integral terms
      int_sum_.resetArea();
      slope_.reset();

      last_update_time_ = std::chrono::steady_clock::now();
      
      // Init the slope and integral terms
      double err = sensor_->read()- *setpoint_;
      slope_ = DDerivative(last_update_time_, err);
      int_sum_ = DIntegral(last_update_time_, err);
    }
    
    double update(){
      TimePoint current_time = std::chrono::steady_clock::now();
      if (current_time - last_update_time_ < min_t_between_updates_) return u_;

      last_update_time_ = current_time;

      // If setpoint changed, reset internal variables
      if (*setpoint_ != prev_setpoint_){
	prev_setpoint_ = *setpoint_;
	//slope_.reset();
      }
      
      double err = sensor_->read() - *setpoint_;

      int_sum_.addPoint(current_time, err);
      slope_.addPoint(current_time, err);

      u_ = K_.p * err + K_.i * int_sum_.area()+ K_.d * slope_.slope();

      return input_clamper_.clamp(u_);
    }

    double u(){
      return u_;
    }

    double errorSum(){
      return int_sum_.area();
    }
    
    double slope(){
      return slope_.slope();
    }

    void updateStatusWin(WINDOW * win, bool init = true){
      if (init){
	// Clear, border and title
	wclear(win);
	wborder(win, '#', '#', '-','=','#','#','#','#');
	mvwaddstr(win, 0, 34, " PID Status ");

	// PWM output line
	mvwprintw(win, 1, 32, "PWM Output - %0.0f  ", u_);
	mvwprintw(win, 3, 7, "Setpoint - %0.2f    ", *setpoint_);
	mvwprintw(win, 3, 32, "Current - %0.2f    ", sensor_->read());
	mvwprintw(win, 3, 58, "Error - %0.2f    ", sensor_->read() - *setpoint_);
	mvwprintw(win, 4, 7, "Error Sum - %0.2f    ", int_sum_.area());
	mvwprintw(win, 4, 32, "Slope - %0.2f    ", slope_.slope());
      }
      else {
	mvwprintw(win, 1, 45, "%0.0f   ", u_);
	mvwprintw(win, 3, 18, "%0.2f    ", *setpoint_);
	mvwprintw(win, 3, 42, "%0.2f    ", sensor_->read());
	mvwprintw(win, 3, 66, "%0.2f    ", sensor_->read() - *setpoint_);
	mvwprintw(win, 4, 19, "%0.2f    ", int_sum_.area());
	mvwprintw(win, 4, 40, "%0.2f    ", slope_.slope());
      }
      
      wrefresh(win);
    }
  private:  
    Sensor<double> * sensor_;
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


/*        1         2         3         4         5         6         7 
01234567890123456789012345678901234567890123456789012345678901234567890123456789
#--------------------------------- PID Status ---------------------------------# 0
#                               PWM Output - 234                               # 1
#                                                                              # 2
#      Setpoint - 140.00        Current - 134.75          Error - 5.25         # 3
#      Error Sum -              Slope -                                        # 4
#==============================================================================# 5

 */
