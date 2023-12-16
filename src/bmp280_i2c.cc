
#include "bmp280_i2c.hh"

#include <stdint.h>
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <driver/i2c.h>

constexpr uint8_t kBnp280Addr = 0x76;
constexpr uint8_t kBnp280ConfTempOversample = 0x01 & 0x07;
constexpr uint8_t kBnp280ConfPresStandard = 0x03 & 0x07;
constexpr uint8_t kBnp280ConfMode = 3;
constexpr uint8_t kBnp280ModeTSb = 0x00 & 0x07;  // 0x00 = 0.5ms
constexpr uint8_t kBnp280ModeFilter = 0x00 & 0x07;  // 0x00 = no filter
constexpr i2c_port_t kBnp280i2cPort = I2C_NUM_1;
constexpr bool INVERT_BYTE_ORDER = false;


// i2c setup, read and write based on https://github.com/espressif/esp-idf/blob/v4.3.6/examples/peripherals/i2c/i2c_self_test/main/i2c_example_main.c
void setup_i2c() {
  i2c_config_t i2cConfig = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = 21,
    .scl_io_num = 22,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master = {.clk_speed = 50000},
    .clk_flags = 0,
  };
  bool ok = true;
  esp_err_t error = i2c_param_config(kBnp280i2cPort, &i2cConfig);
  if (error != ESP_OK) {
    printf("I2C Param Config Failed: %s\n", esp_err_to_name(error));
    ok = false;
  }
  error = i2c_driver_install(kBnp280i2cPort, i2cConfig.mode, 0, 0, 0);
  if (error != ESP_OK) {
    printf("I2C Driver Installed Failed: %s\n", esp_err_to_name(error));
    ok = false;
  }
  printf("I2C setup complete %d\n", ok);
}

void write(i2c_port_t i2c_num, const void* buffer, size_t size) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (kBnp280Addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write(cmd, static_cast<const uint8_t*>(buffer), size, true);
  i2c_master_stop(cmd);
  esp_err_t error = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  if (error != ESP_OK) {
    printf("I2C Write Failed: %s\n", esp_err_to_name(error));
  }
}

void read(i2c_port_t i2c_num, void* buffer, size_t size) {
  if (size == 0) {
    printf("I2C Write called with size 0");
    return;
  }
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (kBnp280Addr << 1) | I2C_MASTER_READ, true);
  if (size > 1) {
    i2c_master_read(cmd, static_cast<uint8_t*>(buffer), size -1, I2C_MASTER_ACK);
  }
  i2c_master_read_byte(cmd, static_cast<uint8_t*>(buffer) + size - 1, I2C_MASTER_LAST_NACK);
  i2c_master_stop(cmd);
  esp_err_t error = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  if (error != ESP_OK) {
    printf("I2C Read Failed: %s\n", esp_err_to_name(error));
  }

}

BMP280I2C::BMP280I2C() {
  setup_i2c();

  unsigned char buffer[0x20 + sizeof(cal_)];

  // Check device
  buffer[0] = bmp280::regmap::kId;
  write(kBnp280i2cPort, buffer, 0x01);
  read(kBnp280i2cPort, buffer, 0x01);
  if (buffer[0] != 0x58) {
    printf("BMP280I2C: Buffer read of 0xD0 was %#04x. Expected 0x58\n", buffer[0]);
  }

  // Setup measurement
  buffer[0] = bmp280::regmap::kCtrlMeas;
  buffer[1] = 0b10110111; // (kBnp280ConfTempOversample << 5) | (kBnp280ConfPresStandard << 2) | kBnp280ConfMode;  // ctrl_meas
  buffer[2] = (kBnp280ModeTSb << 5) | (kBnp280ModeFilter << 2);  // mode
  write(kBnp280i2cPort, buffer, 0x03);

  // Read calibration
  buffer[0] = bmp280::regmap::kCalib00;
  cal_.dig_T1 = 0xDEAD;
  cal_.dig_T2 = 0xBEEF;
  cal_.dig_T3 = 0xAA55;
  if (INVERT_BYTE_ORDER) {
    write(kBnp280i2cPort, buffer, 0x01);
    read(kBnp280i2cPort, buffer, sizeof(cal_));
    uint8_t* calptr = reinterpret_cast<uint8_t*>(&cal_);
    for (size_t i = 0; i < sizeof(cal_); i += 2) {
      calptr[i] = buffer[i+1];
      calptr[i+1] = buffer[i];
    }
  } else {
    write(kBnp280i2cPort, buffer, 0x01);
    read(kBnp280i2cPort, &cal_, sizeof(cal_));
    /* read(kBnp280i2cPort, buffer, sizeof(cal_));
    cal_.dig_T1 = (buffer[1] << 8) | buffer[0];
    cal_.dig_T2 = (buffer[3] << 8) | buffer[2];
    cal_.dig_T3 = (buffer[5] << 8) | buffer[4]; */
  }

  printf("BMP280I2C: Temperature calibration values T1: %d, T2: %d, T3: %d\n", cal_.dig_T1, cal_.dig_T2, cal_.dig_T3);
}

float BMP280I2C::getTemperature() {
  unsigned char buffer[0xF];
  buffer[0] = bmp280::regmap::kTempMsb;
  write(kBnp280i2cPort, buffer, 0x01);
  read(kBnp280i2cPort, buffer, 0x03);
  uint32_t t_raw = (buffer[0] << 12) | (buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);
  uint32_t temperature = bmp280::CompensateTCentiDegC(t_raw, cal_);
  return static_cast<float>(temperature) / 100.0;
}
