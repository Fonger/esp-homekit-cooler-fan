# esp-homekit-cooler-fan

This is a HomeKit NodeMCU infrared controller for my air conditioner and a fan.

## Home Appliance

- Swift Cabinet Window Air Conditioner SWF-08C
  - Model No. SWF-08C
  - Has only cooling ability. No heater
  - The remote controller has no temperature display. It has only temperature UP/DOWN button.
- Y.S. Tech 16" Smart Fan (元山家電)
  - Model No. YS-9166SFR
  - Has IR control ability

## Usage

1. Setup build environment for [esp-open-rtos](https://github.com/SuperHouse/esp-open-rtos)
2. Run `git submodule update --init --recursive` for submodules this project use
3. Copy `config-sample.h` to `config.h` and edit the settings.
