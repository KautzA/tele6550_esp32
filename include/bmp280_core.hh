
#ifndef BMP280_CORE_HH_
#define BMP280_CORE_HH_

#include <stdint.h>

/* class IBMP280 {
 public:
  virtual float getTemperature();
}; */

namespace bmp280 {

namespace regmap {
constexpr uint8_t kTempXlsb  = 0xFC;
constexpr uint8_t kTempLsb   = 0xFB;
constexpr uint8_t kTempMsb   = 0xFA;
constexpr uint8_t kPressXlsb = 0xF9;
constexpr uint8_t kPressLsb  = 0xF8;
constexpr uint8_t kPressMsb  = 0xF7;
constexpr uint8_t kConfig    = 0xF5;
constexpr uint8_t kCtrlMeas  = 0xF4;
constexpr uint8_t kStatus    = 0xF3;
constexpr uint8_t kReset     = 0xE0;
constexpr uint8_t kId        = 0xD0;
constexpr uint8_t kCalib00   = 0x88;
} // namespace regmap

#pragma pack(push, 1)
struct CalibrationData {
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;
  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;
};
#pragma pack(pop)

int32_t CalculateTFine(int32_t adc_t_raw, const CalibrationData& calibration);
int32_t CompensateTCentiDegC(int32_t adc_t_raw, const CalibrationData& calibration);
uint32_t CompensatePPa(int32_t adc_p_raw, int32_t t_fine, const CalibrationData& calibration);

}  // namespace bmp280

#endif  // BMP280_CORE_HH_