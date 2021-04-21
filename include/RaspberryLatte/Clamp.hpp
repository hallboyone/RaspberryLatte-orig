#ifndef CLAMP
#define CLAMP

namespace RaspLatte{
  template <typename T>
  class Clamp{
  public:
    Clamp(): min_(1), max_(-1){}
    Clamp(T min, T max): min_(min), max_(max){}    
    T clamp(T & num){
      if (min_ < max_){
	if(num < min_) num = min_;
	else if(num > max_) num = max_;
      }
      return num;
    }

    void setMin(T min){
      min_ = min;
    }

    void setMax(T max){
      max_ = max;
    }
    
  private:
    T min_;
    T max_;
  };
}
#endif
