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
#include <FreeRTOS.h>  // non-blocking waitings

#include <TFT_eSPI.h> // Hardware-specific library. MUST BE CONFIGURED PRIOR TO USAGE.
#include "TFT_LCD_utils.cpp"  // LCD utils: calibration,

#include "SYSTEM/sys_status.h"



/* ---------------------------------------------------------------------------------------
 * TFT LCD Config
 */


// Comment out to stop drawing black spots
#define BLACK_SPOT

#define X_PIXELS = 240
#define Y_PIXELS = 320
#define SCREEN_ROTATION_VERTICAL 0
#define SCREEN_ROTATION_HORIZONTAL 1
#define SCREEN_ROTATION_VERTICAL_INVERTED 2
#define SCREEN_ROTATION_HORIZONTAL_INVERTED 3



/*---------------------------------------------------------------------------------------
 * Graphic User Interface class
 */


class GUI
{

private:

    TFT_eSPI _tft = TFT_eSPI();       // Invoke custom library

    uint8_t _screenID = 0;  // screen IF in which the system is
    uint16_t _xTouchPos, _yTouchPos;  // touch position

    sys_status *_sys_status;  // pointer to a global systatus variable
                              // accessed as mutex

    void _drawStatusBar();

    void _drawWelcomeScreen();  // screenID 0
    void _drawMainScreen();     // screenID 1
    void _drawLoadingScreen();  // screenID 2
    void _drawMeasureScreen();  // screenID 3
    void _drawSettingsScreen(); // screenID 4
    void _drawHelpScreen();  // screenID 5
    void _drawGoodbyeScreen();  // screenID 6
    

public:
    GUI();

    uint8_t getScreenID();  // gives the _screenID parameter

    void calibrate();   // screen calibration 
    void drawScreen();  // use this to refresh the screen
};
