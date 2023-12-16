#include "pwm_pin.hh"
#include "driver/ledc.h"
#include <iostream>

constexpr ledc_mode_t led_mode = LEDC_HIGH_SPEED_MODE;
constexpr ledc_timer_t led_timer = LEDC_TIMER_0;

PwmPin::PwmPin(uint8_t pin, uint8_t channel) : pin_(pin), channel_(channel) {
  ledc_timer_config_t tconfig = {
    .speed_mode = led_mode,
    .duty_resolution = LEDC_TIMER_8_BIT,
    .timer_num = led_timer,
    .freq_hz = 1000,
    .clk_cfg = LEDC_USE_APB_CLK,
  };
  esp_err_t error = ledc_timer_config(&tconfig);
  if (error != ESP_OK) {
    std::cout << "LEDC timer config error: " << esp_err_to_name(error) << std::endl;
  }

  ledc_channel_config_t cconfig = {
    .gpio_num = static_cast<gpio_num_t>(pin),
    .speed_mode = led_mode,
    .channel = static_cast<ledc_channel_t>(channel),
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = led_timer,
    .duty = 127,
    .hpoint = 0,
    .flags = {.output_invert = 0},
  };

  error = ledc_channel_config(&cconfig);
  if (error != ESP_OK) {
    std::cout << "LEDc channel config error: " << esp_err_to_name(error) << std::endl;
  }
}

void PwmPin::setState(bool assert_pin) {
  setDutyCycle(assert_pin ? 254 : 1);
}

void PwmPin::setDutyCycle(uint8_t level) {
  esp_err_t error = ledc_set_duty(
    led_mode,
    static_cast<ledc_channel_t>(channel_),
    level / 2);
  if (error != ESP_OK) {
    std::cout << "LEDc set duty error: " << esp_err_to_name(error) << std::endl;
  }
  error = ledc_update_duty(led_mode, static_cast<ledc_channel_t>(channel_));
  if (error != ESP_OK) {
    std::cout << "LEDc update duty error: " << esp_err_to_name(error) << std::endl;
  }
  
};