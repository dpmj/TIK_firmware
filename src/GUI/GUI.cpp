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
GUI::GUI(SystemStatus *sys_status, SemaphoreHandle_t *mutex_sys_status,
         touch_pos *touch_pos, SemaphoreHandle_t *mutex_touch_pos,
         screens *screen_id, SemaphoreHandle_t *mutex_spi) 
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
void GUI::drawStatusBar() 
{
    char bat_level[6];

    while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);
    while (xSemaphoreTake(*_mutex_sys_status, LOOP_TICKS) != pdTRUE);

    _tft.fillRect(0, 0, SCREEN_X_PIXELS, 20, TFT_BLACK);  // erase previous info

    // Bluetooth
    if (_sys_status->bt_connected) {
        _tft.pushImage(BT_ICON_X_POS, BT_ICON_Y_POS, BT_ICON_WIDTH, BT_ICON_HEIGHT, 
                       (uint8_t *) BT_ICON_SPRITE);
    }
    
    // Choose based on wifi signal strength
    if (_sys_status->wifi_connected) {
        if (_sys_status->wifi_rssi_dBm >= WIFI_RSSI_GOOD)
            wifi_icon_sprite = WIFI_GOOD_ICON_SPRITE;
        else if (_sys_status->wifi_rssi_dBm >= WIFI_RSSI_OKAY)
            wifi_icon_sprite = WIFI_OKAY_ICON_SPRITE;
        else
            wifi_icon_sprite = WIFI_BAD_ICON_SPRITE;
        
        _tft.pushImage(WIFI_ICON_X_POS, WIFI_ICON_Y_POS, WIFI_ICON_WIDTH, 
                       WIFI_ICON_HEIGHT, (uint8_t *) wifi_icon_sprite);
    }

    // Battery charging or not
    if (_sys_status->battery_charging) {
        _tft.pushImage(BAT_CH_ICON_X_POS, BAT_CH_ICON_Y_POS, BAT_CH_ICON_WIDTH, 
                       BAT_CH_ICON_HEIGHT, (uint8_t *) BAT_CH_ICON_SPRITE);
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
    
    _tft.pushImage(BAT_LEVEL_ICON_X_POS, BAT_LEVEL_ICON_Y_POS, BAT_LEVEL_ICON_WIDTH, 
                   BAT_LEVEL_ICON_HEIGHT, (uint8_t *) bat_level_icon_sprite); 

    // Battery level percentage
    sprintf(bat_level, "%d%%", _sys_status->battery_level_percentage);
    _tft.setTextFont(2);
    _tft.setTextSize(1);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.drawRightString(bat_level, BAT_LEVEL_ICON_X_POS - 5, 2, 2);

    // Page title
    switch (*_screen_id) {
        case screens::MAIN:
            _tft.drawString(TIK_FW_VERSION, 10, 2, 2);
            break;
        case screens::MEASURE:
            _tft.drawString("MEASURE", 10, 2, 2);
            break;
        case screens::SETTINGS:
            _tft.drawString("SETTINGS", 10, 2, 2);
            break;
        case screens::HELP:
            _tft.drawString("HELP", 10, 2, 2);
            break;
        // case screens::SHUTDOWN_CONFIRM:
        //     break;
        default:
            break;
    }

    xSemaphoreGive(*_mutex_spi);
    xSemaphoreGive(*_mutex_sys_status);

}



/* ---------------------------------------------------------------------------------------
 * Tools
 */

/**
 * @brief 
 * 
 */
void GUI::_clearScreen() 
{
    _tft.fillRect(0, STATUSBAR_HEIGHT, SCREEN_X_PIXELS, SCREEN_Y_PIXELS-STATUSBAR_HEIGHT, 
                  TFT_WHITE);
}


/**
 * @brief 
 * 
 * @param vector 
 * @param util 
 * @param start_index 
 * @param end_index 
 * @param x_graph_start 
 * @param x_graph_end 
 * @param y_graph_start 
 * @param y_graph_end 
 */
void GUI::_drawCurve(uint16_t *vector, uint16_t util, 
                     uint16_t start_index, uint16_t end_index, 
                     uint16_t x_graph_start, uint16_t x_graph_end, 
                     uint16_t y_graph_start, uint16_t y_graph_end) 
{
    uint16_t i, j, x_range, y_range, x_graph_range, y_graph_range, 
             x_pos, y_pos, x_pos_prev = 0, y_pos_prev = 0,
             y_max_index, y_max_value = 0, y_min_index, y_min_value = 65535;
    
    if (end_index > util) {
        end_index = util;
    }
    x_range = end_index - start_index;
    x_graph_range = x_graph_end - x_graph_start;

    // find y maximum, minimum indexes and values to scale it to the available space 
    for (i = start_index; i < end_index; i++) {
        if (vector[i] > y_max_value) {
            y_max_index = i;
            y_max_value = vector[i];
        }
        if (vector[i] < y_min_value) {
            y_min_index = i;
            y_min_value = vector[i];
        }
    }
    y_range = y_max_value - y_min_value;

    if (y_range == 0) {  // in case of no variation, center the line
        y_range = 3;
    }

    y_graph_range = y_graph_end - y_graph_start;

    // Serial.printf("\nx_range=%d; y_range=%d;\ny_max_value=%d; y_min_value=%d; y_graph_range=%d, y_graph_start:%d", 
    //               x_range, y_range, y_max_value, y_min_value, y_graph_range, y_graph_start);

    // Draw curve on the available area.
    while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);
    j = 0;
    for (i = start_index; i < end_index; i++) {
        x_pos = x_graph_start + (j * x_graph_range) / x_range;
        y_pos = y_graph_start + (y_graph_range - (vector[i] * y_graph_range) / y_range);
        if (!(x_pos == x_pos_prev && y_pos == y_pos_prev)) {
            if (j) {
                _tft.drawLine(x_pos_prev, y_pos_prev, x_pos, y_pos, TFT_RED);
            }
            _tft.fillCircle(x_pos, y_pos, 1, TFT_RED);
            // Serial.printf("\nFilled circle in pos: x_pos:%d; y_pos:%d", 
            //               x_pos, y_pos);
        }
        j++;
        x_pos_prev = x_pos;
        y_pos_prev = y_pos;
    }
    xSemaphoreGive(*_mutex_spi);
}



/**
 * @brief 
 * 
 * @param vector 
 * @param util 
 * @param start_index 
 * @param end_index 
 */
void GUI::drawCurveOnGraph1(uint16_t *vector, uint16_t util, 
                            uint16_t start_index, uint16_t end_index) {
    uint16_t x_graph_start = GRAPH_GRID_SIDE_MARGIN, 
             x_graph_end = GRAPH_GRID_SIDE_MARGIN + GRAPH_GRID_WIDTH,
             y_graph_start = GRAPH_FIRST_POS + GRAPH_TOP_MARGIN, 
             y_graph_end = GRAPH_FIRST_POS + GRAPH_TOP_MARGIN + GRAPH_GRID_HEIGHT;

    GUI::_drawCurve(vector, util, start_index, end_index, 
                    x_graph_start, x_graph_end, y_graph_start, y_graph_end);
}

/**
 * @brief 
 * 
 * @param vector 
 * @param util 
 * @param start_index 
 * @param end_index 
 */
void GUI::drawCurveOnGraph2(uint16_t *vector, uint16_t util, 
                            uint16_t start_index, uint16_t end_index) 
{
    uint16_t x_graph_start = GRAPH_GRID_SIDE_MARGIN, 
             x_graph_end = GRAPH_GRID_SIDE_MARGIN + GRAPH_GRID_WIDTH,
             y_graph_start = GRAPH_SECOND_POS + GRAPH_TOP_MARGIN, 
             y_graph_end = GRAPH_SECOND_POS + GRAPH_TOP_MARGIN + GRAPH_GRID_HEIGHT;

    GUI::_drawCurve(vector, util, start_index, end_index, 
                    x_graph_start, x_graph_end, y_graph_start, y_graph_end);
}



/**
 * @brief 
 * 
 * @param button 
 * @return true 
 * @return false 
 */
bool GUI::_buttonMonitor(TFT_eSPI_Button_Mod *button) 
{
    bool confirm_press = false;

    if (xSemaphoreTake(*_mutex_touch_pos, 0) == pdTRUE) {
        if (_touch_pos->pressed && button->contains(_touch_pos->x, _touch_pos->y)) {
            button->press(true);
        } else {
            button->press(false);
        }
    }
    xSemaphoreGive(*_mutex_touch_pos);

    while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);

    if (button->justPressed()) {  // wait till its released
        button->drawButton(true);  // draw invert
    }
    else if (button->justReleased()) {  
        button->drawButton(false);
        // confirm that has been pressed correctly
        if (button->contains(_touch_pos->x, _touch_pos->y)) {
            confirm_press = true;
        }
    }
    // do nothing if cursor is released outside the button perimeter
    xSemaphoreGive(*_mutex_spi);

    return confirm_press;
}



/**
 * @brief 
 * 
 */
void GUI::_drawGraphGrid(uint16_t y_pos,  // position
                         float x_lims[2],  // x axis limits. Format: [start, end]
                         float y1_lims[2],  // primary y axis limits
                         float y2_lims[2],  // secondary y axis limits
                         char *title, char *x_title, char *y1_title, char *y2_title)  // titles
{
    uint8_t i;  // iterator

    float x_ticks[GRAPH_X_TICKS];  // ticks of the x axis
    float y1_ticks[GRAPH_Y_TICKS];  // ticks of the primary y axis
    float y2_ticks[GRAPH_Y_TICKS];  // ticks of the secondary y axis

    char x_tick_i[GRAPH_TICKS_LENGTH];   // Auxiliary variables for sprintf
    char y1_tick_i[GRAPH_TICKS_LENGTH];
    char y2_tick_i[GRAPH_TICKS_LENGTH];

    // computing ticks
    for (i = 0; i < GRAPH_X_TICKS; i++)  {
        x_ticks[i] = x_lims[0] + (i*(x_lims[1]-x_lims[0])/(GRAPH_X_TICKS-1));
    }
    for (i = 0; i < GRAPH_Y_TICKS; i++)  {
        y1_ticks[i] = y1_lims[0] + (i*(y1_lims[1]-y1_lims[0])/(GRAPH_Y_TICKS-1));
        y2_ticks[i] = y2_lims[0] + (i*(y2_lims[1]-y2_lims[0])/(GRAPH_Y_TICKS-1));
    }

    _tft.fillRect(0, y_pos, GRAPH_AREA_WIDTH, GRAPH_AREA_HEIGHT, TFT_WHITE);
    y_pos += GRAPH_TOP_MARGIN;  // adjust origin for convenience

    for (i = 0; i < GRAPH_X_TICKS; i++) {
        _tft.drawLine(GRAPH_GRID_SIDE_MARGIN+(i*GRAPH_LINE_X_SEP), y_pos, 
                      GRAPH_GRID_SIDE_MARGIN+(i*GRAPH_LINE_X_SEP), y_pos+GRAPH_GRID_HEIGHT, 
                      TFT_LIGHTGREY);
        _tft.drawPixel(GRAPH_GRID_SIDE_MARGIN+(i*GRAPH_LINE_X_SEP), y_pos+GRAPH_GRID_HEIGHT+1, 
                       TFT_BLACK);

        if (i%2) {  // for minted x-axis ticks
            _tft.setTextColor(TFT_DARKGREY);
        } else {
            _tft.setTextColor(TFT_BLACK);
        }
        sprintf(x_tick_i, "%.2g", x_ticks[i]);
        _tft.drawCentreString(x_tick_i, GRAPH_GRID_SIDE_MARGIN+(i*GRAPH_LINE_X_SEP), 
                              y_pos+GRAPH_GRID_HEIGHT+3, 1);
    }
    _tft.drawCentreString(x_title, SCREEN_X_PIXELS/2, y_pos+GRAPH_GRID_HEIGHT+13, 1);

    for (i = 0; i < GRAPH_Y_TICKS; i++) {
        _tft.drawLine(GRAPH_GRID_SIDE_MARGIN, y_pos+(i*GRAPH_LINE_Y_SEP), 
                      GRAPH_GRID_SIDE_MARGIN+GRAPH_GRID_WIDTH, y_pos+(i*GRAPH_LINE_Y_SEP),
                      TFT_LIGHTGREY);
        _tft.drawPixel(GRAPH_GRID_SIDE_MARGIN-1, y_pos+(i*GRAPH_LINE_Y_SEP), 
                       TFT_BLUE);
        _tft.drawPixel(GRAPH_AREA_WIDTH-(GRAPH_GRID_SIDE_MARGIN-1), y_pos+(i*GRAPH_LINE_Y_SEP),
                       TFT_RED);

        sprintf(y1_tick_i, "%.3g", y1_ticks[(GRAPH_Y_TICKS-1)-i]);
        sprintf(y2_tick_i, "%.3g", y2_ticks[(GRAPH_Y_TICKS-1)-i]);

        _tft.setTextColor(TFT_BLUE);
        _tft.drawRightString(y1_tick_i, GRAPH_GRID_SIDE_MARGIN-1, y_pos+(i*GRAPH_LINE_Y_SEP)-3, 1);
        
        _tft.setTextColor(TFT_RED);
        _tft.drawString(y2_tick_i, GRAPH_AREA_WIDTH-(GRAPH_GRID_SIDE_MARGIN-3), y_pos+(i*GRAPH_LINE_Y_SEP)-3, 1);
    }

    _tft.setTextColor(TFT_BLUE);
    _tft.drawString(y1_title, 5, y_pos+GRAPH_GRID_HEIGHT+13, 1);
    _tft.setTextColor(TFT_RED);
    _tft.drawRightString(y2_title, SCREEN_X_PIXELS-5, y_pos+GRAPH_GRID_HEIGHT+13, 1);

    _tft.drawRect(GRAPH_GRID_SIDE_MARGIN, y_pos, GRAPH_GRID_WIDTH+1, GRAPH_GRID_HEIGHT+1, 
                  TFT_BLACK);
    _tft.drawLine(GRAPH_GRID_SIDE_MARGIN, y_pos, GRAPH_GRID_SIDE_MARGIN, y_pos+GRAPH_GRID_HEIGHT, 
                  TFT_BLUE);
    _tft.drawLine(GRAPH_AREA_WIDTH-GRAPH_GRID_SIDE_MARGIN, y_pos, GRAPH_AREA_WIDTH-GRAPH_GRID_SIDE_MARGIN, 
                  y_pos+GRAPH_GRID_HEIGHT, TFT_RED);

    _tft.setTextColor(TFT_BLACK);
    _tft.drawCentreString(title, SCREEN_X_PIXELS/2, y_pos-GRAPH_TOP_MARGIN+2, 1);

}


/**
 * @brief
 * 
 */
void GUI::_drawGraphs() {

    float x_lims[2] = {7, 70};  // Limits. Format: {start, end}. Both x lims are the same

    float g1_y1_lims[2] = {-20, 20};  // Different limits between graphs and y axis among
    float g1_y2_lims[2] = {-100, 100};  // the same graph
    
    float g2_y1_lims[2] = {-200, 0}; 
    float g2_y2_lims[2] = {-100, 0}; 

    GUI::_drawGraphGrid(GRAPH_FIRST_POS, x_lims, g1_y1_lims, g1_y2_lims,
                        "ACQUIRED WAVEFORMS", "Time (ms)", "Emitter (V)", "Receiver (mV)");
    GUI::_drawGraphGrid(GRAPH_SECOND_POS, x_lims, g2_y1_lims, g2_y2_lims,
                        "PROCESSED AKAIKE FUNCTIONS", "Time (ms)", "Emitter AIC", "Receiver AIC");

    _tft.drawLine(0, GRAPH_FIRST_POS-1, SCREEN_X_PIXELS, GRAPH_FIRST_POS-1, TFT_BLACK);
    _tft.drawLine(0, GRAPH_SECOND_POS-1, SCREEN_X_PIXELS, GRAPH_SECOND_POS-1, TFT_BLACK);
}


/* ---------------------------------------------------------------------------------------
 * Screen drawers
 */


/**
 * @brief 
 * 
 */
void GUI::drawMainScreen()
{
    TFT_eSPI_Button_Mod button_measure;
    TFT_eSPI_Button_Mod button_help;
    TFT_eSPI_Button_Mod button_shutdown;
    TFT_eSPI_Button_Mod button_settings;

    button_measure.initButton(&_tft, 
                               BUTTON_CENTER_X_POS,    // x
                               BUTTON_CENTER_Y_POS_1,  // y
                               BUTTON_CENTER_WIDTH,    // w
                               BUTTON_CENTER_HEIGHT,   // h
                               TFT_BLACK,              // outline 
                               TFT_BLUE,               // fill
                               TFT_WHITE,              // text
                               "Measure",              // label
                               4);                     // text font size        

    button_settings.initButton(&_tft,  BUTTON_CENTER_X_POS,  BUTTON_CENTER_Y_POS_2,  
                               BUTTON_CENTER_WIDTH,  BUTTON_CENTER_HEIGHT,  TFT_BLACK, 
                               TFT_BLUE, TFT_WHITE, "Settings", 4);
    button_help.initButton(&_tft,  BUTTON_CENTER_X_POS,  BUTTON_CENTER_Y_POS_3,  
                           BUTTON_CENTER_WIDTH,  BUTTON_CENTER_HEIGHT,  TFT_BLACK, 
                           TFT_BLUE, TFT_WHITE, "Help", 4);
    button_shutdown.initButton(&_tft,  BUTTON_CENTER_X_POS,  BUTTON_CENTER_Y_POS_4,  
                               BUTTON_CENTER_WIDTH,  BUTTON_CENTER_HEIGHT,  TFT_BLACK, 
                               TFT_RED, TFT_WHITE, "Shutdown", 4);

    while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);

    _tft.setTextFont(2);
    _tft.setTextSize(1);
    _tft.setTextColor(TFT_BLACK, TFT_WHITE);
    GUI::_clearScreen();

    _tft.drawCentreString("LIFE Wood For Future | GranaSat", TITLE_CENTER_X_POS, 
                          TITLE_CENTER_Y_POS + 30, 2);
    _tft.drawCentreString("University of Granada", TITLE_CENTER_X_POS, TITLE_CENTER_Y_POS + 45, 2);
    _tft.drawCentreString("Tree Inspection Kit", TITLE_CENTER_X_POS, TITLE_CENTER_Y_POS, 4);
    _tft.drawCentreString("JUAN DEL PINO MENA github.com/dpmj", TITLE_CENTER_X_POS, 
                          TITLE_CENTER_Y_POS + 65, 1);

    button_measure.drawButton();
    button_settings.drawButton();
    button_help.drawButton();
    button_shutdown.drawButton();

    xSemaphoreGive(*_mutex_spi);

    while (xSemaphoreTake(*_mutex_touch_pos, 0) != pdTRUE);
    _touch_pos->pressed = false;
    _touch_pos->x = 0;
    _touch_pos->y = 0;
    xSemaphoreGive(*_mutex_touch_pos);

    button_measure.press(false);
    button_settings.press(false);
    button_help.press(false);
    button_shutdown.press(false);

    bool active = true;    
    while(active) {
        // Check if any key coordinate boxes contain the touch coordinates
        vTaskDelay(25);

        if (_buttonMonitor(&button_measure)) {
            active = false;
            *_screen_id = screens::MEASURE;
        } else if (_buttonMonitor(&button_help)) {
            active = false;
            *_screen_id = screens::HELP;
        } else if (_buttonMonitor(&button_settings)) {
            active = false;
            *_screen_id = screens::SETTINGS;
        } else if (_buttonMonitor(&button_shutdown)) {
            active = false;
            *_screen_id = screens::SHUTDOWN_CONFIRM;
        }
    }
}



/**
 * @brief 
 * 
 */
void GUI::drawMeasureScreen(TaskHandle_t *sampler_taskHandler)
{
    TFT_eSPI_Button_Mod button_back;
    TFT_eSPI_Button_Mod button_clean;
    TFT_eSPI_Button_Mod button_draw;
    TFT_eSPI_Button_Mod button_zoom;
    TFT_eSPI_Button_Mod button_hist;
    
    button_back.initButton(&_tft, BUTTON_BACK_X_POS, BUTTON_BACK_Y_POS, BUTTON_BACK_WIDTH, 
                           BUTTON_BACK_HEIGHT, TFT_BLACK, TFT_RED, TFT_WHITE, "Back", 4);
    button_clean.initButton(&_tft, BUTTON_BACK_X_POS+BUTTON_BACK_WIDTH, 
                           BUTTON_BACK_Y_POS-BUTTON_BACK_HEIGHT/4-1, BUTTON_BACK_WIDTH, 
                           BUTTON_BACK_HEIGHT/2, TFT_BLACK, TFT_ORANGE, TFT_WHITE, "CLEAN", 2);
    button_draw.initButton(&_tft, BUTTON_BACK_X_POS+BUTTON_BACK_WIDTH, 
                           BUTTON_BACK_Y_POS+BUTTON_BACK_HEIGHT/4, BUTTON_BACK_WIDTH, 
                           BUTTON_BACK_HEIGHT/2, TFT_BLACK, TFT_BLUE, TFT_WHITE, "DRAW", 2);
    button_zoom.initButton(&_tft, BUTTON_BACK_X_POS+BUTTON_BACK_WIDTH*1.75, BUTTON_BACK_Y_POS, 
                           BUTTON_BACK_WIDTH/2, BUTTON_BACK_HEIGHT, TFT_BLACK, TFT_LIGHTGREY, 
                           TFT_WHITE, "Z", 4);
    button_hist.initButton(&_tft, BUTTON_BACK_X_POS+BUTTON_BACK_WIDTH*2.25, BUTTON_BACK_Y_POS, 
                           BUTTON_BACK_WIDTH/2, BUTTON_BACK_HEIGHT, TFT_BLACK, TFT_LIGHTGREY, 
                           TFT_WHITE, "H", 4);

    while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);
    GUI::_clearScreen();
    button_back.drawButton();
    button_clean.drawButton();
    button_draw.drawButton();
    button_zoom.drawButton();
    button_hist.drawButton();
    GUI::_drawGraphs();
    xSemaphoreGive(*_mutex_spi);

    while (xSemaphoreTake(*_mutex_touch_pos, 0) != pdTRUE);
    _touch_pos->pressed = false;
    _touch_pos->x = 0;
    _touch_pos->y = 0;
    xSemaphoreGive(*_mutex_touch_pos);

    button_back.press(false);

    bool active = true;
    while(active) {
        if (GUI::_buttonMonitor(&button_back)) {
            active = false;
            *_screen_id = screens::MAIN;
        }
        if (GUI::_buttonMonitor(&button_clean)) {
            active = true;
            while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);
                GUI::_drawGraphs();
            xSemaphoreGive(*_mutex_spi);
        }
        if (GUI::_buttonMonitor(&button_draw)) {
            vTaskResume(*sampler_taskHandler);
            active = true;
        }
        if (GUI::_buttonMonitor(&button_zoom)) {
            active=true;
        }
        if (GUI::_buttonMonitor(&button_hist)) {
            active = false;
            *_screen_id = screens::MEASURE_HISTORIC;
        }
    }
}



/**
 * @brief 
 * 
 */
void GUI::drawSettingsScreen()
{
    TFT_eSPI_Button_Mod button_calibration;
    TFT_eSPI_Button_Mod button_language;
    TFT_eSPI_Button_Mod button_back;

    button_back.initButton(&_tft, BUTTON_BACK_X_POS, BUTTON_BACK_Y_POS, BUTTON_BACK_WIDTH, 
                           BUTTON_BACK_HEIGHT, TFT_BLACK, TFT_RED, TFT_WHITE, "Back", 4);

    while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);
    
    GUI::_clearScreen();
    _tft.setTextColor(TFT_BLACK);
    _tft.setTextFont(2);
    _tft.setTextSize(1);
    _tft.drawCentreString("Settings", TITLE_CENTER_X_POS, TITLE_CENTER_Y_POS, 4);
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
        vTaskDelay(25);
        if (_buttonMonitor(&button_back)) {
            active = false;
            *_screen_id = screens::MAIN;
        }
    }
}



/**
 * @brief 
 * 
 */
void GUI::drawHelpScreen()
{
    TFT_eSPI_Button_Mod button_calibration;
    TFT_eSPI_Button_Mod button_language;
    TFT_eSPI_Button_Mod button_back;

    button_back.initButton(&_tft, BUTTON_BACK_X_POS, BUTTON_BACK_Y_POS, BUTTON_BACK_WIDTH, 
                           BUTTON_BACK_HEIGHT, TFT_BLACK, TFT_RED, TFT_WHITE, "Back", 4);

    while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);
    
    GUI::_clearScreen();
    _tft.setTextColor(TFT_BLACK);
    _tft.setTextFont(2);
    _tft.setTextSize(1);
    _tft.drawCentreString("Help", TITLE_CENTER_X_POS, TITLE_CENTER_Y_POS, 4);
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
        vTaskDelay(25);
        if (_buttonMonitor(&button_back)) {
            active = false;
            *_screen_id = screens::MAIN;
        }
    }
}


/**
 * @brief 
 * 
 */
void GUI::drawShutdownConfirmScreen()
{
    TFT_eSPI_Button_Mod button_calibration;
    TFT_eSPI_Button_Mod button_language;
    TFT_eSPI_Button_Mod button_back;

    button_back.initButton(&_tft, BUTTON_BACK_X_POS, BUTTON_BACK_Y_POS, BUTTON_BACK_WIDTH, 
                           BUTTON_BACK_HEIGHT, TFT_BLACK, TFT_RED, TFT_WHITE, "Cancel", 4);

    while (xSemaphoreTake(*_mutex_spi, LOOP_TICKS) != pdTRUE);
    
    GUI::_clearScreen();
    _tft.setTextColor(TFT_BLACK);
    _tft.setTextFont(2);
    _tft.setTextSize(1);
    _tft.drawCentreString("Shutdown", TITLE_CENTER_X_POS, TITLE_CENTER_Y_POS, 4);
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
        vTaskDelay(25);
        if (_buttonMonitor(&button_back)) {
            active = false;
            *_screen_id = screens::MAIN;
        }
    }
}
