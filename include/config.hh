
#ifndef INCLUDE_CONFIG_HH_
#define INCLUDE_CONFIG_HH_

#include <stdint.h>

namespace tele6550 {
namespace config {

constexpr uint16_t kMqttPort = 1883;
constexpr char* kMqttBroker = "10.1.3.10";

}  // namespace config
}  // namespace tele6550

#endif  // INCLUDE_CONFIG_HH_