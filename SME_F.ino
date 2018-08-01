 /*
    Low-Power Wireless Inclinometer
  
  By: Piotr Kurzynoga ID: 14143097
  
  This code focuses on the low power aspect and delivery of the coordinates to the SigFox framework.
 */

#include <Arduino.h>
#include <Wire.h>
#include <ArduinoLowPower.h>
#include <SmeSFX.h>
#include <LSM9DS1.h>

//Counter variables.
unsigned long timepassed=0;
long last_time=0;
int interval=55; //desired time -1 to make up for the > sign.


// the setup function runs once when you press reset or power the board
void setup() {
    Wire.begin();
    sfxAntenna.begin(115200); //initialize the SigFox module
    smeAccelerometer.begin(); //initialize the accelerometer
    SerialUSB.begin(115200); //initilize USB debugging console
    sfxWakeup(); //in case the module is in sleep mode
    delay(10);
    setPowerSaveMode(); //put the module in data mode
    delay(1000*60); //Allow for reprogramming of the board
}

//function puts the SigFox chip into PowerSave mode
static void setPowerSaveMode(void){
    int initFinish = 1;

    SerialUSB.println("SFX in Command mode");
    sfxAntenna.setSfxConfigurationMode(); // enter in configuration Mode

    do {
        uint8_t answerReady = sfxAntenna.hasSfxAnswer();
        if (answerReady) {
            switch (initFinish) {
            case 1:
                // set the PowerSave mode
                sfxAntenna.setSfxSleepMode(SFX_HW_WAKE);
                sfxAntenna.setSfxDataMode();
                initFinish++;
                break;
            }
        }
    } while (initFinish != 2);
}

void RP_calculate(int x,int y, int z, double roll, double pitch){
  sfxWakeup();
  delay(10);
  char messageBuffer[12];
  memset(messageBuffer, 0, sizeof(messageBuffer));
  double x_Buff = float(x);
  double y_Buff = float(y);
  double z_Buff = float(z);
  roll = atan2(y_Buff , z_Buff) * 57.3; // 57.3 = 180/PI (conversion to degrees)
  pitch = atan2((- x_Buff) , sqrt(y_Buff * y_Buff + z_Buff * z_Buff)) * 57.3;
  int rollint=roll*10; //multiplying by 10 and converting to an integer
  int pitchint=pitch*10; //multiplying by 10 and converting to an integer
  char message[11]; //only 11 bytes need to be prepared for the payload
  sprintf(message, "%d %d", rollint, pitchint);
  SerialUSB.print("Roll:  ");
  SerialUSB.print(roll);
  SerialUSB.print("Pitch: ");
  SerialUSB.println(pitch);
  SerialUSB.println(message);
  sfxAntenna.sfxSendData(message, strlen((char*)message));
}

// the loop function runs over and over again forever
void loop() {

timepassed=(millis()/1000);
  if(timepassed - last_time >= interval) // greater or equal in case sync issues occur.
    {
    smeAccelerometer.activate(); //Activating the sensor from Power Saving mode.
    delay(70);
    //ledGreenLight(LOW);
    //ledBlueLight(HIGH);
    int x = 0;
    int y = 0;
    int z = 0;
    double roll=0.00,pitch=0.00;
    x = smeAccelerometer.readX(); //Reading the x-axis acceleration
    y = smeAccelerometer.readY(); //Reading the y-axis acceleration
    z = smeAccelerometer.readZ(); //Reading the z-axis acceleration
    last_time=timepassed;
    smeAccelerometer.deactivate(); //Accelerometer going into Power Saving mode.
    RP_calculate(x, y, z, roll, pitch);
    } 

    //Checking if there is contact with the SigFox base station.
    bool answerReady = sfxAntenna.hasSfxAnswer();

    if (answerReady) {
        if (sfxAntenna.getSfxMode() == sfxDataMode) {

            switch (sfxAntenna.sfxDataAcknoledge()) {
            case SFX_DATA_ACK_START:
                SerialUSB.println("Waiting Answer");
                break;

            case SFX_DATA_ACK_PROCESSING:
                SerialUSB.print('.');
                break;

            case SFX_DATA_ACK_OK:
      #ifndef ASME3_REVISION
                ledBlueLight(LOW);
                ledGreenLight(HIGH);
      #endif
                //SerialUSB.println(' ');
                //SerialUSB.println("Answer OK :) :) :) :)");
                sfxSleep(); //Put The SigFoxmodule to sleep
                LowPower.sleep(1*60*1000); //1000ms*60=1minute*60=1 hour
                break;

            case SFX_DATA_ACK_KO:
      #ifndef ASME3_REVISION
                //ledRedLight(HIGH);
      #endif
                //SerialUSB.println(' ');
                //SerialUSB.println("Answer KO :( :( :( :(");
                break;
            }
        }
    }
}
 
