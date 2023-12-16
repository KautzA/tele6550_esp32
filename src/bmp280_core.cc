#include "bmp280_core.hh"

namespace bmp280 {

// modified from the example code given in section 3.11.3 of BMP280 data sheet Rev 1.4.
int32_t CalculateTFine(int32_t adc_t_raw, const CalibrationData& calibration) {
  int32_t var1, var2, t_fine;
  var1 = (
    (
      (
        (adc_t_raw >> 3) - (static_cast<int32_t>(calibration.dig_T1) << 1)
      )
    ) * (
      static_cast<int32_t>(calibration.dig_T2)
    )
  ) >> 11;
  var2 = (
    (
      (
        (
          (adc_t_raw >> 4) - (static_cast<int32_t>(calibration.dig_T1))
        ) * (
          (adc_t_raw >> 4) - (static_cast<int32_t>(calibration.dig_T1))
        )
      ) >> 12
    ) * (
      static_cast<int32_t>(calibration.dig_T3)
    )
  ) >> 14;
  t_fine = var1 + var2;
  return t_fine;
}

// modified from the example code given in section 3.11.3 of BMP280 data sheet Rev 1.4.
int32_t CompensateTCentiDegC(int32_t adc_t_raw, const CalibrationData& calibration) {
  int32_t T;
  int32_t t_fine = CalculateTFine(adc_t_raw, calibration);
  T = ((t_fine * 5) + 128) >> 8;
  return T;
}

// modified from the example code given in section 3.11.3 of BMP280 data sheet Rev 1.4.
uint32_t CompensatePPa(int32_t adc_p_raw, int32_t t_fine, const CalibrationData& calibration) {
  int64_t var1, var2, p;
  var1 = (static_cast<int64_t>(t_fine)) - 128000;
  var2 = var1 * var1 * static_cast<int64_t>(calibration.dig_P6);
  var2 = var2 + ((var1*static_cast<int64_t>(calibration.dig_P5))<<17);
  var2 = var2 + ((static_cast<int64_t>(calibration.dig_P4))<<35);
  var1 = ((var1 * var1 * static_cast<int64_t>(calibration.dig_P3))>>8)
         + ((var1 * static_cast<int64_t>(calibration.dig_P2))<<12);
  var1 = ((((static_cast<int64_t>(1))<<47)+var1))*(static_cast<int64_t>(calibration.dig_P1))>>33;
  if (var1 == 0)
  {
    return 0; // avoid exception caused by division by zero
  }
  p = 1048576-adc_p_raw;
  p = (((p<<31)-var2)*3125)/var1;
  var1 = ((static_cast<int64_t>(calibration.dig_P9)) * (p>>13) * (p>>13)) >> 25;
  var2 = ((static_cast<int64_t>(calibration.dig_P8)) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + ((static_cast<int64_t>(calibration.dig_P7))<<4);
  return static_cast<uint32_t>(p);
}

}  // namespace bmp280
