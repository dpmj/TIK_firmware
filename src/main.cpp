/**
 * @file main.cpp
 * @author Juan Del Pino Mena (dpmj@protonmail.com)
 * @brief Handheld TIK software main file. Coordinates all tasks.
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) Juan Del Pino Mena 2022
 * 
 * Main program file. Setup and tasks
 * 
 */



/* ---------------------------------------------------------------------------------------
 * Import required header files
 */

#include <Arduino.h>   // Arduino Framework
#include <freertos/FreeRTOS.h>  // Multitasking
//#include <SPI.h>       // SPI communication

//#include <TFT_eSPI.h> // Hardware-specific library. MUST BE CONFIGURED PRIOR TO USAGE.
//#include "TFT_LCD/TFT_LCD_utils.cpp"  // LCD utils: calibration,

//#include "ADCSampler/Signal.h"  // signal data type, with useful #defines
#include "ADCSampler/ADCSampler.h"    // ADC Sampler lib

#include "AIC/AIC.h"             // Akaike computation
#include "AIC/SignalRE.h"

#include "GUI/GUI.h"  // LCD user interface

#include "SYSTEM/SystemStatus.h"  // system status variable



/* ---------------------------------------------------------------------------------------
 * ADCSampler
 */

ADCSampler *adcSampler = NULL;

#define BUF_COUNT 4         // number of dma buffers
#define BUF_LEN 1024        // dma buffer length, number of samples. Maximum: 1024



/* ---------------------------------------------------------------------------------------
 * Akaike calculus
 */ 


Signal_RE SIGNALS_SENSORS;

AIC aic = AIC(&SIGNALS_SENSORS);



/* ---------------------------------------------------------------------------------------
 * System status 
 */

static struct SystemStatus SYSTEM_STATUS;  // system status
static SemaphoreHandle_t MUTEX_SYS_STATUS = xSemaphoreCreateMutex();  // mutex for system status


static SemaphoreHandle_t MUTEX_SPI = xSemaphoreCreateMutex();  // mutex for SPI conflicts




/* ---------------------------------------------------------------------------------------
 * User interface
 */ 


static struct touch_pos TOUCH_POS; // touch position detection
static SemaphoreHandle_t MUTEX_TOUCH_POS = xSemaphoreCreateMutex();  // mutex for touch position

static enum screens SCREEN_ID = screens::WELCOME;  // ID of the current screen
//static SemaphoreHandle_t MUTEX_SCREEN_ID = xSemaphoreCreateMutex(); 
// to-do: eliminar mutex, screen_id se va a alterar sólo secuencialmente



// contains the TFT object from the TFT_eSPI library
GUI gui = GUI(&SYSTEM_STATUS, &MUTEX_SYS_STATUS, 
              &TOUCH_POS, &MUTEX_TOUCH_POS,
              &SCREEN_ID, //&MUTEX_SCREEN_ID,
              &MUTEX_SPI
             );


/* ---------------------------------------------------------------------------------------
 * Tasks
 */

// REFRESH TOUCH

//const uint32_t LOOP_TICKS = 20 / portTICK_PERIOD_MS; // 20 ms / 1 ms = 20 ticks
xTaskHandle refreshTouch_TaskHandler;  // refreshScreen task handler pointer

/**
 * @brief Non-Blocking screen touch polling task
 */
void refreshTouch(void *parameters)
{ 
    while(true) {
        if (xSemaphoreTake(MUTEX_SPI, LOOP_TICKS) == pdTRUE) {
            // Touch detection everytime the screen is refreshed
            TOUCH_POS.pressed = gui._tft.getTouch(&TOUCH_POS.x, &TOUCH_POS.y);
            if (TOUCH_POS.pressed) {
                gui._tft.fillCircle(TOUCH_POS.x, TOUCH_POS.y, 2, TFT_RED);
            }
            gui._tft.setCursor(0, 20);
            gui._tft.setTextFont(2);
            gui._tft.setTextSize(1);
            gui._tft.setTextColor(TFT_RED, TFT_WHITE);
            gui._tft.print("x: ");
            gui._tft.println(TOUCH_POS.x);
            gui._tft.print("y: ");
            gui._tft.println(TOUCH_POS.y);
            gui._tft.print("SCREEN Nº: ");
            gui._tft.println(SCREEN_ID);
            xSemaphoreGive(MUTEX_SPI);
        }
        vTaskDelay(LOOP_TICKS);
    }
}



// REFRESH STATUSBAR 
// TO-DO: How to only refresh in events?

const uint32_t STATUS_BAR_TICKS = 1000 / portTICK_PERIOD_MS;  // once a second
xTaskHandle statusBarRefresher_TaskHandler;  // refreshScreen task handler pointer

/**
 * @brief Draw status bar to ensure it's up to date
 */
void statusBarRefresher(void *parameters)
{   
    while(true) {
        if (xSemaphoreTake(MUTEX_SPI, LOOP_TICKS) == pdTRUE) {
            gui.drawStatusBar();
            xSemaphoreGive(MUTEX_SPI);
        }
        vTaskDelay(STATUS_BAR_TICKS);
    }
}



// SCREEN HANDLER. SELECTS SCREEN TO DRAW
xTaskHandle screenManager_TaskHandler;  // refreshScreen task handler pointer

/**
 * @brief 
 * 
 * @param parameters 
 */
void screenManager(void *parameters)
{   
    while(true) {
        Serial.printf("screen ID: %d", SCREEN_ID);
        switch(SCREEN_ID) {
            case WELCOME:
                gui.drawWelcomeScreen();
                break;

            case MEASURE:
                gui.drawMeasureScreen();
                break;
            
            default:
                // SHOULD NEVER ENTER HERE, ERROR OTHERWISE
                while (xSemaphoreTake(MUTEX_SPI, LOOP_TICKS) != pdTRUE);
                gui._tft.fillScreen(TFT_BLUE);  // blue screen of death
                xSemaphoreGive(MUTEX_SPI);
                return; // forced exit
                break;
        }
        vTaskDelay(LOOP_TICKS);
    }
}



/* ---------------------------------------------------------------------------------------
 * Setup
 */

/**
 * @brief Setup interfaces, communication, task initialization, etc.
 */
void setup(void)
{
    // UART INIT

    Serial.begin(115200);
    Serial.printf("setup\n");
    
    // UPDATE SYSTEM STATUS  -- EXAMPLE PARAMETERS
    // In the real system these should be read or adquired somehow

    SYSTEM_STATUS.battery_level_percentage = 50;
    SYSTEM_STATUS.battery_current_mA = 500;
    SYSTEM_STATUS.battery_power_mW = 1400;
    SYSTEM_STATUS.battery_total_capacity_mWh = 9600;
    SYSTEM_STATUS.battery_used_capacity_mWh = 0;
    SYSTEM_STATUS.battery_charging = true;
    SYSTEM_STATUS.battery_fully_charged = true;
    SYSTEM_STATUS.wifi_connected = true;
    SYSTEM_STATUS.wifi_rssi_dBm = -69;
    SYSTEM_STATUS.bt_connected = true;
    SYSTEM_STATUS.bt_rssi_dBm = -60;
    SYSTEM_STATUS.triggered = true;
    SYSTEM_STATUS.ready = false;

    // GUI INIT

    gui.init();  // initiates screen
    gui.calibrate();  // calibrates only if necessary

    // Status messages

    Serial.printf("calibration done\n");

    // TASKS CREATION

    // Screen touch refresher
    xTaskCreatePinnedToCore(
        refreshTouch,                   // Task function
        "refreshScreenTask",            // Task Name
        16384,                           // Stack depth in bytes
        NULL,                           // Task function parameters
        1,                              // Task priority
        &refreshTouch_TaskHandler,      // Task handler pointer 
        0                               // Core in which to run task  
    );

    xTaskCreatePinnedToCore(
        screenManager,                  // Task function
        "refreshManagerTask",            // Task Name
        16384,                          // Stack depth in bytes
        NULL,                           // Task function parameters
        1,                              // Task priority
        &screenManager_TaskHandler,     // Task handler pointer 
        0                               // Core in which to run task  
    ); 


    xTaskCreatePinnedToCore(
        statusBarRefresher,             // Task function
        "statusBarRefreshTask",            // Task Name
        16384,                           // Stack depth in bytes
        NULL,                           // Task function parameters
        1,                              // Task priority
        &statusBarRefresher_TaskHandler,  // Task handler pointer 
        1                               // Core in which to run task  
    ); 

    Serial.printf("activated tasks\n");


}



/* ---------------------------------------------------------------------------------------
 * Loop
 */

/**
 * @brief Nothing to do here, everything is taken care by tasks.
 */
void loop()
{
    vTaskDelete(NULL);
}
