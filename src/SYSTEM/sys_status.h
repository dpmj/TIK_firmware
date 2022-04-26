/**
 * @file sys_status.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-04-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>


struct sys_status
{
    //uint64_t uptime_ms;

    uint8_t battery_level_percentage;
    uint16_t battery_voltage_mV;
    uint16_t battery_current_mA;
    uint16_t battery_power_mW;
    uint16_t battery_total_capacity_mWh;
    uint16_t battery_used_capacity_mWh;
    bool battery_charging;
    bool battery_fully_charged;

    bool wifi_connected;
    uint8_t wifi_rssi_dBm;

    bool bt_connected;
    uint8_t bt_rssi_dBm;

    bool triggered;
    bool ready;
    //uint8_t current_screen;

};
