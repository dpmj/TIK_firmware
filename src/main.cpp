/**
 * @file main.cpp
 * @author Juan Del Pino Mena (dpmj@protonmail.com)
 * @brief Handheld TIK software main file. Coordinates all tasks.
 * @version 0.1
 * @date 2022-04-13
 *  
 * Main program file. Setup and tasks
 * 
 */

/* ---------------------------------------------------------------------------------------
 * Import required header files
 */

#include <Arduino.h>   // Arduino Framework
#include <freertos/FreeRTOS.h>  // Multitasking

#include "ADCSampler/ADCSampler.h"  // ADC Sampler lib
#include "AIC/AIC.h"  // Akaike computation
#include "AIC/SignalRE.h"  // Signal structure
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

static enum screens SCREEN_ID = screens::MAIN;  // ID of the current screen

// contains the TFT object from the TFT_eSPI library
GUI gui = GUI(&SYSTEM_STATUS, &MUTEX_SYS_STATUS, 
              &TOUCH_POS, &MUTEX_TOUCH_POS,
              &SCREEN_ID, &MUTEX_SPI);


/* ---------------------------------------------------------------------------------------
 * Tasks
 */

xTaskHandle refreshTouch_TaskHandler;  // refreshScreen task handler pointer

/**
 * @brief Non-Blocking screen touch polling task
 */
void refreshTouch(void *parameters)
{ 
    while(true) {
        if (xSemaphoreTake(MUTEX_SPI, LOOP_TICKS) == pdTRUE) {
            // Touch detection every time the screen is refreshed
            TOUCH_POS.pressed = gui._tft.getTouch(&TOUCH_POS.x, &TOUCH_POS.y);
            if (TOUCH_POS.pressed) {
                gui._tft.fillCircle(TOUCH_POS.x, TOUCH_POS.y, 2, TFT_RED);
            }
            gui._tft.setCursor(0, 20);
            gui._tft.setTextFont(1);
            gui._tft.setTextSize(1);
            gui._tft.setTextColor(TFT_RED, TFT_WHITE);
            gui._tft.print("X: ");
            gui._tft.print(TOUCH_POS.x);
            gui._tft.print(" | Y: ");
            gui._tft.print(TOUCH_POS.y);
            xSemaphoreGive(MUTEX_SPI);
        }
        vTaskDelay(LOOP_TICKS);
    }
}


// REFRESH STATUSBAR 
// TO-DO: Only refresh in events

const uint32_t STATUS_BAR_TICKS = 5000 / portTICK_PERIOD_MS;  // once every 5 seconds
xTaskHandle statusBarRefresher_TaskHandler;  // refreshScreen task handler pointer

/**
 * @brief Draw status bar to ensure it's up to date
 */
void statusBarRefresher(void *parameters)
{   
    while(true) {
        gui.drawStatusBar();
        vTaskDelay(STATUS_BAR_TICKS);
    }
}


// SAMPLER. SAMPLES SIGNALS WHEN INDICATED
xTaskHandle sampler_TaskHandler;  // refreshScreen task handler pointer

/**
 * @brief Responsible for reading from ADC and updating the 
 * 
 */
void sampler(void *parameters) 
{   
    while (true)
    {
        vTaskDelay(1000);
        vTaskSuspend(NULL);  // Suspend task until necessary. Will be activated by another task.
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
        Serial.printf("screen ID: %d\n", SCREEN_ID);
        switch(SCREEN_ID) {
            case screens::MAIN:
                gui.drawStatusBar();
                gui.drawMainScreen();
                break;
            case screens::MEASURE:
                gui.drawStatusBar();
                gui.drawMeasureScreen(&sampler_TaskHandler);
                break;
            case screens::SETTINGS:
                gui.drawStatusBar();
                gui.drawSettingsScreen();
                break;
            case screens::HELP:
                gui.drawStatusBar();
                gui.drawHelpScreen();
                break;
            case screens::SHUTDOWN_CONFIRM:
                gui.drawStatusBar();
                gui.drawShutdownConfirmScreen();
                break;
            default:
                // SHOULD NEVER ENTER HERE, ERROR OTHERWISE
                while (xSemaphoreTake(MUTEX_SPI, LOOP_TICKS) != pdTRUE);
                gui._tft.fillScreen(TFT_BLUE);  // blue screen of death
                gui._tft.setTextColor(TFT_WHITE);
                gui._tft.drawCentreString("BSOD", SCREEN_X_PIXELS/2, SCREEN_Y_PIXELS/2, 4);
                xSemaphoreGive(MUTEX_SPI);
                return; // forced reboot of the system
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
    // In the real system these should be read or acquired somehow

    SYSTEM_STATUS.battery_level_percentage = 100;
    SYSTEM_STATUS.battery_current_mA = 500;
    SYSTEM_STATUS.battery_power_mW = 1400;
    SYSTEM_STATUS.battery_total_capacity_mWh = 9600;
    SYSTEM_STATUS.battery_used_capacity_mWh = 0;
    SYSTEM_STATUS.battery_charging = true;
    SYSTEM_STATUS.battery_fully_charged = true;
    SYSTEM_STATUS.wifi_connected = true;
    SYSTEM_STATUS.wifi_rssi_dBm = -25;
    SYSTEM_STATUS.bt_connected = true;
    SYSTEM_STATUS.bt_rssi_dBm = -60;
    SYSTEM_STATUS.triggered = true;
    SYSTEM_STATUS.ready = false;

    // GUI INIT

    gui.init();  // initiates screen
    gui.calibrate();  // calibrates only if necessary

    //////////////////////////////////////////////////////////////////////////////////////
    // TASKS WHICH RUN ON CORE 0

    // Screen touch refresher
    xTaskCreatePinnedToCore(
        refreshTouch,               // Task function
        "refreshScreenTask",        // Task Name
        16384,                      // Stack depth in bytes
        NULL,                       // Task function parameters
        1,                          // Task priority
        &refreshTouch_TaskHandler,  // Task handler pointer 
        0);                         // Core in which to run task  

    // Screen manager
    xTaskCreatePinnedToCore(screenManager, "refreshManagerTask", 16384, NULL, 1, 
        &screenManager_TaskHandler, 0); 

    // Status bar refresher
    xTaskCreatePinnedToCore(statusBarRefresher, "statusBarRefreshTask", 16384, NULL, 1, 
        &statusBarRefresher_TaskHandler, 0); 

    //////////////////////////////////////////////////////////////////////////////////////
    // TASKS WHICH RUN ON CORE 1

    // ADC Sampler
    xTaskCreatePinnedToCore(sampler, "refreshManagerTask", 16384, NULL, 10, 
        &sampler_TaskHandler, 1);
    vTaskSuspend(sampler_TaskHandler);

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


