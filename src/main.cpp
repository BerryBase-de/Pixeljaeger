#include "main.h"

void setup()
{
  mState = 1;
  mButtonState = LOW;
  mDirection = 1;
#if DEBUG
  Serial.begin(DEBUG_BAUDRATE);
#endif
  writeDebugMsg("----- Start setup -----");
  writeDebugMsg("Init LCD-Display ... ", false);

  mLCD.init();
  mLCD.backlight();
  mLCD.clear();

  writeDebugMsg(); //Problem here, no function to check if display is available
  writeDebugMsg("Init NeoPixel-Ring ... ", false);

  mPixel.begin(); //Init Pixel-Ring
  mPixel.clear(); //All neo pixel 'off'
  mPixel.setBrightness(10); //Brightness to 20%
  mPixel.show();  //Show update for each pixel

  writeDebugMsg(); //Problem here, no function to check, if NEO-Ring is available
  writeDebugMsg("Init buttons ... ", false);
  pinMode(BUTTON_P1, INPUT_PULLUP); //Button player one or switch in menu
  pinMode(PIN_RESET, INPUT_PULLUP); //This is a special one to reset EEPROM for highscore
  writeDebugMsg();
  writeDebugMsg("Some last inits ... ", false);
  randomSeed(analogRead(0));  // Make randome more randome ;)

  writeDebugMsg();
}

void loop()
{
  static int iRound = 0;
  static unsigned int iScore = 0;
  static int iWallPos = 0;
  static int iTargetPos = 0;
  static int iPlayerPos = 0;
  static int iInterval = 0;
  static unsigned long iLastPlayerMove = 0;
  int iDebounceButton = DebounceButton(); //Check and debounce button
  
  if(!bFirstRun)
  {
    static int iLastButtonState;
    if(mState == 1) //Startscreen
    {
      iRound = 1;
      iScore = 0;
      iInterval = STARTINTERVAL;
      mLCD.clear();
      mLCD.home();
      mLCD.print("Highscore: " + String(mStoredHighscore));
      mLCD.setCursor(0,1);
      mLCD.print("Press button ...");
      mState = 2;
    }
    else if(mState == 2) //Init new game
    {
      if(iDebounceButton == LOW)
      {
        if(iRound == 1) //Only show once during game
          Serial.println("-------- New game --------");
        mLCD.clear();
        mLCD.home();
        mLCD.print("Release button");
        mLCD.setCursor(0,1);
        mLCD.print("to start");
        iPlayerPos = random(0,NUMPIXELS-1);
        Serial.println("Player start pos: " + String(iPlayerPos));
        mState = 3; 
      }
    }
    else if(mState == 3) //Init next round
    {
      if(iDebounceButton == HIGH)
      {
        mLCD.clear();
        mLCD.home();
        mLCD.print("Round: " + String(iRound));
        Serial.println("Round: " + String(iRound));
        mLCD.setCursor(0,1);
        mLCD.print("Score: " + String(iScore));
        Serial.println("Score: " + String(iScore));
        //Get new wall position
        iWallPos = random(0,NUMPIXELS-1);
        while(abs(iWallPos - iPlayerPos) < SPACEPLAYERWALL)
          iWallPos = random(0,NUMPIXELS-1);
        Serial.println("Wall pos: " + String(iWallPos));
        //Get new target position
        iTargetPos = random(0,NUMPIXELS-1);
        while(iTargetPos == iWallPos || iTargetPos == iPlayerPos)
          iTargetPos = random(0,NUMPIXELS-1);
        Serial.println("New target pos: " + String(iTargetPos));
        Serial.println("Player pos: " + String(iPlayerPos));
        iLastButtonState = iDebounceButton;
        iLastPlayerMove = millis();  //Update timer for moving
        DrawGameOnLEDRing(iTargetPos, iWallPos, iPlayerPos);  //Draw game on LED-ring
        mState = 4;
      }
    }
    else if(mState == 4) //Player move
    {
      //Check if button was pressed
      if(iLastButtonState != iDebounceButton)
      {
        if(iDebounceButton == LOW)
          ChangePlayerDirection();
        iLastButtonState = iDebounceButton;
      }

      //Check if there is needed to redraw the LEDRing
      unsigned long currentMillis = millis();
      if(currentMillis - iLastPlayerMove > iInterval)
      {
        iPlayerPos += mDirection;
        if(iPlayerPos >= NUMPIXELS)
          iPlayerPos = 0;
        else if(iPlayerPos < 0)
          iPlayerPos = NUMPIXELS -1;
        iLastPlayerMove = currentMillis;
        DrawGameOnLEDRing(iTargetPos, iWallPos, iPlayerPos);  //Redraw LED-Ring
        Serial.println("Player pos: " + String(iPlayerPos));
      }
      if(iPlayerPos == iTargetPos)
        mState = 5; //Got the target
      if(iPlayerPos == iWallPos)
        mState = 90;  //Run against the wall
    }
    else if(mState == 5) //Player got target
    {
      iScore++; //Update score
      iRound++; //Update rounds
      mState = 3; //Get to next round
      iInterval = random(MAXSPEED,MINSPEED);  //New speed for player
      Serial.println("New interval: " + String(iInterval));
    }
    else if(mState == 90)  //Game over
    {
      Serial.println("Game ends");
      mLCD.clear();
      mLCD.home();
      iDebounceButton = HIGH;
      if(iScore > mStoredHighscore) //New highscore?
      {
        mLCD.print("New highscore ");
        mLCD.setCursor(0,1);
        mLCD.print("New score: " + String(iScore));
        Serial.println("New highscore is " + String(iScore));
        EEPROM.write(0,iScore); //Store new highscore to EEPROM
        mStoredHighscore = iScore;
      }
      else  //Loser
      {
        mLCD.print("Game Over");
        mLCD.setCursor(0,1);
        mLCD.print("You lose");
        Serial.println("You lose!");
      }
      Serial.println("-------- End game --------");
      delay(2000);
      mState = 1;
    }
    else
    {
      mState = 1;
    }
  }
  else  //Show init screen and test LED-ring
    InitFirstRun(); //Init Firstrun to check LCD and WS218B-ring


}

void writeDebugMsg(String msg, bool bNewLine)
{
#if DEBUG
  bNewLine == true ? Serial.println(msg) : Serial.print(msg);
#endif
}

void InitRingTest(int iRed, int iGreen, int iBlue)
{
  writeDebugMsg("R: " + String(iRed) + " G: " + String(iGreen) + " B: " + String(iBlue));
  for(int iPixel = 0; iPixel < NUMPIXELS; iPixel++)
  {
    mPixel.setPixelColor(iPixel, mPixel.Color(iRed, iGreen, iBlue));
    mPixel.show();
    delay(50);
  }
}

void CheckHighscore()
{
  if(mScore > mHighScore)
  {
    mHighScore = mScore;
    EEPROM.write(0, mHighScore);  //Write data to EEPROM
  }
}

int DebounceButton()
{
  static int iLastButtonPressed = HIGH;
  static unsigned long ulLastDebounceTime = 0;
  int iCurrentButtonState = digitalRead(BUTTON_P1);
  if(iCurrentButtonState != iLastButtonPressed)
    ulLastDebounceTime = millis();

  if((millis() - ulLastDebounceTime) > DEBOUNCETIME)
  {
    if(iCurrentButtonState != mButtonState)
      mButtonState = iCurrentButtonState;
  }
  iLastButtonPressed = iCurrentButtonState;
  return mButtonState;
}

/*
  =================================================================
  Function:     InitFirstRun
  Returns:      void
  Description:  Function to read and overwrite Highscore
                Also check LED-Ring
  =================================================================
*/
void InitFirstRun()
{
  if(digitalRead(PIN_RESET) == LOW) //Overwrite EEPROM with "0"
  {
    Serial.println("Reset highscore");
    for(unsigned int iCnt = 0; iCnt < EEPROM.length(); iCnt++)
      EEPROM.write(iCnt,0);
  }
  Serial.println("---- Start init ----");
  
  //Read latest highscore from EEPROM
  mStoredHighscore = EEPROM.read(0);
  Serial.println("Last stored highscore: " + String(mStoredHighscore));
  
  mLCD.home();
  mLCD.print("  Pixel chaser  ");
  Serial.println("Game Pixel chaser");
  mLCD.setCursor(0,1);
  mLCD.print("(c)  M3taKn1ght");
  Serial.print("(c)  M3taKn1ght");
  delay(1000);
  mLCD.clear();
  mLCD.home();
  mLCD.print("For Berrybase");
  Serial.println("For Berrybase");
  mLCD.setCursor(0,1);
  mLCD.print("Testing ring ...");
  Serial.println("Testing ring ...");
  delay(1000);
  mPixel.clear();
  //Check every single LED
  for(int i = 0; i<=255; i+=51)
  {
    InitRingTest(i,0,0);
    delay(50);
  }
  mPixel.clear();
  for(int i = 0; i<=255; i+=51)
  {
    InitRingTest(0,i,0);
    delay(50);
  }
  mPixel.clear();
  for(int i = 0; i<=255; i+=51)
  {
    InitRingTest(0,0,i);
    delay(50);
  }
  mPixel.clear();
  mPixel.show();
  Serial.println("----  End init  ----");
  bFirstRun = false;
  Serial.println("bFirstRun: " + String(bFirstRun));
  Serial.println("Activate state for game");
}

void DrawGameOnLEDRing(int &iTargetPos, int &iWallPos, int &iPlayerPos)
{
  mPixel.clear();
  mPixel.setPixelColor(iTargetPos, mPixel.Color(0, 0, 255));
  mPixel.setPixelColor(iWallPos, mPixel.Color(255, 0, 0));
  mPixel.setPixelColor(iPlayerPos, mPixel.Color(0, 255, 0));
  mPixel.show();
}

void ChangePlayerDirection()
{
  mDirection = -mDirection;
};