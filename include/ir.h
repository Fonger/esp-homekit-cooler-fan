#pragma once

#include <ir/ir.h>
#include <ir/raw.h>
#include <ir/rx.h>
#include <ir/tx.h>

#define IR_SEND(command)                                                       \
  ir_raw_send(command, sizeof(command) / sizeof(command[0]));

void ir_init();
void ir_ac_power();
void ir_ac_temp_up();
void ir_ac_temp_down();
void ir_ac_wind_speed();
void ir_ac_swing_enable();
void ir_ac_swing_disable();

void ir_fan_power();
void ir_fan_rotation_speed();

void ir_dump_task(void *arg);