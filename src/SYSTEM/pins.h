/**
 * @file pins.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-07-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

// PIN DEFINITIONS FOR THE SPI TFT DISPLAY

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)

#define TOUCH_CS 12  // Chip select pin (T_CS) of touch screen
#define PIN_SD_CS 5

#define SPI_FREQUENCY  80000000  // High speed for smoother operation. Max: 80 MHz
#define SPI_READ_FREQUENCY  20000000  // Optional reduced SPI frequency for reading TFT
#define SPI_TOUCH_FREQUENCY  2500000  // The XPT2046 requires a lower SPI clock rate of 2.5MHz so we define that here:

// PIN DEFINITIONS FOR THE ANALOG SIGNALS

#define PIN_EMITTER_AMP 36
#define PIN_RECEIVER_AMP 39

// PIN DEFINITIONS FOR THE HX711

#define PIN_HX_DATA 35
#define PIN_HX_SCK 25

// PIN DEFINITIONS FOR THE EXPANSION PORT

#define PIN_EXP_PORT_1 32
#define PIN_EXP_PORT_2 33

// PIN DEFINITIONS FOR BUTTONS

#define PIN_POWER_SW_USER 34
#define PIN_POWER_SW_HOLD 26

#define PIN_THUMB_SW_CCW 27
#define PIN_THUMB_SW_PUSH 14
#define PIN_THUMB_SW_CW 13

#define PIN_GPIO0 0

#define PIN_SW_EXP 8
#define PIN_SW_HX 6

// PIN DEFINITIONS FOR BATTERY CHARGER STATUS

#define PIN_BAT_CHRG 16
#define PIN_BAT_STBY 17