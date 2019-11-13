#include <FreeRTOS.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <esplibs/libmain.h>
#include <espressif/esp_common.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>
#include <etstimer.h>
#include <event_groups.h>
#include <homekit/characteristics.h>
#include <homekit/homekit.h>
#include <stdio.h>
#include <sysparam.h>
#include <task.h>

#include "homekit_callback.h"
#include "homekit_config.h"
#include "ir.h"

void init();
void led_init();

static void wifi_init() {
  struct sdk_station_config wifi_config = {
    .ssid = WIFI_SSID,
    .password = WIFI_PASS,
  };

  sdk_wifi_set_opmode(STATION_MODE);
  sdk_wifi_station_set_config(&wifi_config);
  sdk_wifi_station_connect();
}

void on_homekit_event(homekit_event_t event) {
  if (event == HOMEKIT_EVENT_PAIRING_ADDED) {
    if (!homekit_initialized) {
      init();
    }
  } else if (event == HOMEKIT_EVENT_PAIRING_REMOVED) {
    if (!homekit_is_paired()) {
      printf("Restarting\n");
      sdk_system_restart();
    }
  }
}

void init() {
  led_init();
  ir_init();

  xTaskCreate(temperature_sensor_task, "TempSensor", 256, NULL,
              tskIDLE_PRIORITY, NULL);
  xTaskCreate(update_display_task, "UpdateDisplay", 512, NULL, tskIDLE_PRIORITY,
              NULL);
  // xTaskCreate(ir_dump_task, "IR dump", 2048, NULL, tskIDLE_PRIORITY, NULL);

  gpio_enable(RELAY_GPIO, GPIO_OUTPUT);
  gpio_enable(BTN_GPIO, GPIO_INPUT);
  xTaskCreate(button_poll_task, "ButtonPoll", 256, NULL, tskIDLE_PRIORITY,
              NULL);
  homekit_initialized = true;
}

void led_init() {
  gpio_enable(LED_GPIO, GPIO_OUTPUT);
  led_write(false);
}

void user_init(void) {
  uart_set_baud(0, 115200);
  gpio_set_pullup(14, false, false);
  sysparam_set_string("hostname", "Fonger-Homekit");
  wifi_init();
  homekit_server_init(&homekit_config);

  if (homekit_is_paired()) {
    init();
  }
}
