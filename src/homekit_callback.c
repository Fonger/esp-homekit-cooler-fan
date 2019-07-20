
#include <FreeRTOS.h>
#include <esp8266.h>
#include <esplibs/libmain.h>
#include <espressif/esp_system.h>
#include <event_groups.h>
#include <stdio.h>
#include <stdlib.h>
#include <task.h>

#include <dht/dht.h>

#include "homekit_callback.h"
#include "homekit_config.h"
#include "ir.h"

void led_write(bool on) { gpio_write(LED_GPIO, on ? 0 : 1); }

void ac_identify_task(void *_args) {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      led_write(true);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      led_write(false);
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelay(250 / portTICK_PERIOD_MS);
  }

  led_write(false);
  vTaskDelete(NULL);
}

void ac_identify(homekit_value_t _value) {
  printf("AC identify\n");
  xTaskCreate(ac_identify_task, "Thermostat identify", 128, NULL, 2, NULL);
};

homekit_value_t ac_active_get() { return HOMEKIT_UINT8(AC.active); };
void ac_active_set(homekit_value_t value) {
  if (AC.active == value.bool_value)
    return;

  ac_active.value = value;
  AC.active = value.bool_value;

  ir_ac_power();
};

homekit_value_t target_temperature_get() {
  return HOMEKIT_FLOAT(AC.targetTemperature);
};
void target_temperature_set(homekit_value_t value) {
  if (!AC.active) {
    printf("Can't change temperature while AC is off\n");
    return;
  }

  printf("set target temp = %g\n", value.float_value);

  int diff = value.float_value - AC.targetTemperature;
  int times = abs(diff);
  if ((xTaskGetTickCount() - AC.lastTargetTempChange) * portTICK_PERIOD_MS >=
      5000) {
    times++;
  }
  for (int i = 0; i < times; i++) {
    if (diff > 0) {
      ir_ac_temp_up();
    } else {
      ir_ac_temp_down();
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
    AC.lastTargetTempChange = xTaskGetTickCount();
    vTaskDelay(150 / portTICK_PERIOD_MS);
  }
  AC.targetTemperature = value.float_value;
  target_temperature.value = value;
};

homekit_value_t fan_active_get() { return HOMEKIT_UINT8(FAN.active); };
void fan_active_set(homekit_value_t value) {
  if (FAN.active == value.bool_value)
    return;

  fan_active.value = value;
  FAN.active = value.bool_value;

  ir_fan_power();
};

void temperature_sensor_task(void *_args) {
  gpio_set_pullup(TEMPERATURE_SENSOR_GPIO, false, false);

  float humidity_value, temperature_value;
  while (1) {
    bool success = dht_read_float_data(DHT_TYPE_DHT22, TEMPERATURE_SENSOR_GPIO,
                                       &humidity_value, &temperature_value);
    if (success) {
      printf("Got readings: temperature %g, humidity %g\n", temperature_value,
             humidity_value);
      current_temperature.value = HOMEKIT_FLOAT(temperature_value);
      current_humidity.value = HOMEKIT_FLOAT(humidity_value);

      homekit_characteristic_notify(&current_temperature,
                                    current_temperature.value);
      homekit_characteristic_notify(&current_humidity, current_humidity.value);

      /* INACTIVE = 0; IDLE = 1; HEATING = 2; COOLING = 3; */
      homekit_value_t new_state_value = HOMEKIT_UINT8(0);
      printf("ac state: %d, target temp: %g\n", AC.active,
             AC.targetTemperature);
      if (AC.active) {
        printf("ac state: active\n");
        if (AC.targetTemperature > current_temperature.value.float_value) {
          printf("ac state: idle\n");
          new_state_value = HOMEKIT_UINT8(1);
        } else {
          printf("ac state: cooling\n");
          new_state_value = HOMEKIT_UINT8(3);
        }
      }
      if (!homekit_value_equal(&new_state_value,
                               &current_heater_cooler_state.value)) {
        current_heater_cooler_state.value = new_state_value;

        homekit_characteristic_notify(&current_heater_cooler_state,
                                      current_heater_cooler_state.value);
      }

    } else {
      printf("Couldn't read data from sensor\n");
    }

    vTaskDelay(TEMPERATURE_POLL_PERIOD / portTICK_PERIOD_MS);
  }
}
