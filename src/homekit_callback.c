
#include <FreeRTOS.h>
#include <esp8266.h>
#include <esplibs/libmain.h>
#include <espressif/esp_system.h>
#include <event_groups.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <task.h>

#include <dht/dht.h>
#include <ssd1306/ssd1306.h>

#include "config.h"

#include "fonts/fonts.h"
#include <i2c/i2c.h>

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
  AC.swing = false; // Swing mode is always reset to false

  ir_ac_power();
};

homekit_value_t ac_target_temperature_get() {
  return HOMEKIT_FLOAT(AC.targetTemperature);
};
void ac_target_temperature_set(homekit_value_t value) {
  if (!AC.active) {
    printf("Can't change temperature while AC is off\n");
    homekit_characteristic_notify(&target_temperature,
                                  target_temperature.value);
    return;
  }

  printf("set target temp = %g\n", value.float_value);

  int diff = value.float_value - AC.targetTemperature;
  int times = abs(diff);
  if ((xTaskGetTickCount() - AC.lastTargetTempChange) * portTICK_PERIOD_MS >=
      5000) {
    times++; // If last temperature change > 5 seconds from now, the first temp
             // up/down key will not change the temperature, only display
             // temperature on AC so we need to add one more time.
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

homekit_value_t ac_swing_get() { return HOMEKIT_UINT8(AC.swing); }
void ac_swing_set(homekit_value_t value) {
  AC.swing = value.bool_value;
  ac_swing_mode.value = value;
  if (value.bool_value) {
    ir_ac_swing_enable();
  } else {
    ir_ac_swing_disable();
  }
}

uint8_t speed_adjust_table[4][4] = {
  {0, 0, 0, 0}, {0, 0, 1, 2}, {0, 3, 0, 1}, {0, 2, 3, 0}};

homekit_value_t ac_speed_get() { return HOMEKIT_FLOAT(AC.rotationSpeed); }

void ac_speed_set(homekit_value_t value) {
  if (value.float_value > 0 && // prevent losing state from power off
      value.float_value <= 4 && AC.active) {

    int times =
      speed_adjust_table[(int)AC.rotationSpeed][(int)value.float_value];

    for (int i = 0; i < times; i++) {
      ir_ac_wind_speed();
      vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    ac_rotation_speed.value = value;
    AC.rotationSpeed = value.float_value;
  } else {
    ac_rotation_speed.value = HOMEKIT_FLOAT(AC.rotationSpeed);
    homekit_characteristic_notify(&ac_rotation_speed, ac_rotation_speed.value);
  }
}

homekit_value_t fan_active_get() { return HOMEKIT_UINT8(FAN.active); }

void fan_active_set(homekit_value_t value) {
  if (FAN.active == value.bool_value)
    return;

  fan_active.value = value;
  FAN.active = value.bool_value;

  ir_fan_power();
}

homekit_value_t fan_speed_get() { return HOMEKIT_FLOAT(FAN.rotationSpeed); }

void fan_speed_set(homekit_value_t value) {
  printf("FAN roation speed set: %f\n", value.float_value);
  if (value.float_value > 0 && // prevent losing state from power off
      value.float_value <= 4 && FAN.active) {

    int times =
      speed_adjust_table[(int)FAN.rotationSpeed][(int)value.float_value];

    for (int i = 0; i < times; i++) {
      ir_fan_rotation_speed();
      vTaskDelay(1100 / portTICK_PERIOD_MS);
    }

    fan_rotation_speed.value = value;
    FAN.rotationSpeed = value.float_value;
  } else {
    fan_rotation_speed.value = HOMEKIT_FLOAT(FAN.rotationSpeed);
    homekit_characteristic_notify(&fan_rotation_speed,
                                  fan_rotation_speed.value);
  }
}

/* Declare device descriptor */
static const ssd1306_t dev = {.protocol = PROTOCOL,
                              .i2c_dev.bus = I2C_BUS,
                              .i2c_dev.addr = ADDR,
                              .width = DISPLAY_WIDTH,
                              .height = DISPLAY_HEIGHT};

/* Local frame buffer */
static uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
bool dht_success = false;
void temperature_sensor_task(void *_args) {
  gpio_set_pullup(TEMPERATURE_SENSOR_GPIO, false, false);
  float humidity_value, temperature_value;
  while (1) {
    dht_success = dht_read_float_data(DHT_TYPE_DHT22, TEMPERATURE_SENSOR_GPIO,
                                      &humidity_value, &temperature_value);
    if (dht_success) {
      printf("[DHT22] temperature %g°C, humidity %g%%\n", temperature_value,
             humidity_value);
      current_temperature.value = HOMEKIT_FLOAT(temperature_value);
      current_humidity.value = HOMEKIT_FLOAT(humidity_value);

      homekit_characteristic_notify(&current_temperature,
                                    current_temperature.value);
      homekit_characteristic_notify(&current_humidity, current_humidity.value);

      /* INACTIVE = 0; IDLE = 1; HEATING = 2; COOLING = 3; */
      homekit_value_t new_state_value = HOMEKIT_UINT8(0);

      if (AC.active) {
        if (AC.targetTemperature > current_temperature.value.float_value) {
          new_state_value = HOMEKIT_UINT8(1);
        } else {
          new_state_value = HOMEKIT_UINT8(3);
        }
      }
      if (!homekit_value_equal(&new_state_value,
                               &current_heater_cooler_state.value)) {
        current_heater_cooler_state.value = new_state_value;

        homekit_characteristic_notify(&current_heater_cooler_state,
                                      current_heater_cooler_state.value);
      }

      /*
 read light lux with photoresistor in A0

 uint16_t raw_val = sdk_system_adc_read();
 float ratio = ((float)1024 / (float)raw_val) - 1;

 static unsigned long resistor = 1000;

 unsigned long photocell_resistor = resistor * ratio;
 float lux = 29634400 / (float)pow(photocell_resistor, 1.6689);
 printf("[LIGHT] raw: %d / 1024 lux: %.3f\n", raw_val, lux);
 if (lux < MIN_LIGHT_SENSOR_LUX)
   lux = MIN_LIGHT_SENSOR_LUX;
 if (lux > MAX_LIGHT_SENSOR_LUX)
   lux = MAX_LIGHT_SENSOR_LUX;
   fan_light_level.value = HOMEKIT_FLOAT(lux);
   homekit_characteristic_notify(&fan_light_level, fan_light_level.value);
 } else { printf("Couldn't read data from
    sensor\n");
 */
    }
    vTaskDelay(TEMPERATURE_POLL_PERIOD / portTICK_PERIOD_MS);
  }
}

void update_display_task(void *_args) {
  char text[25];
  const font_info_t *font =
    font_builtin_fonts[FONT_FACE_TERMINUS_6X12_ISO8859_1];
  const font_info_t *font_sm = font_builtin_fonts[FONT_FACE_GLCD5x7];

  i2c_init(I2C_BUS, SCL_PIN, SDA_PIN, I2C_FREQ_400K);
  while (ssd1306_init(&dev) != 0) {
    printf("%s: failed to init SSD1306 lcd\n", __func__);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  ssd1306_set_whole_display_lighting(&dev, false);
  ssd1306_set_contrast(&dev, 0);
  ssd1306_set_precharge_period(&dev, 0);
  ssd1306_set_deseltct_lvl(&dev, 0);

  while (1) {

    ssd1306_fill_rectangle(&dev, buffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                           OLED_COLOR_BLACK);

    if (dht_success) {
      sprintf(text, "Temperature %0.1f C",
              current_temperature.value.float_value);
      ssd1306_draw_string(&dev, buffer, font, 0, 0, text, OLED_COLOR_WHITE,
                          OLED_COLOR_BLACK);
      sprintf(text, "Humidity    %0.1f %%", current_humidity.value.float_value);
      ssd1306_draw_string(&dev, buffer, font, 0, 12, text, OLED_COLOR_WHITE,
                          OLED_COLOR_BLACK);
    } else {
      ssd1306_draw_string(&dev, buffer, font, 0, 0, "DHT22 No Data",
                          OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    }

    if (AC.active) {
      sprintf(text, "AC ON(%gC)", AC.targetTemperature);
    } else {
      sprintf(text, "AC OFF");
    }
    ssd1306_draw_string(&dev, buffer, font_sm, 0, 25, text, OLED_COLOR_WHITE,
                        OLED_COLOR_BLACK);

    if (FAN.active) {
      sprintf(text, "FAN ON(%g)", FAN.rotationSpeed);
    } else {
      sprintf(text, "FAN OFF");
    }
    ssd1306_draw_string(&dev, buffer, font_sm, 64, 25, text, OLED_COLOR_WHITE,
                        OLED_COLOR_BLACK);

    if (ssd1306_load_frame_buffer(&dev, buffer)) {
      printf("error print to ssd1306\n");
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

homekit_value_t light_on_get() { return HOMEKIT_BOOL(LIGHT.on); };
void light_on_set(homekit_value_t value) {
  light_on.value = value;
  LIGHT.on = value.bool_value;
  gpio_write(RELAY_GPIO, LIGHT.on);
};

void button_poll_task(void *pvParameters) {
  while (true) {
    while (gpio_read(BTN_GPIO) == true) {
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    printf("button_press!\n");
    light_on_set(HOMEKIT_BOOL(!LIGHT.on));
    homekit_characteristic_notify(&light_on, light_on.value);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}