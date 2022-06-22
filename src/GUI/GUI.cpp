/**
 * @file GUI.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-04-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "GUI.h"


//#define FONT_FREESANSBOLD &FreeSansBold9pt7b

/**
 * @brief 
 * 
 */
GUI::GUI(SystemStatus *sys_status, SemaphoreHandle_t *mutex_sys_status,
         touch_pos *touch_pos, SemaphoreHandle_t *mutex_touch_pos,
         screens *screen_id, //SemaphoreHandle_t *mutex_screen_id,
         SemaphoreHandle_t *mutex_spi
        ) 
{
    _sys_status = sys_status; 
    _mutex_sys_status = mutex_sys_status;
    _touch_pos = touch_pos;
    _mutex_touch_pos = mutex_touch_pos;
    _screen_id = screen_id;
    //_mutex_screen_id = mutex_screen_id;
    _mutex_spi = mutex_spi;
}


/**
 * @brief 
 * 
 */
void GUI::init()
{
    while(xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);
    _tft.setRotation(SCREEN_ROTATION);
    _tft.init();
    _tft.fillScreen(TFT_WHITE);
    xSemaphoreGive(*_mutex_spi);
}



/**
 * @brief 
 * 
 */
void GUI::calibrate() 
{
    uint16_t calData[5];
    uint8_t calDataOK = 0;

    // check file system exists
    if (!SPIFFS.begin()) {
        //Serial.println("Formatting file system");
        SPIFFS.format();
        SPIFFS.begin();
    }

    // check if calibration file exists and size is correct
    if (SPIFFS.exists(CALIBRATION_FILE)) {
        if (REPEAT_CAL) {
            // Delete if we want to re-calibrate
            SPIFFS.remove(CALIBRATION_FILE);
        }
        else {
            File f = SPIFFS.open(CALIBRATION_FILE, "r");
            if (f) {
                if (f.readBytes((char *)calData, 14) == 14) {
                    calDataOK = 1;
                }
                f.close();
            }
        }
    }

    if (calDataOK && !REPEAT_CAL) {
        // calibration data valid
        _tft.setTouch(calData);
    } 
    else {
        // data not valid so recalibrate
        _tft.fillScreen(TFT_BLACK);
        _tft.setCursor(20, 0);
        _tft.setTextFont(2);
        _tft.setTextSize(1);
        _tft.setTextColor(TFT_WHITE, TFT_BLACK);

        _tft.println("Touch corners as indicated");

        _tft.setTextFont(1);
        _tft.println();

        if (REPEAT_CAL) {
            _tft.setTextColor(TFT_RED, TFT_BLACK);
            _tft.println("Set REPEAT_CAL to false to stop this running again!");
        }

        _tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

        _tft.setTextColor(TFT_GREEN, TFT_BLACK);
        _tft.println("Calibration complete!");

        // store data
        File f = SPIFFS.open(CALIBRATION_FILE, "w");
        if (f) {
            f.write((const unsigned char *)calData, 14);
            f.close();
        }
    }
}




/**
 * @brief Returns whether the display is available or not
 * 
 * @return true cannot use the screen, being used by an internal function
 * @return false can use the screen without problems
 */


/* ---------------------------------------------------------------------------------------
 * Screen element drawers
 */

const uint8_t *wifi_icon_sprite = WIFI_GOOD_ICON_SPRITE;
const uint8_t *bat_level_icon_sprite = BAT_LEVEL_HIGH_ICON_SPRITE;

/**
 * @brief 
 * 
 */
void GUI::drawStatusBar() {
    _tft.fillRect(0, 0, SCREEN_X_PIXELS, 20, TFT_BLACK);  // black bar on top
    // TODO: access mutex structure from main.cpp containing system status parameters

    //_spr_bt.setColorDepth(4);
    //_spr_bt.createSprite(BT_ICON_WIDTH+1, BT_ICON_HEIGHT+1);
    //_spr_bt.fillSprite(TFT_PINK);
    //_spr.setBitmapColor(TFT_WHITE, TFT_BLACK);

    // Bluetooth
    if (_sys_status->bt_connected) {
        _tft.pushImage(BT_ICON_X_POS, 
                       BT_ICON_Y_POS, 
                       BT_ICON_WIDTH, 
                       BT_ICON_HEIGHT, 
                       (uint8_t *) BT_ICON_SPRITE);
        // _tft.drawBitmap(BT_ICON_X_POS,
        //                 BT_ICON_Y_POS, 
        //                 BT_ICON_SPRITE,
        //                 BT_ICON_WIDTH,
        //                 BT_ICON_HEIGHT,
        //                 TFT_WHITE);

        //_spr_bt.pushSprite(BT_ICON_X_POS, BT_ICON_Y_POS);
    }
    
    // Choose based on wifi signal stregth
    if (_sys_status->wifi_connected) {
        if (_sys_status->wifi_rssi_dBm >= WIFI_RSSI_GOOD)
            wifi_icon_sprite = WIFI_GOOD_ICON_SPRITE;
        else if (_sys_status->wifi_rssi_dBm >= WIFI_RSSI_OKAY)
            wifi_icon_sprite = WIFI_OKAY_ICON_SPRITE;
        else
            wifi_icon_sprite = WIFI_BAD_ICON_SPRITE;
        
        _tft.pushImage(WIFI_ICON_X_POS,
                       WIFI_ICON_Y_POS,
                       WIFI_ICON_WIDTH,
                       WIFI_ICON_HEIGHT,
                       (uint8_t *) wifi_icon_sprite);
    }

    // Battery charging or not
    if (_sys_status->battery_charging) {
        _tft.pushImage(BAT_CH_ICON_X_POS,
                       BAT_CH_ICON_Y_POS, 
                       BAT_CH_ICON_WIDTH,
                       BAT_CH_ICON_HEIGHT,
                       (uint8_t *) BAT_CH_ICON_SPRITE);
    }

    // Choose based on battery level
    if (_sys_status->battery_level_percentage >= BAT_LEVEL_HIGH)
        bat_level_icon_sprite = BAT_LEVEL_HIGH_ICON_SPRITE;
    else if (_sys_status->battery_level_percentage >= BAT_LEVEL_OKAY)
        bat_level_icon_sprite = BAT_LEVEL_OKAY_ICON_SPRITE;
    else if (_sys_status->battery_level_percentage >= BAT_LEVEL_MED)
        bat_level_icon_sprite = BAT_LEVEL_MED_ICON_SPRITE;
    else
        bat_level_icon_sprite = BAT_LEVEL_LOW_ICON_SPRITE;
    
    _tft.pushImage(BAT_LEVEL_ICON_X_POS,
                   BAT_LEVEL_ICON_Y_POS, 
                   BAT_LEVEL_ICON_WIDTH,
                   BAT_LEVEL_ICON_HEIGHT,
                   (uint8_t *) bat_level_icon_sprite); 
}



/* ---------------------------------------------------------------------------------------
 * Tools
 */





/* ---------------------------------------------------------------------------------------
 * Screen drawers
 */


/**
 * @brief 
 * 
 */
void GUI::drawWelcomeScreen()
{
    TFT_eSPI_Button_Mod button_continue;

    //*_screen_id = WELCOME;

    while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);

    _tft.setTextFont(2);
    _tft.setTextSize(1);
    _tft.setTextColor(TFT_BLACK, TFT_WHITE);
    _tft.fillScreen(TFT_WHITE);

    drawStatusBar();

    _tft.drawCentreString("Tree Inspection Kit", 120, 70, 4);
    _tft.drawCentreString("MODULE 1", 120, 100, 2);
    _tft.drawCentreString("FW ver 0.1-20220622", 120, 240, 2);
    _tft.drawCentreString("Juan Del Pino Mena", 120, 270, 2);
    _tft.drawCentreString("University of Granada", 120, 300, 2);

    button_continue.initButton(&_tft, 
                               SCREEN_X_PIXELS/2,  // x
                               SCREEN_Y_PIXELS/2,  // y
                               100, // w
                               50, // h
                               TFT_BLACK,  // outline 
                               TFT_BLUE,   // fill
                               TFT_WHITE,  // text
                               "CONTINUE",  // label
                               1);  // text size
    button_continue.drawButton();

    xSemaphoreGive(*_mutex_spi);

    while (xSemaphoreTake(*_mutex_touch_pos, 0) != pdTRUE);
    _touch_pos->pressed = false;
    _touch_pos->x = 0;
    _touch_pos->y = 0;
    xSemaphoreGive(*_mutex_touch_pos);
    button_continue.press(false);

    bool active = true;    
    while(active) {
        // Check if any key coordinate boxes contain the touch coordinates

        if (xSemaphoreTake(*_mutex_touch_pos, 0) == pdTRUE) {
            if (_touch_pos->pressed && button_continue.contains(_touch_pos->x, _touch_pos->y)) {
                button_continue.press(true);
            } else {
                button_continue.press(false);
            }
        }
        xSemaphoreGive(*_mutex_touch_pos);

        while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);
        if (button_continue.justPressed()) {  // wait till its released
            button_continue.drawButton(true);  // draw invert
        }
        else if (button_continue.justReleased()) {  // do something
            //button_continue.drawButton();  // draw normal.
            active = false;
            *_screen_id = screens::MEASURE;  // NEXT SCREEN
        }
        xSemaphoreGive(*_mutex_spi);
        vTaskDelay(27);
    }
}



// /**
//  * @brief 
//  * 
//  */
// void GUI::_drawMainScreen()
// {
//     _sys_status->current_screen = MAIN;
//     _tft.fillScreen(TFT_WHITE);
//     drawStatusBar();
// }



// /**
//  * @brief 
//  * 
//  */
// void GUI::_drawLoadingScreen()
// {
//     _sys_status->current_screen = LOADING;
//     _tft.fillScreen(TFT_WHITE);
//     _tft.setTextColor(TFT_BLACK);  // white text
//     _tft.drawCentreString("Loading", 120, 120, 4);
// }



/**
 * @brief 
 * 
 */
void GUI::drawMeasureScreen()
{
    TFT_eSPI_Button_Mod button_back;

    while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);
    
    _tft.fillScreen(TFT_WHITE);  // white screen
    drawStatusBar();
    _tft.setTextColor(TFT_BLACK);  // black text
    _tft.drawCentreString("Measure", 120, 120, 4);
    
    button_back.initButton(&_tft, 
                            SCREEN_X_PIXELS/2,  // x
                            200,  // y
                            100, // w
                            50, // h
                            TFT_BLACK,  // outline 
                            TFT_RED,   // fill
                            TFT_WHITE,  // text
                            "BACK",  // label
                            1);  // text size
    button_back.drawButton();

    xSemaphoreGive(*_mutex_spi);

    while (xSemaphoreTake(*_mutex_touch_pos, 0) != pdTRUE);
    _touch_pos->pressed = false;
    _touch_pos->x = 0;
    _touch_pos->y = 0;
    xSemaphoreGive(*_mutex_touch_pos);
    button_back.press(false);

    bool active = true;
    while(active) {
        // Check if any key coordinate boxes contain the touch coordinates

        if (xSemaphoreTake(*_mutex_touch_pos, 0) == pdTRUE) {
            if (_touch_pos->pressed && button_back.contains(_touch_pos->x, _touch_pos->y)) {
                button_back.press(true);
            } else {
                button_back.press(false);
            }
            xSemaphoreGive(*_mutex_touch_pos);
        }

        while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);
        if (button_back.justPressed()) {  // wait till its released
            button_back.drawButton(true);  // draw invert
        }
        else if (button_back.justReleased()) {  // do something
            //button_back.drawButton();  // draw normal.
            active = false;
            *_screen_id = screens::WELCOME;  // NEXT SCREEN
        }
        xSemaphoreGive(*_mutex_spi);
        vTaskDelay(27);
    }
}



// /**
//  * @brief 
//  * 
//  */
// void GUI::_drawSettingsScreen()
// {
//     _sys_status->current_screen = SETTINGS;
//     _tft.fillScreen(TFT_WHITE);
//     _tft.setTextColor(TFT_BLACK);  // white text
//     _tft.drawCentreString("Settings", 120, 120, 4);
// }



// /**
//  * @brief 
//  * 
//  */
// void GUI::_drawHelpScreen()
// {
//     _sys_status->current_screen = HELP;
//     _tft.fillScreen(TFT_BLACK);  // black screen
//     _tft.setTextColor(TFT_WHITE);  // white text
//     _tft.drawCentreString("Help", 120, 120, 4);



// /**
//  * @brief 
//  * 
//  */
// void GUI::_drawGoodbyeScreen()
// {
//     _sys_status->current_screen = GOODBYE;
//     _tft.fillScreen(TFT_BLACK);  // black screen
//     _tft.setTextColor(TFT_WHITE);  // white text
//     _tft.drawCentreString("GoodBye", 120, 120, 4);
// }
