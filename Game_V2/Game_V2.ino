#include "types.h"
#include <TinyGPS++.h>
#include <HardwareSerial.h>

#include <Wire.h>
#include "SSD1306.h"
#include "oled_display.h"
//#include <MPU9255.h>
#include "MPU9250.h"
MPU9250 mpu(Wire,0x68);

//files
const char characterSheet = "Character_Sheet"; //holds data on user name, stats and such
const char moveSheet = "Move_Sheet"; //holds data on their moves and casting as well as damage and such
const char bag = "bag"; //holds data on what is in their bag
const char items = "items"; //used for basic info on items and their effects

//buttons
const int enterButton = 35; 
const int leftButton = 34;
const int rightButton = 32;
//record when the button was pressed so we can possibly allow scrolling or other actions (like accessing settings)
int holdingLeft; 
int holdingEnter;
int holdingRight;
//button states
int leftButtonState = 0;
bool leftButtonPushed = false;
int rightButtonState = 0;
bool rightButtonPushed = false;
int enterButtonState = 0;
bool enterButtonPushed = false;
int holdingTime = 3000;//how long it takes before action is taken (like scrolling)  in millis

//screen
bool loaded = false; //used to tell if all the data has been loaded and such. until then it ignores input
uint8_t screen = STARTUP;
bool justFlipped = false;

//sd card
#include "FS.h"
#include "SD.h"
#include "SPI.h"

//saved data
const int numDataPoints = 3;
String variables[numDataPoints] = 
{
  "username",
  "species",
  "race"
};
//const int numMaxMoves = 8;
//String moveNotes;
//String moves[numMaxMoves];
//int moveDamage[numMaxMoves];
//String moveEffect[numMaxMoves];
//float moveRange[numMaxMoves];
//const int numMaxBagItems = 16;
//String bagNotes;
//int bagSlot[numMaxBagItems];
//String bagData[numMaxBagItems]; //item name to be looked up in the item list
//const int numOfGameItems = 64;
//String items[numOfGameItems];
//int itemDamage[numOfGameItems];
//String itemEffect[numOfGameItems];
//float itemRange[numOfGameItems];
//String itemNotes;

String savedData[numDataPoints];
String characterNotes; //data saved at the top of the character sheet for human only reading, just saving it here so it is easy to rewrite back in

#define OLED_ADDR 0x3C
#define OLED_SDA 21
#define OLED_SCL 22

TaskHandle_t RadioIO;

double Long;
double Lat;

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
SSD1306 display(OLED_ADDR, OLED_SDA, OLED_SCL);
OledDisplay oledDisplay(&display);

GpsDataState_t gpsState = {};

#define TASK_OLED_RATE 200
#define TASK_SERIAL_RATE 100

uint32_t nextSerialTaskTs = 0;
uint32_t nextOledTaskTs = 0;

//variables that hold the data for the mpu
float accelX;
float accelY;
float accelZ;
float gyroX;
float gyroY;
float gyroZ;
float magX;
float magY;
float magZ;

double roll , pitch, yaw;
#define g 9.81
#define magnetometer_cal 0.06




//do i have data incoming?
String incomingData;
bool dataIncoming = false;
//do i have data to transmit?
String outgoingData;
bool dataToTransmit = false;
//need to loop this forever, it is the same thing as loop()
//use global variables to pass data between cores, make sure it is only read by one and written by the other
void radioIOCode( void * parameter )
{
  for(;;)
  {
  }
}

//updates the file with all the data pulled from the files so any changes are saved for the next startup
//add more files and variables as needed.
void updateFiles(fs::FS &fs)
{
  
  //update character sheet
  fs.remove(characterSheet);
  File file = fs.open(characterSheet, FILE_WRITE);
  file.println(characterNotes);
  file.println("*/");
  for (int i = 0; i < numDataPoints; i++)
  {
    String temp = variables[i] + ": " + savedData[i];
    file.println(temp);  
  }
  file.close();
//  
//  //update move sheet
//  fs.remove(moveSheet);
//  File file = fs.open(moveSheet, FILE_WRITE);
//  file.println(moveNotes);
//  file.println("*/");
//  for (int i = 0; i < numMaxMoves; i++)
//  {
//    String temp = moves[i] + ": " + moveDamage[i] + ". " + moveEffect[i] +  "' " + moveRange[i];
//    file.println(temp);  
//  }
//  file.close();
//  
//  //update bag
//  fs.remove(bag);
//  File file = fs.open(bag, FILE_WRITE);
//  file.println(bagNotes);
//  file.println("*/");
//  for (int i = 0; i < numMaxBagItems; i++)
//  {
//    String temp = bagSlot[i] + ": " + bagData[i];
//    file.println(temp);  
//  }
//  file.close();
//  
//  //update items
//  fs.remove(items);
//  File file = fs.open(items, FILE_WRITE);
//  file.println(itemNotes);
//  file.println("*/");
//  for (int i = 0; i < numOfGameItems; i++)
//  {
//    String temp = items[i] + ": " + itemInfo[i] + ". " + itemEffect[i] +  "' " + itemRange[i];
//    file.println(temp);  
//  }
//  file.close();
//  
}

//search through all the variables and sort them
void sortDataCharacterSheet(String inputData[], String inputVariables[])
{
  for(int i = 0; i < numDataPoints; i++) //for keeping track of the variables
  {
    for(int x = 0; x < numDataPoints; x++)//for keeping track of the input variables
    {
      if(inputVariables[x].equalsIgnoreCase( variables[i]))
      {
        savedData[i] = inputData[x];
      }
    }
  }
  /*Example of this:
   * blue 23
   * red 83
   * green 104
   * 
   * into a array in red green blue order
   * 
   * red 83
   * green 104
   * blue 23
   * 
   */
}


//grabs all the data saved in the sd card
//add more files and variables as needed
void retrieveData(fs::FS &fs, const char * path)
{
  String savedDataTemp[numDataPoints];
  String variablesTemp[numDataPoints];
  //do this everytime the program starts then check to make sure that all the data is in the right order before saving it to the data area
  File file = fs.open(path);
  notes = file.readStringUntil('*/');
  for (int i = 0; i < numDataPoints; i++)
  {
    variablesTemp[i] = file.readStringUntil(':');
    file.readStringUntil(' ');
    savedDataTemp[i] = file.readStringUntil('\n');
    Serial.println(variables[i] + " is " + savedData[i]);
  }
  sortData(savedDataTemp, variablesTemp);
}

void gpsUpdate() //update the gps cordinates and save them to global variables so they can be used in multiple areas
{
    static int p0 = 0;

    while (SerialGPS.available() > 0) {
        gps.encode(SerialGPS.read());
    }

    if (gps.satellites.value() > 4) {


        if (gps.altitude.meters() > gpsState.altMax) {
            gpsState.altMax = gps.altitude.meters();
        }

        if (gps.speed.mps() > gpsState.spdMax) {
            gpsState.spdMax = gps.speed.mps();
        }

        if (gps.altitude.meters() < gpsState.altMin) {
            gpsState.altMin = gps.altitude.meters();
        }
    }

    if (nextSerialTaskTs < millis()) {
      double tmp = gps.location.lng();
      if (!isnan(tmp))
      {
        Long = tmp;
      }
      tmp = gps.location.lat();
      if (!isnan(tmp))
      {
        Lat = tmp;
      }
      /*
      Serial.print(" ");
      Serial.print(Lat, 6);
      Serial.print(" ");
      Serial.println(Long, 6);
        
        Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
        Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
        Serial.print("LAT AVG=");  Serial.println(avgLat, 6);
        Serial.print("LONG AVG="); Serial.println(avgLong, 6);
        Serial.print("ALT=");  Serial.println(gps.altitude.meters());
        Serial.print("Sats=");  Serial.println(gps.satellites.value());
//*/
        nextSerialTaskTs = millis() + TASK_SERIAL_RATE;
    }
}

void mpuUpdate() //update the mpu data
{
  mpu.readSensor();

  //get the mpu data
  accelX = mpu.getAccelX_mss();
  accelY = mpu.getAccelY_mss();
  accelZ = mpu.getAccelZ_mss();
  gyroX = mpu.getGyroX_rads()/57.3;
  gyroY = mpu.getGyroY_rads()/57.3;
  gyroZ = mpu.getGyroZ_rads()/57.3;
  magX = mpu.getMagX_uT();
  magY = mpu.getMagY_uT();
  magZ = mpu.getMagZ_uT();
  //calculate the pitch, roll and yaw (tilt up/down, twist, and turning left/right in that order)
  pitch = atan2 (accelY ,( sqrt ((accelX * accelX) + (accelZ * accelZ))));
  roll = atan2(-accelX ,( sqrt((accelY * accelY) + (accelZ * accelZ))));
  float Yh = (magY * cos(roll)) - (magZ * sin(roll));
  float Xh = (magX * cos(pitch))+(magY * sin(roll)*sin(pitch)) + (magZ * cos(roll) * sin(pitch));
  
  yaw =  atan2(Yh, Xh);
  
  
  roll = roll*57.3;
  pitch = pitch*57.3;
  yaw = yaw*57.3;
}

void setup() 
{
    pinMode(enterButton, INPUT);
    pinMode(rightButton, INPUT);
    pinMode(leftButton, INPUT);
    btStop(); //make sure the bluetooth is off
    Serial.begin(115200); //if we are displaying things to a computer then we need to setup serial
    SD.begin(); //sd card initializer
    SerialGPS.begin(9600, SERIAL_8N1, 16, 17); //gps initializer
    mpu.begin(); //mpu initializer
    oledDisplay.init();//display initializer
    oledDisplay.page(OLED_PAGE_STATS);

    //calibrate the mpu
    bool calibrated = true; //we are goign to skip the calibration for a while until we get to game testing and such
    if(calibrated == false)
    {
      Serial.println("Please dont move the device");
      delay(1000);
      mpu.calibrateAccel();
      mpu.calibrateGyro();
      Serial.println("Please continously rotate the device on all 3 axis");
      delay(1000);
      mpu.calibrateMag();
      Serial.println("Done, here is the new bias and scale factors");
    }

    //pin a loop to the other core since its not running wifi or bt. this will instead be running radio opperation and interpretation. 
    //in return we are free to use the other core to do more intensive work without having to wait for a message
    /*xTaskCreatePinnedToCore(
      radioIOCode,
      "RadioIO",
      1000,
      NULL,
      1,
      &RadioIO,
      0);*/
}

void buttons()
{
  String tempScreen;
  switch (screen)
  {
    case STARTUP: tempScreen = "STARTUP"; break;
    case SETTINGS: tempScreen = "SETTINGS"; break;
  }
  //record wether or not the buttons are being pushed and if they are take appropriate action
  leftButtonState = digitalRead(leftButton);
  rightButtonState = digitalRead(rightButton);
  enterButtonState = digitalRead(enterButton);
  if (leftButtonState == HIGH && leftButtonPushed == false) //left is pushed
  {
    leftButtonPushed = true;
    holdingLeft = millis();
  }
  else if (rightButtonState == HIGH && rightButtonPushed == false) //right is pushed
  {
    rightButtonPushed = true;
    holdingRight = millis();
  }
  else if (enterButtonState == HIGH && enterButtonPushed == false) //enter (fire) is pushed
  {
    enterButtonPushed = true;
    holdingEnter = millis();
  }

  int holdingLeftTime = millis() - holdingLeft;
  int holdingRightTime = millis() - holdingRight;
  int holdingEnterTime = millis() - holdingEnter;
  if (leftButtonState == LOW && leftButtonPushed == true && holdingLeftTime <= holdingTime) //left is released
  {
    leftButtonPushed = false;
  }
  else if (rightButtonState == LOW && rightButtonPushed == true && holdingRightTime <= holdingTime) //right is released
  {
    rightButtonPushed = false;
  }
  else if (enterButtonState == LOW && enterButtonPushed == true && tempScreen == "Game" && justFlipped == false) //fire is released
  {
    enterButtonPushed = false;
  }
  else if (enterButtonState == LOW && enterButtonPushed == true && justFlipped == true) //fire is released
  {
    enterButtonPushed = false;
    justFlipped = false;
  }
  else if (enterButtonState == LOW && enterButtonPushed == true && tempScreen == "Settings" && justFlipped == false) //fire is released
  {
    enterButtonPushed = false;
  }
  else if (leftButtonState == HIGH && leftButtonPushed == true && holdingLeftTime > holdingTime)//what to do if the next button is held for longer then holding time
  {
  }
  else if (enterButtonState == HIGH && enterButtonPushed == true && holdingEnterTime > holdingTime && tempScreen == "Game" && justFlipped == false)//what to do if the enter button is held for longer then holding time and in the game screen
  {
    justFlipped = true;
  }
  else if (enterButtonState == HIGH && enterButtonPushed == true && holdingEnterTime > holdingTime && tempScreen == "Settings" && justFlipped == false)//what to do if the enter button is held for longer then holding time and in the settings screen
  {
    justFlipped = true;
  }
  else if (rightButtonState == HIGH && rightButtonPushed == true && holdingRightTime > holdingTime)//what to do if the right button is held for longer then holding time
  {
  }

}

void loop() 
{
  //run all the various updates
  mpuUpdate();
  gpsUpdate();
  buttons();
  oledDisplay.loop();
  
  Serial.print( pitch );
  Serial.print(" ");
  Serial.print( roll);
  Serial.print(" ");
  Serial.println( yaw );

  if (loaded == true && enterButtonPushed == true)
  {
    screen = OLED_PAGE_STATS;
  }
}
