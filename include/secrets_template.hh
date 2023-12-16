// This file is a template and should be filled out and renamed to secrets.hh

#include <string>
#ifndef INCLUDE_SECRETS_HH_
#define INCLUDE_SECRETS_HH_

#define CONFIG_ESP_WIFI_SSID "SSID Here"
#define CONFIG_ESP_WIFI_PSK "Password"

namespace tele6550 {
namespace secrets {
constexpr char* broker_url = "mqtt://10.1.0.51:1883";
constexpr char* broker_username = "brokerusername";
constexpr char* broker_password = "brokerpassword";
}
}

#endif  // INCLUDE_SECRETS_HH_