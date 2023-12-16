
#include <cstdint>

#ifndef PWM_PIN_HH_
#define PWM_PIN_HH_

class PwmPin {
 public:
  PwmPin(uint8_t pin, uint8_t channel);
  ~PwmPin();
  void setState(bool assert_pin);
  void setDutyCycle(uint8_t level);
 protected:
 uint8_t pin_;
 uint8_t channel_;
};

#endif  // PWM_PIN_HH_