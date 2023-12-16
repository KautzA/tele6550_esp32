
#ifndef BMP280_I2C_HH_
#define BMP280_I2C_HH_

#include "bmp280_core.hh"

class BMP280I2C {
 public:
  BMP280I2C();
  ~BMP280I2C();
  virtual float getTemperature();
 private:
  bmp280::CalibrationData cal_;
};

#endif  // BMP280_I2C_HH_