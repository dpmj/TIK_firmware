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

#include <TFT_eSPI.h>  // Hardware-specific library. 

#include "SYSTEM/SystemStatus.h"
#include "SYSTEM/version.h"

#include "button/ButtonMod.h"  // Button override from TFT_eSPI


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
#define WIFI_ICON_X_POS 120
#define WIFI_ICON_Y_POS 2

// BLUETOOTH
#include "sprites/BT/BT_ICON_SPRITE.c"
#define BT_ICON_X_POS 142
#define BT_ICON_Y_POS 3

// BATTERY CHARGING
#include "sprites/BAT/BAT_CH_ICON_SPRITE.c"
#define BAT_CH_ICON_X_POS 157
#define BAT_CH_ICON_Y_POS 2

// BATTERY LEVEL
#include "sprites/BAT/BAT_LEVELS_SPRITES.c"
#define BAT_LEVEL_ICON_X_POS 205
#define BAT_LEVEL_ICON_Y_POS 4


/* ---------------------------------------------------------------------------------------
 * Status bar style
 */

#define STATUSBAR_HEIGHT 20


/* ---------------------------------------------------------------------------------------
 * Title style
 */

#define TITLE_RIGHT_MARGIN 20
#define TITLE_RIGHT_X_POS SCREEN_X_PIXELS - TITLE_RIGHT_MARGIN

#define TITLE_CENTER_X_POS SCREEN_X_PIXELS/2
#define TITLE_CENTER_Y_POS 30


/* ---------------------------------------------------------------------------------------
 * Button style
 */

#define BUTTON_CENTER_WIDTH 220
#define BUTTON_CENTER_HEIGHT 40
#define BUTTON_CENTER_MARGIN 10

#define BUTTON_CENTER_X_POS SCREEN_X_PIXELS/2
#define BUTTON_CENTER_Y_POS_1 120 + BUTTON_CENTER_HEIGHT/2
#define BUTTON_CENTER_Y_POS_2 BUTTON_CENTER_Y_POS_1 + BUTTON_CENTER_HEIGHT + BUTTON_CENTER_MARGIN
#define BUTTON_CENTER_Y_POS_3 BUTTON_CENTER_Y_POS_2 + BUTTON_CENTER_HEIGHT + BUTTON_CENTER_MARGIN
#define BUTTON_CENTER_Y_POS_4 BUTTON_CENTER_Y_POS_3 + BUTTON_CENTER_HEIGHT + BUTTON_CENTER_MARGIN

#define BUTTON_BACK_WIDTH 80
#define BUTTON_BACK_HEIGHT 30

#define BUTTON_BACK_X_POS BUTTON_BACK_WIDTH/2
#define BUTTON_BACK_Y_POS SCREEN_Y_PIXELS - BUTTON_BACK_HEIGHT/2

/* ---------------------------------------------------------------------------------------
 * Measurements display on top
 */

#define MEAS_HEIGHT 30
#define MEAS_READY_WIDTH 30
#define MEAS_READY_HEIGHT 15
#define MEAS_READY_X_POS 10+MEAS_READY_WIDTH/2
#define MEAS_READY_Y_POS STATUSBAR_HEIGHT+MEAS_HEIGHT/2


/* --------------------------------------------------------------------------------------- 
 * Graph grid style
 */

#define GRAPH_X_TICKS 9  // number of char sub-divisions, like an oscilloscope
#define GRAPH_Y_TICKS 7

#define GRAPH_TICKS_LENGTH 6  // maximum nº of possible characters: e.g. '-100K\0'
#define GRAPH_TOP_MARGIN 10

#define GRAPH_AREA_WIDTH SCREEN_X_PIXELS  // total area of graph including margins
#define GRAPH_AREA_HEIGHT 120

#define GRAPH_GRID_WIDTH 176  // width of the graph 
#define GRAPH_GRID_SIDE_MARGIN (GRAPH_AREA_WIDTH-GRAPH_GRID_WIDTH)/2

#define GRAPH_GRID_HEIGHT 84  // height of the graph
#define GRAPH_GRID_BOTTOM_MARGIN GRAPH_AREA_HEIGHT-GRAPH_TOP_MARGIN-GRAPH_GRID_HEIGHT

#define GRAPH_LINE_X_SEP GRAPH_GRID_WIDTH/(GRAPH_X_TICKS-1)
#define GRAPH_LINE_Y_SEP GRAPH_GRID_HEIGHT/(GRAPH_Y_TICKS-1)

#define GRAPH_FIRST_POS 20
#define GRAPH_SECOND_POS GRAPH_FIRST_POS+GRAPH_AREA_HEIGHT

/* ---------------------------------------------------------------------------------------
 * Data structures
 */

enum screens {
    MAIN,
    MEASURE,
    MEASURE_HISTORIC,
    SETTINGS,
    HELP,
    SHUTDOWN_CONFIRM,
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

    // Tools
    bool _buttonMonitor(TFT_eSPI_Button_Mod *button);
    void _drawTitle();
    void _clearScreen();
    void _drawGraphGrid(uint16_t y_pos,  // position
                         float x_lims[2],  // x axis limits. Format: [start, end]
                         float y1_lims[2],  // primary y axis limits
                         float y2_lims[2],  // secondary y axis limits
                         char *title, char *x_title, char *y1_title, char *y2_title);  // titles
    void _drawGraphs();

public:
    GUI(SystemStatus *sys_status, SemaphoreHandle_t *mutex_sys_status,
        touch_pos *touch_pos, SemaphoreHandle_t *mutex_touch_pos,
        screens *screen_id, SemaphoreHandle_t *mutex_spi);

    TFT_eSPI _tft = TFT_eSPI();       // Invoke custom library
    TFT_eSprite _spr_bt = TFT_eSprite(&_tft);

    void drawStatusBar();                 
    void drawMainScreen();
    void drawMeasureScreen(TaskHandle_t *sampler_taskHandler);
    void drawSettingsScreen();
    void drawHelpScreen();
    void drawShutdownConfirmScreen();

    void init();                    // initialize screen 
    void calibrate();               // screen calibration only if necessary    
};
