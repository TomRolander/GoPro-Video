#include "time.h"
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <GDBStub.h>

SoftwareSerial espSerial(12,14);

#include <GoProControl.h>
#include "Secrets.h"

#define VERSION "0.4 2021-11-13"

#define ESP8266_BUTTON 4

#define GOPRO_CONNECT_TIMEOUT 30000

/*
  Control your GoPro with the Serial Monitor 
  edit the file Secrets.h with your camera netword name and password
  CAMERA could be: HERO3, HERO4, HERO5, HERO6, HERO7, FUSION, HERO8, MAX
*/

int iEnabled = 0;
int iButtonRecording = 0;
long lStartTimeMS = 0;

GoProControl gp(GOPRO_SSID, GOPRO_PASS, CAMERA);

const char* ssid     = "Stanford";                    // your network SSID (name)
const char* password = "";                    // your network password

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -(8*3600);
int   daylightOffset_sec = 0;  // 3600;

/*
  %a Abbreviated weekday name 
  %A Full weekday name 
  %b Abbreviated month name 
  %B Full month name 
  %c Date and time representation for your locale 
  %d Day of month as a decimal number (01-31) 
  %H Hour in 24-hour format (00-23) 
  %I Hour in 12-hour format (01-12) 
  %j Day of year as decimal number (001-366) 
  %m Month as decimal number (01-12) 
  %M Minute as decimal number (00-59) 
  %p Current locale's A.M./P.M. indicator for 12-hour clock 
  %S Second as decimal number (00-59) 
  %U Week of year as decimal number,  Sunday as first day of week (00-51) 
  %w Weekday as decimal number (0-6; Sunday is 0) 
  %W Week of year as decimal number, Monday as first day of week (00-51) 
  %x Date representation for current locale 
  %X Time representation for current locale 
  %y Year without century, as decimal number (00-99) 
  %Y Year with century, as decimal number 
  %z %Z Time-zone name or abbreviation, (no characters if time zone is unknown) 
  %% Percent sign 
  You can include text literals (such as spaces and colons) to make a neater display or for padding between adjoining columns. 
  You can suppress the display of leading zeroes  by using the "#" character  (%#d, %#H, %#I, %#j, %#m, %#M, %#S, %#U, %#w, %#W, %#y, %#Y) 
*/
char timeBuffer[80];

void printLocalTime()
{
  time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (timeBuffer,80,"%Y-%m-%d %H:%M:%S ",timeinfo);
//  strftime (timeBuffer,80," %d %B %Y %H:%M:%S ",timeinfo);
  //struct tm timeinfo;
  //time_t now = time(nullptr);
  Serial.println(timeBuffer);
  //Serial.print(ctime(&now));
  //Serial.print(&timeinfo, " %d %B %Y %H:%M:%S ");
}


void setup()
{
  int iRet;

//  espSerial.begin(115200);
  espSerial.begin(1200);

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  delay(1000);
#if 0
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#endif
  gdbstub_init();

  delay(5000);

  pinMode(ESP8266_BUTTON, INPUT_PULLUP);
  
  Serial.println("GoProVideoControl");
  Serial.print("Ver ");
  Serial.println(VERSION);
  Serial.println("=================");

  gp.enableDebug(&Serial, 9600);

#if 0  
  // We start by connecting to a WiFi network
  Serial.print("\n\nConnecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
    would try to act as both a client and an access-point and could cause
  network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  Serial.println("\nWaiting for time");
  unsigned timeout = 5000;
  unsigned start = millis();
  while (!time(nullptr)) 
  {
    Serial.print(".");
    delay(1000);
  }
  delay(1000);
  
  Serial.println("Time...");
  printLocalTime();
#endif
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
    if (gp.isConnected())
    {
      Serial.println("Already connected!");
      break;
    }

    lStartTimeMS = millis();
    while ((iRet = gp.begin()) != 1)
    {
      Serial.print("gp.begin() = ");
      Serial.println(iRet);
      delay(2500);
      if (millis() > (lStartTimeMS + GOPRO_CONNECT_TIMEOUT))
      {
        Serial.println("Connection failed, TIMEOUT");
        break;
      }
    }
    if (gp.isConnected() == false)
    {
      Serial.println("Probably connection lost!");
    }
    break;

  case 'c':
    Serial.print("Connected: ");
    if (gp.isConnected())
    {
      Serial.println("Yes");
      espSerial.print('1');
    }
    else
    {
      Serial.println("No");
      espSerial.print('0');
    }
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
    if (gp.isConnected())
    {       
      gp.shoot();
      espSerial.print('1');
    }
    else
    {
      Serial.println("A failed: Not Connected");
      espSerial.print('0');      
    }
    break;

  // Stop the video
  case 'S':
    if (gp.isConnected())
    {       
      gp.stopShoot();
      espSerial.print('1');
    }
    else
    {
      Serial.println("S failed: Not Connected");
      espSerial.print('0');      
    }
    break;

  // Check if it is recording
  case 'r':
    Serial.print("Recording: ");
    Serial.println(gp.isRecording() == true ? "Yes" : "No");
    break;

  // Set modes
  case 'V':
    if (gp.isConnected())
    {       
      gp.setMode(VIDEO_MODE);
      espSerial.print('1');
    }
    else
    {
      Serial.println("V failed: Not Connected");
      espSerial.print('0');      
    }
    break;

  case 'P':
    if (gp.isConnected())
    {       
      gp.setMode(PHOTO_MODE);
      espSerial.print('1');
    }
    else
    {
      Serial.println("P failed: Not Connected");
      espSerial.print('0');      
    }
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

  // Turn OFF daylight savings
  case '4':
    daylightOffset_sec = 0;
    Serial.println("Turn OFF daylight savings");
    break;
    
  // Turn ON daylight savings
  case '3':
    daylightOffset_sec = 3600;
    Serial.println("Turn ON daylight savings");
    break;
    
  // Get current time
  case '2':
    GetLocalTime();
    espSerial.print(timeBuffer);
    break;
    
  // Open the connection
  case '1':
    lStartTimeMS = millis();
    while ((iRet = gp.begin()) != 1)
    {
      Serial.print("gp.begin() = ");
      Serial.println(iRet);
      delay(2500);
      if (millis() > (lStartTimeMS + GOPRO_CONNECT_TIMEOUT))
      {
        Serial.println("Connection failed, TIMEOUT");
        break;
      }
    }
    
    if (gp.isConnected())
    {
      Serial.println("CONNECTED");
      //gp.setMode(VIDEO_MODE);
      espSerial.print('1');
    }
    else
    {
      Serial.print("CONNECTION FAILED!");
      espSerial.print('0');
    }
    break;

  // Close the connection
  case '0':
    if (gp.isConnected())
    {       
      gp.end();
      espSerial.print('1');
      Serial.println("gp.end()");
    }
    else
    {
      espSerial.print('0');      
    }
    break;
  }
  
  if (gp.isConnected())
    gp.keepAlive(); // not needed on HERO3

#if 0
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
#endif  
}

void GetLocalTime()
{
  // We start by connecting to a WiFi network
  Serial.print("\n\nConnecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
    would try to act as both a client and an access-point and could cause
  network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  Serial.println("\nWaiting for time");
  unsigned timeout = 5000;
  unsigned start = millis();
  while (!time(nullptr)) 
  {
    Serial.print(".");
    delay(1000);
  }
  delay(1000);
  
  Serial.println("Time...");
  printLocalTime();  
}
