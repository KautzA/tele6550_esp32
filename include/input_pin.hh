
#include <cstdint>

#ifndef INPUT_PIN_HH_
#define INPUT_PIN_HH_

class InputPin {
 public:
  InputPin(uint8_t pin);
  ~InputPin();
  bool getState();
 protected:
 uint8_t pin_;
};

#endif  // INPUT_PIN_HH_