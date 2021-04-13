#ifndef PID_CLASS
#define PID_CLASS

#include "Sensor.hpp"

#include <vector>
#include <chrono>
#include <ctime>
#include <iostream>

namespace RaspLatte{
  class PID{
  public:
    typedef struct PIDGains_{
      double p;
      double i;
      double d;
    } PIDGains;

    PID(Sensor<double> * sensor_ptr, PIDGains gains, double set_point = 0): sensor_(sensor_ptr), K(gains), set_point_(set_point){
      slope_period_ = 1000; //microseconds
      start_time_ = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      std::cout<<start_time_<<std::endl;
      previous_times_.push_back(start_time_);
      previous_errs_.push_back(sensor_->read()-set_point_);
    }

    void update(){
      double err = sensor_->read()-set_point_;
      unsigned long current_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

      //Get the integral term
      if (!previous_times_.empty()){
	integral_sum_ += err * (current_time - previous_times_[0])/1000000;
      }

      //Remove all old records
      while (!previous_times_.empty() && current_time-previous_times_.back() > slope_period_){
	previous_times_.pop_back();
	previous_errs_.pop_back();
      }
      
      //Save the new value and time
      previous_times_.insert(previous_times_.begin(), current_time);
      previous_errs_.insert(previous_errs_.begin(), err);

      double slope = getSlope();

      u_ = err * K.p + integral_sum_ * K.i + slope * K.d;
      //std::cout<<"Time :"<<current_time<<"\nError: "<<err<<"\nIntegral Sum: "<<integral_sum_<<"\nSlope: "<<slope<<"\nSlope Count: "<<previous_times_.size()<<std::endl;
    }

    double u(){ return u_; }
    
    void updateSetpoint(double set_point){
      set_point_ = set_point;
      previous_errs_ = std::vector<double>();
      integral_sum_ = 0;
       previous_times_.push_back(start_time_);
       previous_errs_.push_back(sensor_->read()-set_point_);
    }
    
  private: 
    Sensor<double> * sensor_;
    PIDGains K;
    double set_point_;

    std::vector<double> previous_errs_;
    std::vector<unsigned long> previous_times_;
    unsigned long start_time_; // micro sec
    unsigned long slope_period_; // microsec
    double integral_sum_ = 0;

    double u_ = 0;
    double getSlope(){
      // Can't get slope off one point.
      if (previous_errs_.size() <= 1) return 0;

      double avg_err = 0;
      double avg_t = 0;

      for(unsigned int i = 0; i<previous_errs_.size(); i++){
	avg_err += previous_errs_[i];
	avg_t += previous_times_[i];
      }
      
      avg_err /= previous_errs_.size();
      avg_t /= previous_errs_.size();

      double num = 0;
      double dem = 0;
      for(unsigned int i = 0; i<previous_errs_.size(); i++){
	num += (previous_times_[i] - avg_t) * (previous_errs_[i] - avg_err);
	dem += (previous_times_[i] - avg_t) * (previous_times_[i] - avg_t);
      }
    return num/(dem*100000);
    }
  };
}
#endif
