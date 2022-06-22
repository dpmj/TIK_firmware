/**
 * @file GUI.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-04-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include <Arduino.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>  // non-blocking waitings

#include "FS.h"        // file system wrapper
#include <SPI.h>

#include <TFT_eSPI.h> // Hardware-specific library. 

/* MUST BE CONFIGURED PRIOR TO USAGE.
    // Configuration:
    #define TFT_MISO 19
    #define TFT_MOSI 23
    #define TFT_SCLK 18
    #define TFT_CS   15  // Chip select control pin
    #define TFT_DC    2  // Data Command control pin
    #define TFT_RST   4  // Reset pin (could connect to RST pin)
    #define TOUCH_CS 12  // Chip select pin (T_CS) of touch screen
    #define SPI_FREQUENCY  80000000
    // Optional reduced SPI frequency for reading TFT
    #define SPI_READ_FREQUENCY  20000000
    // The XPT2046 requires a lower SPI clock rate of 2.5MHz so we define that here:
    #define SPI_TOUCH_FREQUENCY  2500000
*/


#include "SYSTEM/SystemStatus.h"

// Button override from TFT_eSPI

#include "button/Button.h"



/* ---------------------------------------------------------------------------------------
 * TFT LCD Config
 */

#define SCREEN_X_PIXELS 240
#define SCREEN_Y_PIXELS 320
#define SCREEN_ROTATION 0  // 0: vertical, 1: horizontal.



/* ---------------------------------------------------------------------------------------
 * Calibration
 */

// This is the file name used to store the touch coordinate calibration data
#define CALIBRATION_FILE "/TouchCalData3"

// Set REPEAT_CAL to true to run calibration again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false



/* ---------------------------------------------------------------------------------------
 * Status bar icons
 */

// WI-FI
#include "sprites/WIFI/WIFI_SPRITES.c"
#define WIFI_ICON_X_POS 110
#define WIFI_ICON_Y_POS 2

// BLUETOOTH
#include "sprites/BT/BT_ICON_SPRITE.c"
#define BT_ICON_X_POS 126
#define BT_ICON_Y_POS 2

// BATTERY CHARGING
#include "sprites/BAT/BAT_CH_ICON_SPRITE.c"
#define BAT_CH_ICON_X_POS 142
#define BAT_CH_ICON_Y_POS 2

// BATTERY LEVEL
#include "sprites/BAT/BAT_LEVELS_SPRITES.c"
#define BAT_LEVEL_ICON_X_POS 200
#define BAT_LEVEL_ICON_Y_POS 4


/* ---------------------------------------------------------------------------------------
 * Data structures
 */

enum screens {
    WELCOME,
    MAIN,
    LOADING,
    MEASURE,
    SETTINGS,
    HELP,
    GOODBYE,
};

struct touch_pos {
    bool pressed = false;
    uint16_t x = 0;
    uint16_t y = 0;
};

const uint32_t LOOP_TICKS = 50 / portTICK_PERIOD_MS; // X ms / 1 ms = X ticks



/* ---------------------------------------------------------------------------------------
 * Graphic User Interface class
 */



class GUI
{

private:
    // uint8_t _screenID = 0;  // screen IF in which the system is
    uint16_t _xTouchPos, _yTouchPos;  // touch position

    SystemStatus *_sys_status;  // pointer to a global systatus variable accessed as mutex_
    SemaphoreHandle_t *_mutex_sys_status;

    touch_pos *_touch_pos;
    SemaphoreHandle_t *_mutex_touch_pos;

    screens *_screen_id;
    //SemaphoreHandle_t *_mutex_screen_id;

    SemaphoreHandle_t *_mutex_spi;

public:
    GUI(SystemStatus *sys_status, SemaphoreHandle_t *mutex_sys_status,
        touch_pos *touch_pos, SemaphoreHandle_t *mutex_touch_pos,
        screens *screen_id, //SemaphoreHandle_t *mutex_screen_id,
        SemaphoreHandle_t *mutex_spi
        );

    TFT_eSPI _tft = TFT_eSPI();       // Invoke custom library
    TFT_eSprite _spr_bt = TFT_eSprite(&_tft);


    void drawStatusBar();
    void drawWelcomeScreen();       // screenID 0                     
    // void drawMainScreen();       // screenID 1
    void drawMeasureScreen();       // screenID 2
    // void drawSettingsScreen();   // screenID 3
    // void drawHelpScreen();       // screenID 4
    // void drawGoodbyeScreen();    // screenID 5


    void init();                    // initialize screen 
    void calibrate();               // screen calibration only if necessary
    
};
