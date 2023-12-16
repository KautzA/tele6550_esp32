
#include <mqtt_client.h>

#include <cstdint>
#include <string>
#include <functional>

#ifndef MQTT_CLIENT_OBJ_HH_
#define MQTT_CLIENT_OBJ_HH_

class MqttClient {
 public:
  MqttClient();
  void sendMessage(const std::string& topic, const std::string& payload);
  void subscribe(
    const std::string& topic,
    std::function<void(const std::string&, const std::string&)> callback
    );
 protected:
  void* client_;
};

#endif  // MQTT_CLIENT_OBJ_HH_