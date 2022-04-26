/**
 * @file main.cpp
 * @author Juan Del Pino Mena (dpmj@protonmail.com)
 * @brief Handheld TIK software main file. Coordinates all tasks.
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) Juan Del Pino Mena 2022 under MIT license
 * 
 * Configuration, setup and task coordination
 * 
 */


// This sketch does not use the libraries button drawing
// and handling functions.

// Touch handling for XPT2046 based screens is handled by
// the TFT_eSPI library.

// Calibration data is stored in SPIFFS so we need to include it


/* ---------------------------------------------------------------------------------------
 * Import required header files
 */

#include <Arduino.h>   // Arduino Framework
#include <FreeRTOS.h>  // Multitasking
#include <SPI.h>       // SPI communication
#include "FS.h"        // file system wrapper

//#include <TFT_eSPI.h> // Hardware-specific library. MUST BE CONFIGURED PRIOR TO USAGE.
//#include "TFT_LCD/TFT_LCD_utils.cpp"  // LCD utils: calibration,

#include "ADCSampler/trunk_signal.h"  // signal data type, with useful #defines
#include "ADCSampler/ADCSampler.h"    // ADC Sampler lib

#include "AIC/AIC.h"             // Akaike computation

#include "GUI/GUI.h"  // LCD user interface



/* ---------------------------------------------------------------------------------------
 * ADCSampler
 */

ADCSampler *adcSampler = NULL;

#define BUF_COUNT 4         // number of dma buffers
#define BUF_LEN 1024        // dma buffer length, number of samples. Maximum: 1024


/* ---------------------------------------------------------------------------------------
 * Akaike calculus
 */ 



// // Draw a red button
// void redBtn()
// {
//   tft.fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, TFT_RED);
//   tft.fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, TFT_DARKGREY);
//   drawFrame();
//   tft.setTextColor(TFT_WHITE);
//   tft.setTextSize(2);
//   tft.setTextDatum(MC_DATUM);
//   tft.drawString("ON", GREENBUTTON_X + (GREENBUTTON_W / 2), GREENBUTTON_Y + (GREENBUTTON_H / 2));
//   SwitchOn = false;
// }

// // Draw a green button
// void greenBtn()
// {
//   tft.fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, TFT_GREEN);
//   tft.fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, TFT_DARKGREY);
//   drawFrame();
//   tft.setTextColor(TFT_WHITE);
//   tft.setTextSize(2);
//   tft.setTextDatum(MC_DATUM);
//   tft.drawString("OFF", REDBUTTON_X + (REDBUTTON_W / 2) + 1, REDBUTTON_Y + (REDBUTTON_H / 2));
//   SwitchOn = true;
// }


/* ---------------------------------------------------------------------------------------
 * User interface
 */ 

// Screen update period. 20 ms --> 50 Hz
const uint32_t LOOP_TICKS = 20 / portTICK_PERIOD_MS; // 20 ms / 1 ms = 20 ticks

GUI gui = GUI();


/**
 * @brief Non-Blocking screen refresh task
 * 
 * @param parameters 
 * @return ** void 
 */
void refreshScreen(void *parameters)
{
    while (true) {
        gui.drawScreen();
        vTaskDelay(LOOP_TICKS);
    }
}


xTaskHandle refreshScreen_TaskHandler;

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void setup(void)
{
    Serial.begin(9600);
    //tft.init();

    // Set the rotation before we calibrate
    //tft.setRotation(1);

    // call screen calibration
    //touch_calibrate();

    // clear screen
    //tft.fillScreen(TFT_BLUE);

    // Draw button (this example does not use library Button class)
    //redBtn();

    xTaskCreatePinnedToCore(
        refreshScreen,          // Task function
        "refreshScreenTask",    // Task Name
        4096,                   // Stack depth in bytes
        NULL,                   // Task function parameters
        1,                      // Task priority
        &refreshScreen_TaskHandler,  // Task handler pointer 
        0);                     // Core in which to run task
    
    
}


//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void loop()
{
  // uint16_t x, y;

  // // See if there's any touch data for us
  // if (tft.getTouch(&x, &y))
  // {
  //   // Draw a block spot to show where touch was calculated to be
  //   #ifdef BLACK_SPOT
  //     tft.fillCircle(x, y, 2, TFT_BLACK);
  //   #endif
    
  //   if (SwitchOn)
  //   {
  //     if ((x > REDBUTTON_X) && (x < (REDBUTTON_X + REDBUTTON_W))) {
  //       if ((y > REDBUTTON_Y) && (y <= (REDBUTTON_Y + REDBUTTON_H))) {
  //         Serial.println("Red btn hit");
  //         redBtn();
  //       }
  //     }
  //   }
  //   else //Record is off (SwitchOn == false)
  //   {
  //     if ((x > GREENBUTTON_X) && (x < (GREENBUTTON_X + GREENBUTTON_W))) {
  //       if ((y > GREENBUTTON_Y) && (y <= (GREENBUTTON_Y + GREENBUTTON_H))) {
  //         Serial.println("Green btn hit");
  //         greenBtn();
  //       }
  //     }
  //   }

  //   Serial.println(SwitchOn);

  // }
}