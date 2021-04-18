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

      void applyClamp(double min, double max){
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
    PID(Sensor<double> * sensor_ptr, PIDGains gains, double setpoint = 0): sensor_(sensor_ptr), K_(gains), setpoint_(setpoint){
      min_t_between_updates_ = Duration(0.001);
      
      last_update_time_ = std::chrono::steady_clock::now();
      
      // Init the slope and integral terms
      double err = sensor_->read()-setpoint_;
      slope_ = DDerivative(last_update_time_, err);
      int_sum_ = DIntegral(last_update_time_, err);
      
      input_clamper_.setMin(0);
      input_clamper_.setMax(255);
    }

    // ============================ Set up ===========================
    void setIntegralSumLimits(double min, double max){
      int_sum_.applyClamp(min, max);
    }
    
    void setInputLimits(double min, double max){
      input_clamper_.setMin(min);
      input_clamper_.setMax(max);
    }
    
    void setSlopePeriodSec(double period){
      slope_.setPeriod(period);
    }
    
    double setpoint(){ return setpoint_; }
    
    void setSetpoint(double setpoint){
      setpoint_ = setpoint;
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
      double err = sensor_->read()-setpoint_;
      

      int_sum_.addPoint(current_time, err);
      slope_.addPoint(current_time, err);

      u_ = K_.p * err + K_.i * int_sum_.area()+ K_.d * slope_.slope();

      return input_clamper_.clamp(u_);
    }

    double u(){
      return u_;
    }

    double iSum(){
      return int_sum_.area();
    }
    double slope(){
      return slope_.slope();
    }
    
    void printStatusIn(WINDOW * win){
      int h, w;
      getmaxyx(win, h, w);
      if(h< 16 || w< 20) throw "Window to samll for PID Panel";
      wclear(win);
      wborder(win, '|', '|', '=','=','|','|','|','|');
      mvwprintw(win, 1, w/2-7, "PID Controller");
      mvhline(2, 1, '-', w-2);
      mvwprintw(win, 3, w/2-7, "Tracking Data");
      mvwprintw(win, 4, 2, "> Setpoint = %0.2f", setpoint_);
      mvwprintw(win, 5, 2, "> Current  = %0.2f", sensor_->read());
      mvwprintw(win, 6, 2, "> Error    = %0.2f", setpoint_ - sensor_->read());
      mvwprintw(win, 7, 2, "> Output   = %d", setpoint_ - u_);
      mvhline(8, 1, '-', w-2);
      mvwprintw(win, 9, w/2-7, "Tracking Data");
      mvwprintw(win, 10, 2, "> Kp    = %0.2f", K_.p);
      mvwprintw(win, 11, 2, "> Ki    = %0.2f", K_.i);
      mvwprintw(win, 12, 2, "> Kd    = %0.2f", K_.d);
      mvwprintw(win, 13, 2, "> Area  = %0.2f", int_sum_.area());
      mvwprintw(win, 14, 2, "> Slope = %0.2f", slope_.slope());
      wrefresh(win);
    }
    
  private:  
    Sensor<double> * sensor_;
    PIDGains K_;
    double setpoint_;

    Duration min_t_between_updates_;
    TimePoint last_update_time_;

    DDerivative slope_;
    DIntegral int_sum_;
    
    double u_ = 0;
    Clamp<double> input_clamper_;
  };
}
#endif


/*         1         2 
 012345678901234567890123456789
 |============================|0
 |       PID Controller       |1
 |----------------------------|2
 |       Tracking Data        |3
 | > Setpoint = 140.00        |4
 | > Current  = 134.75        |5
 | > Error    =   5.25        |6
 | > Output   = 234           |
 |----------------------------|7
 |       Internal Data        |8
 | > Kp    =   15.00          |9
 | > Ki    =    0.01          |0 1
 | > Kd    =  100000          |1
 | > Sum   = -000.00          |2
 | > Slope =    5.43          |3
 |============================|4

 */
