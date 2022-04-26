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


/**
 * @brief 
 * 
 */
GUI::GUI() 
{
    _tft.init();
    _tft.setRotation(SCREEN_ROTATION_VERTICAL);
    _tft.fillScreen(TFT_BLACK);

    GUI::_drawWelcomeScreen();  // Welcome screen: product name, logos, etc
    
    vTaskDelay(1000);  
    // 1 second delay to show welcome screen
    // TO-DO: Wait until ready state from another task.

    GUI::_drawMainScreen();  // main measurement screen
}


/* ---------------------------------------------------------------------------------------
 * Screen element drawers
 */

/**
 * @brief 
 * 
 */
void GUI::_drawStatusBar() {
    _tft.drawRect(0, 0, 240, 20, TFT_BLACK);  // black bar on top

    // TODO: access mutex structure from main.cpp containing system status parameters
    
    // System status icons
    if (_sys_status->battery_charging) {
        _tft.drawBitmap(BAT_CHARGING_ICON_X_POS,
                        BAT_CHARGING_ICON_Y_POS, 
                        &BAT_CHARGING_ICON_SPRITE,
                        BAT_CHARGING_ICON_WIDTH,
                        BAT_CHARGING_ICON_HEIGHT,
                        TFT_BLACK);
    }
    if (_sys_status->bt_connected) {
        _tft.drawBitmap(BT_ICON_X_POS,
                        BT_ICON_Y_POS, 
                        &BT_ICON_SPRITE,
                        BT_ICON_WIDTH,
                        BT_ICON_HEIGHT,
                        TFT_BLACK);
    }
    if (_sys_status->wifi_connected) {
        if (_sys_status->wifi_rssi_dBm >= WIFI_RSSI_BEST)
            wifi_icon_sprite = WIFI_ICON_SPRITE_BEST;
        else if (_sys_status->wifi_rssi_dBm >= WIFI_RSSI_GOOD)
            wifi_icon_sprite = WIFI_ICON_SPRITE_GOOD;
        else if (_sys_status->wifi_rssi_dBm >= WIFI_RSSI_MED)
            wifi_icon_sprite = WIFI_ICON_SPRITE_MED;
        else (_sys_status->wifi_rssi_dBm >= WIFI_RSSI_BAD)
            wifi_icon_sprite = WIFI_ICON_SPRITE_BAD;
        
        _tft.drawBitmap(WIFI_ICON_X_POS,
                        WIFI_ICON_Y_POS, 
                        &wifi_icon_sprite,
                        WIFI_ICON_WIDTH,
                        WIFI_ICON_HEIGHT,
                        TFT_BLACK);
    }
    

}


/* ---------------------------------------------------------------------------------------
 * Screen drawers
 */


/**
 * @brief 
 * 
 */
void GUI::_drawWelcomeScreen()
{
    _screenID = 0;
    _tft.fillScreen(TFT_WHITE);
    _tft.setTextColor(TFT_BLACK);
    _tft.drawCentreString("Tree Inspection Kit", 120, 50, 4);
    _tft.drawCentreString("µSecond precision timer", 120, 70, 2);
}


/**
 * @brief 
 * 
 */
void GUI::_drawMainScreen()
{
    _screenID = 1;
    _tft.fillScreen(TFT_WHITE);
}


/**
 * @brief 
 * 
 */
void GUI::_drawLoadingScreen()
{
    _screenID = 2;
    _tft.fillScreen(TFT_WHITE);
}


/**
 * @brief 
 * 
 */
void GUI::_drawMeasureScreen()
{
    _screenID = 3;
    _tft.fillScreen(TFT_WHITE);
}


/**
 * @brief 
 * 
 */
void GUI::_drawSettingsScreen()
{
    _screenID = 4;
    _tft.fillScreen(TFT_WHITE);
}


/**
 * @brief 
 * 
 */
void GUI::_drawGoodbyeScreen()
{
    _screenID = 5;
    _tft.fillScreen(TFT_BLACK);  // black screen
    _tft.setTextColor(TFT_WHITE);  // white text
    _tft.drawCentreString("GoodBye", 120, 120, 4);  // Center-up goodbye message
}



/**
 * @brief 
 * 
 */
void GUI::drawScreen() {
    _xTouchPos = _yTouchPos = 0;

    bool pressed = tft.getTouch(&_xTouchPos, &_yTouchPos);  // Touch detection everytime the screen is refreshed

}