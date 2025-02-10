//Start with needed included
#include <Arduino.h>
#include "Wire.h"
#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

enum EnumStatus
{
        StartScreen = 0,
        InitNewGame = 1,

        GameOver = 90
};
//Functions
/*
* =================================================================
* Function:     setup
* Description:  Setup display and sensors   
* Returns:      void
* =================================================================
*/
void setup();

/** main-loop, for every cyclus
*
* Main loop, that will execute every cyclus
* @returns void
*/
void loop();

/** Write debug-message to serial monitor.
*
* If debug-mode is active, the message will written to serial monitor
*
* @param msg Message for the serial monitor, default "DONE"
* @param bNewLine Message ends with new line or next message will shown in same line
* @returns void
*/
void writeDebugMsg(String msg = "DONE", bool bNewLine = true);

/** Draw some color on the NEOPIXEL-Ring to check, if everything is okay.
*
* This function is only needed for the player, to check during the init, if all NEOPIXEL-
* elements are working or some are demaged in color.
*
* @param iRed Color value for red
* @param iGreen Color value for green
* @param iBlue Color value for blue
* @returns void
*/
void InitRingTest(int iRed = 0, int iGreen = 0, int iBlue = 0);

/** Check the current score with the highscore.
*
* If score from player ist higher than stored highscore, the data will overwrite
* and will be stored in EEPROM
*
* @returns void
*/
void CheckHighscore();

/**
 * Debounce button, so code get clear signal
 * @returns int
*/
int DebounceButton();

/**
 * Function to read and overwrite Highscore
 * Also check LED-Ring
 * @returns void
*/
void InitFirstRun();

/**
  * Change direction from player
  * @returns void
*/
void ChangePlayerDirection();


/**
  * Redraw every current position for:
                - target
                - wall
                - player dot
  * @returns void
  
*/
void DrawGameOnLEDRing(int &iTargetPos, int &iWallPos, int &iPlayerPos);

/*
* -------------------
* ---- Variables ----
* -------------------
*/
LiquidCrystal_I2C mLCD(LCD_ADDRESS,LCD_COLUMNS,LCD_ROWS); // Global variable for the LCD-Display
Adafruit_NeoPixel mPixel(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800); // NeoPixel object
unsigned int mHighScore = 0; //Stored highscore from EEPROM
unsigned int mScore = 0;
bool bFirstRun = true;
unsigned int mStoredHighscore = 0;
int mState;
int mButtonState;
int mDirection; //Clockwise "-1" is counterclockwise
