#include <FreeRTOS.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <stdio.h>
#include <stdlib.h>
#include <task.h>

#include "homekit_config.h"
#include "ir.h"

int16_t AC_POWER[] = {
  9000, -4500, 562, -562,  562, -562,  562, -562,  562, -562,  562, -562,
  562,  -562,  562, -562,  562, -1688, 562, -1688, 562, -1688, 562, -1688,
  562,  -1688, 562, -1688, 562, -1688, 562, -1688, 562, -1688, 562, -562,
  562,  -562,  562, -562,  562, -562,  562, -562,  562, -562,  562, -562,
  562,  -562,  562, -1688, 562, -1688, 562, -1688, 562, -1688, 562, -1688,
  562,  -1688, 562, -1688, 562, -1688, 562};

int16_t AC_TEMP_DOWN[] = {
  9000, -4500, 562, -562,  562, -562,  562, -562,  562, -562,  562, -562,
  562,  -562,  562, -562,  562, -1688, 562, -1688, 562, -1688, 562, -1688,
  562,  -1688, 562, -1688, 562, -1688, 562, -1688, 562, -1688, 562, -1688,
  562,  -562,  562, -562,  562, -562,  562, -1688, 562, -562,  562, -562,
  562,  -562,  562, -562,  562, -1688, 562, -1688, 562, -1688, 562, -562,
  562,  -1688, 562, -1688, 562, -1688, 562};

int16_t AC_TEMP_UP[] = {
  9000, -4500, 562, -562,  562, -562,  562, -562,  562, -562,  562, -562,
  562,  -562,  562, -562,  562, -1688, 562, -1688, 562, -1688, 562, -1688,
  562,  -1688, 562, -1688, 562, -1688, 562, -1688, 562, -1688, 562, -562,
  562,  -1688, 562, -562,  562, -1688, 562, -562,  562, -562,  562, -562,
  562,  -562,  562, -1688, 562, -562,  562, -1688, 562, -562,  562, -1688,
  562,  -1688, 562, -1688, 562, -1688, 562};

int16_t AC_WIND_SPEED[] = {
  9000, -4500, 562, -562,  562, -562,  562, -562,  562, -562,  562, -562,
  562,  -562,  562, -562,  562, -1688, 562, -1688, 562, -1688, 562, -1688,
  562,  -1688, 562, -1688, 562, -1688, 562, -1688, 562, -1688, 562, -1688,
  562,  -562,  562, -562,  562, -562,  562, -562,  562, -562,  562, -562,
  562,  -562,  562, -562,  562, -1688, 562, -1688, 562, -1688, 562, -1688,
  562,  -1688, 562, -1688, 562, -1688, 562};

int16_t AC_SWING_ENABLE[] = {
  9000, -4500, 562, -562,  562, -562,  562, -562,  562, -562,  562, -562,
  562,  -562,  562, -562,  562, -1688, 562, -1688, 562, -1688, 562, -1688,
  562,  -1688, 562, -1688, 562, -1688, 562, -1688, 562, -1688, 562, -562,
  562,  -562,  562, -1688, 562, -1688, 562, -562,  562, -562,  562, -562,
  562,  -562,  562, -1688, 562, -1688, 562, -562,  562, -562,  562, -1688,
  562,  -1688, 562, -1688, 562, -1688, 562};

int16_t AC_SWING_DISABLE[] = {
  9000, -4500, 562, -562,  562, -562,  562, -562,  562, -562,  562, -562,
  562,  -562,  562, -562,  562, -1688, 562, -1688, 562, -1688, 562, -1688,
  562,  -1688, 562, -1688, 562, -1688, 562, -1688, 562, -1688, 562, -562,
  562,  -562,  562, -562,  562, -562,  562, -1688, 562, -562,  562, -562,
  562,  -562,  562, -1688, 562, -1688, 562, -1688, 562, -1688, 562, -562,
  562,  -1688, 562, -1688, 562, -1688, 562};

int16_t FAN_POWER[] = {
  +590,  -666,  +1130, -672,  +1126, -1376, +588,  -1372, +590,  -1376, +586,
  -1374, +590,  -1374, +590,  -668,  +1128, -9999, -9999, -9999, -2768, +588,
  -666,  +1128, -670,  +1128, -1374, +590,  -1374, +588,  -1372, +592,  -1372,
  +590,  -1374, +588,  -666,  +1132, -9999, -9999, -9999, -2768, +590,  -664,
  +1126, -672,  +1128, -1376, +586,  -1376, +590,  -1376, +586,  -1374, +592,
  -1372, +586,  -670,  +1128, -9999, -9999, -9999, -2768, +586,  -670,  +1126,
  -668,  +1130, -1374, +590,  -1372, +590,  -1374, +590,  -1374, +590,  -1374,
  +588,  -670,  +1128, -9999, -9999, -9999, -2768, +590,  -666,  +1130, -672,
  +1126, -1376, +588,  -1372, +590,  -1376, +586,  -1374, +590,  -1374, +590,
  -668,  +1128, -9999, -9999, -9999, -2768, +590,  -666,  +1130, -672,  +1126,
  -1376, +588,  -1372, +590,  -1376, +586,  -1374, +590,  -1374, +590,  -668,
  +1128};

int16_t FAN_ROTATION_SPEED[] = {
  +586,  -666,  +1126, -672,  +1126, -1376, +588,  -1374, +590,  -1374, +588,
  -1410, +552,  -668,  +1128, -1376, +588,  -9999, -9999, -9999, -2768, +586,
  -668,  +1128, -668,  +1128, -1378, +588,  -1374, +590,  -1374, +590,  -1374,
  +588,  -668,  +1130, -1376, +588,  -9999, -9999, -9999, -2768, +588,  -666,
  +1126, -670,  +1128, -1380, +586,  -1374, +588,  -1386, +576,  -1376, +588,
  -716,  +1000, -1456, +584,  -9999, -9999, -9999, -2768, +540,  -712,  +1084,
  -672,  +1128, -1374, +590,  -1424, +536,  -1376, +586,  -1380, +584,  -668,
  +1130, -1412, +550,  -9999, -9999, -9999, -2768, +540,  -712,  +1084, -672,
  +1128, -1374, +590,  -1424, +536,  -1376, +586,  -1380, +584,  -668,  +1130,
  -1412, +550,  -9999, -9999, -9999, -2768, +540,  -712,  +1084, -672,  +1128,
  -1374, +590,  -1424, +536,  -1376, +586,  -1380, +584,  -668,  +1130, -1412,
  +550};

int16_t FAN_TIMER[] = {
  +588,  -668,  +1126, -670,  +1132, -1374, +590,  -1374, +588,  -1376, +590,
  -668,  +1126, -1380, +586,  -1372, +592,  -9999, -9999, -9999, -2768, +522,
  -734,  +1128, -668,  +1130, -1376, +586,  -1376, +584,  -1380, +588,  -668,
  +1126, -1382, +590,  -1370, +586,  -9999, -9999, -9999, -2768, +586,  -716,
  +1078, -668,  +1130, -1374, +588,  -1376, +590,  -1370, +594,  -664,  +1132,
  -1372, +590,  -1438, +524,  -9999, -9999, -9999, -2768, +592,  -666,  +1128,
  -724,  +1070, -1376, +588,  -1378, +590,  -1370, +594,  -712,  +976,  -1480,
  +582,  -1396, +574,  -9999, -9999, -9999, -2768, +592,  -666,  +1128, -724,
  +1070, -1376, +588,  -1378, +590,  -1370, +594,  -712,  +976,  -1480, +582,
  -1396, +574,  -9999, -9999, -9999, -2768, +592,  -666,  +1128, -724,  +1070,
  -1376, +588,  -1378, +590,  -1370, +594,  -712,  +976,  -1480, +582,  -1396,
  +574};

void ir_init() {
  ir_tx_init();
  ir_rx_init(IR_RX_GPIO, 1024);
}
void ir_ac_power() { IR_SEND(AC_POWER); }
void ir_ac_temp_up() { IR_SEND(AC_TEMP_UP); }
void ir_ac_temp_down() { IR_SEND(AC_TEMP_DOWN); }
void ir_ac_wind_speed() { IR_SEND(AC_WIND_SPEED); }
void ir_ac_swing_enable() { IR_SEND(AC_SWING_ENABLE); }
void ir_ac_swing_disable() { IR_SEND(AC_SWING_DISABLE); }

void ir_fan_power() { IR_SEND(FAN_POWER); }
void ir_fan_rotation_speed() { IR_SEND(FAN_ROTATION_SPEED); }

void ir_dump_task(void *arg) {
  ir_decoder_t *raw_decoder = ir_raw_make_decoder();

  uint16_t buffer_size = sizeof(int16_t) * 1024;
  int16_t *buffer = malloc(buffer_size);
  while (1) {
    int size = ir_recv(raw_decoder, 0, buffer, buffer_size);
    if (size <= 0)
      continue;

    printf("Decoded packet (size = %d):\n", size);
    for (int i = 0; i < size; i++) {
      printf("%5d ", buffer[i]);
      if (i % 16 == 15)
        printf("\n");
    }

    if (size % 16)
      printf("\n");
  }

  raw_decoder->free(raw_decoder);
  vTaskDelete(NULL);
}