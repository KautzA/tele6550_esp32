#include "mqtt_client_obj.hh"

#include "mqtt_client.h"

#include "secrets.hh"
#include <iostream>

// MQTT Code is modified from https://github.com/espressif/esp-idf/blob/692c1fcc52b9b936c73dead4ef0c2ea1fbdfb602/examples/protocols/mqtt/tcp/main/app_main.c
static void log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    std::cout << "Error: " << message << " Code: " << error_code << std::endl;
  }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    std::cout << "Event dispatched from event loop base=" << base << ", event_id=" << event_id<< std::endl;
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        std::cout << "MQTT_EVENT_CONNECTED" << std::endl;
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        std::cout << "sent publish successful, msg_id=" << msg_id << std::endl;

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        std::cout << "sent subscribe successful, msg_id=" << msg_id << std::endl;

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        std::cout << "sent subscribe successful, msg_id=" << msg_id << std::endl;

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        std::cout << "sent unsubscribe successful, msg_id=" << msg_id << std::endl;
        break;
    case MQTT_EVENT_DISCONNECTED:
        std::cout << "MQTT_EVENT_DISCONNECTED" << std::endl;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        std::cout << "MQTT_EVENT_SUBSCRIBED, msg_id=" << event->msg_id << std::endl;
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        std::cout << "sent publish successful, msg_id=" << msg_id << std::endl;
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        std::cout << "MQTT_EVENT_UNSUBSCRIBED, msg_id=" << event->msg_id << std::endl;
        break;
    case MQTT_EVENT_PUBLISHED:
        std::cout << "MQTT_EVENT_PUBLISHED, msg_id=" << event->msg_id << std::endl;
        break;
    case MQTT_EVENT_DATA:
        std::cout << "MQTT_EVENT_DATA" << std::endl;
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        std::cout << "MQTT_EVENT_ERROR" << std::endl;
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            std::cout << "Last errno string (" << strerror(event->error_handle->esp_transport_sock_errno) << ")" << std::endl;

        }
        break;
    default:
        std::cout << "Other event id:" << event->event_id << std::endl;
        break;
    }
}


MqttClient::MqttClient() {
  esp_mqtt_client_config_t mqtt_cfg = {
    .broker = {.address = {.uri = tele6550::secrets::broker_url},},
    .credentials = {
      .username = tele6550::secrets::broker_username,
      .authentication = {
        .password = tele6550::secrets::broker_password,
      }
    },
  };
  client_ = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(
    static_cast<esp_mqtt_client_handle_t>(client_),
    static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID),
    mqtt_event_handler, NULL);
  esp_mqtt_client_start(static_cast<esp_mqtt_client_handle_t>(client_));
}

void MqttClient::sendMessage(const std::string& topic, const std::string& payload) {
  std::cout << "sending message to " << topic << " payload: " << payload << std::endl;
  esp_mqtt_client_publish(
    static_cast<esp_mqtt_client_handle_t>(client_),
    topic.c_str(),
    payload.c_str(),
    payload.size(),
    1,
    0
  );
}

void MqttClient::subscribe(
    const std::string& topic,
    std::function<void(const std::string&, const std::string&)> callback
    ) {
  return;
}
