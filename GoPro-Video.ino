#include <SoftwareSerial.h>
#include <GDBStub.h>

SoftwareSerial espSerial(12,14);

#include <GoProControl.h>
#include "Secrets.h"

#define VERSION "0.3 2021-07-02"

#define ESP8266_BUTTON 4

/*
  Control your GoPro with the Serial Monitor 
  edit the file Secrets.h with your camera netword name and password
  CAMERA could be: HERO3, HERO4, HERO5, HERO6, HERO7, FUSION, HERO8, MAX
*/

int iEnabled = 0;
int iButtonRecording = 0;
int iRemoteRecording = 0;

GoProControl gp(GOPRO_SSID, GOPRO_PASS, CAMERA);

void setup()
{
  int iRet;

//  espSerial.begin(115200);
  espSerial.begin(1200);

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  gdbstub_init();
  delay(5000);

  pinMode(ESP8266_BUTTON, INPUT_PULLUP);
  
  Serial.println("GoProVideoControl");
  Serial.print("Ver ");
  Serial.println(VERSION);
  Serial.println("=================");

  gp.enableDebug(&Serial, 9600);
}

void loop()
{
  char in = 0;
  int iRet;
  
  if (Serial.available() > 0)
  {
    in = Serial.read();
  }
  else
  if (espSerial.available() > 0)
  {
    in = espSerial.read();    
#if 1    
{
Serial.println("**********");
Serial.println(in);
Serial.println("**********");
}
#endif
  }

  switch (in)
  {
  default:
    break;

  // Connect
  case 'b':
    iRemoteRecording = 0;
//    iRet = gp.begin();
    while ((iRet = gp.begin()) != 1)
    {
      Serial.print("gp.begin() = ");
      Serial.println(iRet);
      delay(2500);
    }
    break;

  case 'c':
    Serial.print("Connected: ");
    Serial.println(gp.isConnected() == true ? "Yes" : "No");
    break;

  case 'p':
    gp.confirmPairing();
    break;

  case 's':
    Serial.println("Status:");
    gp.getStatus();
    break;

  case 'm':
    Serial.println("Media List:");
    gp.getMediaList();
    break;

  // Turn on and off
  case 'T':
    gp.turnOn();
    break;

  case 't':
    gp.turnOff();
    break;

  // Take a picture or start a video
  case 'A':
    gp.shoot();
    break;

  // Stop the video
  case 'S':
    gp.stopShoot();
    break;

  // Check if it is recording
  case 'r':
    Serial.print("Recording: ");
    Serial.println(gp.isRecording() == true ? "Yes" : "No");
    break;

  // Set modes
  case 'V':
    gp.setMode(VIDEO_MODE);
    break;

  case 'P':
    gp.setMode(PHOTO_MODE);
    break;

  case 'M':
    gp.setMode(MULTISHOT_MODE);
    break;

  // Change the orientation
  case 'U':
    gp.setOrientation(ORIENTATION_UP);
    break;

  case 'D':
    gp.setOrientation(ORIENTATION_DOWN);
    break;

  // Change other parameters
  case 'f':
    gp.setVideoFov(MEDIUM_FOV);
    break;

  case 'F':
    gp.setFrameRate(FR_120);
    break;

  case 'R':
    gp.setVideoResolution(VR_1080p);
    break;

  case 'h':
    gp.setPhotoResolution(PR_12MP_WIDE);
    break;

  case 'L':
    gp.setTimeLapseInterval(60);
    break;

  // Localize the camera
  case 'O':
    gp.localizationOn();
    break;

  case 'o':
    gp.localizationOff();
    break;

  // Delete some files, be carefull!
  case 'l':
    gp.deleteLast();
    break;

  case 'g':
    gp.deleteAll();
    break;

  // Print useful data
  case 'd':
    gp.printStatus();
    break;

  // Close the connection
  case 'X':
    gp.end();
    break;

  // Open the connection
  case '1':
    Serial.print("iRemoteRecording=");
    Serial.println(iRemoteRecording);
    if (iRemoteRecording == 0)
    {
      while ((iRet = gp.begin()) != 1)
      {
        Serial.print("gp.begin() = ");
        Serial.println(iRet);
        delay(1000);
      }
   
//      iRet = gp.begin();
//      Serial.print("gp.begin() = ");
//      Serial.println(iRet);
      
      if (gp.isConnected())
      {
        Serial.println("CONNECTED");
        gp.setMode(VIDEO_MODE);
        gp.shoot();
        iRemoteRecording = 1;
        espSerial.print('1');
      }
      else
      {
        Serial.print("CONNECTION FAILED!");
      }
    }
    break;

  // Close the connection
  case '0':
    if (iRemoteRecording == 1 && gp.isConnected())
    {
      iRemoteRecording = 0;
      gp.stopShoot();
      gp.end();
      Serial.println("gp.end()");
    }
    break;
  }
  gp.keepAlive(); // not needed on HERO3

  if (digitalRead(ESP8266_BUTTON) == 0)
  {
    if (iButtonRecording == 0)
    {
      iRet = gp.begin();
      Serial.print("gp.begin() = ");
      Serial.println(iRet);
      if (gp.isConnected())
      {
        gp.shoot();
        iButtonRecording = 1;
      }
      else
      {
        Serial.print("CONNECTION FAILED!");
      }
    }
  }
  else
  {
    if (iButtonRecording == 1 && gp.isConnected())
    {
      iButtonRecording = 0;
      gp.stopShoot();
      gp.end();
      Serial.println("gp.end()");
    }
  }
}
