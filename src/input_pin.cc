#include "input_pin.hh"
#include "driver/gpio.h"
#include <iostream>

InputPin::InputPin(uint8_t pin) : pin_(pin) {
  gpio_config_t config = {
    .pin_bit_mask = 0x01ul << pin,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };

  esp_err_t error = gpio_config(&config);
  if (error != ESP_OK) {
    std::cout << "GPIO config error: " << esp_err_to_name(error) << std::endl;
  }
}

bool InputPin::getState() {
  return gpio_get_level(static_cast<gpio_num_t>(pin_));
}
