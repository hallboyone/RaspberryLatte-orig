#include "../../include/RaspberryLatte/PID.hpp"
namespace RaspLatte{   
  /** Add a point to the integral. Assume a linear change from the previous v to current*/
  void PID::DIntegral::addPoint(TimePoint t, double v){
    Duration delta_t = t - prev_time_;
    double avg_v = (v+prev_val_)/2.0;

    area_ += (delta_t.count() * avg_v);

    if(clamping_) clamp_.clamp(area_);
	
    prev_val_ = v;
    prev_time_ = t;
  }

  void PID::DIntegral::setClamp(double min, double max){
    clamping_ = true;
    clamp_.setMin(min);
    clamp_.setMax(max);
    clamp_.clamp(area_);
  }

  double PID::DIntegral::area(){
    return area_;
  }
      
  void PID::DIntegral::resetArea(){
    area_ = 0;
  }
  
  /**
   * DDerivative implementation
   */   
  double PID::DDerivative::addPoint(TimePoint t, double v){
    vals_.insert(vals_.begin(), v);
    times_.insert(times_.begin(), t);
	
    // Dump points older than the period
    cleanPoints();
    updateSlope();
    return slope_;
  }

  void PID::DDerivative::setPeriod(double p){
    if (p<0) period_ = Duration(0);
    else period_ = Duration(p);
    updateSlope();
  }

  void PID::DDerivative::reset(){
    vals_ = std::vector<double>();
    times_ = std::vector<TimePoint>();
    slope_ = 0;
  }
      
  double PID::DDerivative::slope(){ return slope_; }
  
  void PID::DDerivative::cleanPoints(){
    while (times_.size() > 10) {//times_.front() - times_.back() > period_){
      times_.pop_back();
      vals_.pop_back();
    }
  }

  void PID::DDerivative::updateSlope(){
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
    double num = 0;
    double den = 0;
    for(unsigned int i = 0; i<vals_.size(); i++){
      double sqrt_den = (times_[i].time_since_epoch() - avg_t).count();
      num += sqrt_den * (vals_[i] - avg_err);
      den +=  sqrt_den * sqrt_den;
    }
	
    slope_ = num/den;
  }

  // ========================= Constructors =========================
  PID::PID(PIDGains gains, double * setpoint, Sensor<double> * const sensor_ptr): sensor_(sensor_ptr), K_(gains), setpoint_(setpoint){
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
  void PID::setIntegralSumLimits(double min, double max){
    int_sum_.setClamp(min, max);
  }
    
  void PID::setInputLimits(double min, double max){
    input_clamper_.setMin(min);
    input_clamper_.setMax(max);
  }
    
  void PID::setSlopePeriodSec(double period){
    slope_.setPeriod(period);
  }
    
  double PID::setpoint(){ return *setpoint_; }
     
  void PID::setGains(PIDGains gains){
    K_ = gains;
  }
    
  void PID::setMinUpdateTimeSec(double t){
    min_t_between_updates_ = Duration(t);
  }
    
  // ======================== Operation ============================
  void PID::reset(){
    // Reset slope and integral terms
    int_sum_.resetArea();
    slope_.reset();

    last_update_time_ = std::chrono::steady_clock::now();
      
    // Init the slope and integral terms
    double err = sensor_->read()- *setpoint_;
    slope_.addPoint(last_update_time_, err);
    int_sum_.addPoint(last_update_time_, err);
  }
    
  double PID::update(int feed_forward){
    TimePoint current_time = std::chrono::steady_clock::now();
    if (current_time - last_update_time_ < min_t_between_updates_) return u_;

    last_update_time_ = current_time;

    // If setpoint changed, reset internal variables
    if (*setpoint_ != prev_setpoint_){
      prev_setpoint_ = *setpoint_;
      //slope_.reset();
    }
      
    double err = *setpoint_ - sensor_->read();

    int_sum_.addPoint(current_time, err);
    slope_.addPoint(current_time, err);

    u_ = K_.p * err + K_.i * int_sum_.area()+ K_.d * slope_.slope() + feed_forward;

    return input_clamper_.clamp(u_);
  }

  double PID::u(){
    return u_;
  }

  double PID::errorSum(){
    return int_sum_.area();
  }
    
  double PID::slope(){
    return slope_.slope();
  }
}

/*        1         2         3         4         5         6         7 
01234567890123456789012345678901234567890123456789012345678901234567890123456789
#--------------------------------- PID Status ---------------------------------# 0
#                               PWM Output - 234                               # 1
#                                                                              # 2
#      Setpoint - 140.00        Current - 134.75          Error - 5.25         # 3
#      Error Sum -              Slope -                                        # 4
#==============================================================================# 5

 */
