
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include <iostream>

#include "bmp280_i2c.hh"
#include "input_pin.hh"
#include "pwm_pin.hh"
#include "secrets.hh"
#include "mqtt_client_obj.hh"

// Wifi setup is modified from https://github.com/espressif/esp-idf/blob/master/examples/wifi/getting_started/station/main/station_example_main.c

void setup_nvs() {
  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

constexpr size_t kMaximumRetry = 10;

static EventGroupHandle_t s_wifi_event_group;
constexpr EventBits_t kWifiConnectedBit = BIT0;
constexpr EventBits_t kWifiFailBit = BIT1;

static size_t s_retry_num = 0;

void event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < kMaximumRetry) {
            esp_wifi_connect();
            s_retry_num++;
            std::cout << "retry to connect to the AP" << std::endl;
        } else {
            xEventGroupSetBits(s_wifi_event_group, kWifiFailBit);
        }
        std::cout << "connect to the AP fail" << std::endl;
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        std::cout << "got ip:" << &event->ip_info.ip << std::endl;
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, kWifiConnectedBit);
    }
}

void setup_wifi() {
  s_wifi_event_group = xEventGroupCreate();
  esp_err_t error = esp_netif_init();
  if (error != ESP_OK)
    std::cout << "WiFi setup: esp_netif_init failed: " << esp_err_to_name(error) << std::endl;

  error = esp_event_loop_create_default();
  if (error != ESP_OK)
    std::cout << "WiFi setup: esp_event_loop_create_default failed: " << esp_err_to_name(error) << std::endl;

  esp_netif_create_default_wifi_sta();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  error = esp_wifi_init(&cfg);
  if (error != ESP_OK)
    std::cout << "WiFi setup: esp_wifi_init failed: " << esp_err_to_name(error) << std::endl;
  
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  error = esp_event_handler_instance_register(
    WIFI_EVENT,
    ESP_EVENT_ANY_ID,
    &event_handler,
    NULL,
    &instance_any_id
  );
  if (error != ESP_OK)
    std::cout << "WiFi setup: esp_handler_instance_register failed: " << esp_err_to_name(error) << std::endl;
  
  error = esp_event_handler_instance_register(
    IP_EVENT,
    IP_EVENT_STA_GOT_IP,
    &event_handler,
    NULL,
    &instance_got_ip
  );
  if (error != ESP_OK)
    std::cout << "WiFi setup: esp_handler_instance_register failed: " << esp_err_to_name(error) << std::endl;

  wifi_config_t wifi_config = {
    .sta = {
      .ssid = CONFIG_ESP_WIFI_SSID,
      .password = CONFIG_ESP_WIFI_PSK,
      .threshold = {.authmode = WIFI_AUTH_WPA2_WPA3_PSK,}
    },
  };
  error = esp_wifi_set_mode(WIFI_MODE_STA);
  if (error != ESP_OK)
    std::cout << "WiFi setup: esp_wifi_set_mode failed: " << esp_err_to_name(error) << std::endl;
  error = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  if (error != ESP_OK)
    std::cout << "WiFi setup: esp_wifi_set_config failed: " << esp_err_to_name(error) << std::endl;
  error = esp_wifi_start();
  if (error != ESP_OK)
    std::cout << "WiFi setup: esp_wifi_start failed: " << esp_err_to_name(error) << std::endl;
  std::cout << "WiFi Started" << std::endl;

  EventBits_t bits = xEventGroupWaitBits(
    s_wifi_event_group,
    kWifiConnectedBit | kWifiFailBit,
    pdFALSE,
    pdFALSE,
    portMAX_DELAY);

  if (bits & kWifiConnectedBit) {
    std::cout << "connected to AP " << CONFIG_ESP_WIFI_SSID << std::endl;
  } else if (bits & kWifiFailBit) {
    std::cout << "Failed to connect to AP " << CONFIG_ESP_WIFI_SSID << std::endl;
  } else {
    std::cout << "Unexpected Event" << std::endl;
  }
}

const char* presence_topic_discovery = "homeassistant/binary_sensor/tele6550/tele6550_presence/config";
const char* presence_topic = "tele6550/tele6550_presence";
const char* presence_discovery_payload = "{"
"\"name\":\"presence\", "
"\"device_class\":\"occupancy\", "
"\"state_topic\":\"tele6550/tele6550_presence\", "
"\"unique_id\":\"tele6550_presence\", "
"\"device\":{"
  "\"name\":\"tele6550\", "
  "\"model\":\"tele6550_esp32\", "
  "\"manufacturer\":\"Arthur Kautz\", "
  "\"identifiers\": [\"tele6550\"]"
"}"
"}";

const char* temperature_topic_discovery = "homeassistant/sensor/tele6550/tele6550_temperature/config";
const char* temperature_topic = "tele6550/tele6550_temperature";
const char* temperature_discovery_payload = "{"
"\"name\":\"temperature\", "
"\"device_class\":\"temperature\", "
"\"state_topic\":\"tele6550/tele6550_temperature\", "
"\"unique_id\":\"tele6550_temperature\", "
"\"device\":{"
  "\"name\":\"tele6550\", "
  "\"model\":\"tele6550_esp32\", "
  "\"manufacturer\":\"Arthur Kautz\", "
  "\"identifiers\": [\"tele6550\"]"
"}"
"}";

const char* indicator_topic_discovery = "homeassistant/light/tele6550/tele6550_indicator/config";
const char* indicator_topic_brightness = "tele6550/tele6550_indicator/brightness";
const char* indicator_topic_command = "tele6550/tele6550_indicator/command";
const char* indicator_discovery_payload = "{"
"\"name\":\"indicator\", "
"\"device_class\":\"light\", "
"\"command_topic\":\"tele6550/tele6550_indicator/command\", "
"\"brightness_command_topic\":\"tele6550/tele6550_indicator/brightness\", "
"\"on_command_type\":\"brightness\", "
"\"unique_id\":\"tele6550_temperature\", "
"\"device\":{"
  "\"name\":\"tele6550\", "
  "\"model\":\"tele6550_esp32\", "
  "\"manufacturer\":\"Arthur Kautz\", "
  "\"identifiers\": [\"tele6550\"]"
"}"
"}";



extern "C" void app_main() {
  BMP280I2C sensor_bmp = BMP280I2C();
  InputPin mmwave_detection = InputPin(14);
  PwmPin led_red = PwmPin(32, 0);
  PwmPin led_gre = PwmPin(33, 1);

  setup_nvs();

  setup_wifi();

  MqttClient mqtt = MqttClient();
  std::string test_topic("test/topic");
  std::string test_payload("testpayload");


  mqtt.sendMessage(std::string(presence_topic_discovery), std::string(presence_discovery_payload));
  mqtt.sendMessage(std::string(temperature_topic_discovery), std::string(temperature_discovery_payload));
  mqtt.sendMessage(std::string(indicator_topic_discovery), std::string(indicator_discovery_payload));

  mqtt.subscribe(std::string(indicator_topic_command), [&led_gre](std::string t, std::string m) {
    std::cout << "Indicator Command callback" << std::endl;
    led_gre.setDutyCycle(0);
  });
  mqtt.subscribe(std::string(indicator_topic_brightness), [&led_gre](std::string t, std::string m) {
    std::cout << "Indicator Brightness callback" << std::endl;
    uint8_t val = std::stoi(m);
    led_gre.setDutyCycle(val);
  });

  uint8_t counter = 0;
  while(1) {
    std::cout << std::endl << "testoutput" << std::endl;
    float temp = sensor_bmp.getTemperature();
    std::cout << "Temperature: " << temp << std::endl;
    bool presence = mmwave_detection.getState();
    std::cout << "MMWave: " << presence << std::endl;
    counter++;
    std::cout << "PWM: " << static_cast<int>(counter) << std::endl;
    led_red.setDutyCycle(counter);
    mqtt.sendMessage(std::string(presence_topic), std::string(presence ? "ON" : "OFF"));
    mqtt.sendMessage(std::string(temperature_topic), std::to_string(temp));

    usleep(500 * 1000);
  }
}