// ESP8266 WiFi main library
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

//Libraries for INSTAGRAM Stat
#include "InstagramStats.h"       // Thanks to Brian Lough for creating this awesome Library. https://github.com/witnessmenow/arduino-instagram-stats
#include "JsonStreamingParser.h"

// Libraries for internet time
#include <WiFiUdp.h>
#include <NTPClient.h>          // include NTPClient library
#include <TimeLib.h>            // include Arduino time library

// Libraries for SSD1306 OLED display
#include <SPI.h>
#include <Wire.h>              // include wire library (for I2C devices such as the SSD1306 display)
#include <Adafruit_GFX.h>      // include Adafruit graphics library
#include <Adafruit_SSD1306.h>  // include Adafruit SSD1306 OLED display driver

WiFiClientSecure client;
InstagramStats instaStats(client);

unsigned long delayBetweenChecks = 60000; //mean time between api requests
unsigned long whenDueToCheck = 0;

//UID
String userName = "electropoint4u"; // follow this instagram url https://www.instagram.com/electropoint4u/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET   -1     // define SSD1306 OLED reset at ESP8266 GPIO5 (NodeMCU D1)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// set Wi-Fi SSID and password
const char *ssid     = "SSID";
const char *password = "PASSWORD";

WiFiUDP ntpUDP;
// 'time.nist.gov' is used (default server) with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
NTPClient timeClient(ntpUDP, "time.nist.gov", 3600, 60000);

void setup()
{
  Serial.begin(115200);
  delay(10);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  Wire.setClock(400000L);   // set I2C clock to 400kHz

  display.clearDisplay();   // clear the display buffer
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting.");
  display.setCursor(0, 24);
  display.println("Connecting...");
  display.display();

  while ( WiFi.status() != WL_CONNECTED )
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("connected");
  display.print("connected");
  display.display();

  client.setInsecure();

  timeClient.begin();

  delay(10);
  display.clearDisplay();
}

char Time[] = "  :  :  ";
char Date[] = "  -  -20  ";
byte last_second, last_minute, second_, minute_, hour_, wday, day_, month_, year_;

void loop()
{
  if (WiFi.status() == WL_CONNECTED)  // check WiFi connection status
  {
    timeClient.update();
    unsigned long unix_epoch = timeClient.getEpochTime();   // get UNIX Epoch time

    second_ = second(unix_epoch);        // get seconds from the UNIX Epoch time
    if (last_second != second_)          // update time & date every 1 second
    {
      minute_ = minute(unix_epoch);      // get minutes (0 - 59)
      hour_   = hour(unix_epoch);        // get hours   (0 - 23)
      wday    = weekday(unix_epoch);     // get minutes (1 - 7 with Sunday is day 1)
      day_    = day(unix_epoch);         // get month day (1 - 31, depends on month)
      month_  = month(unix_epoch);       // get month (1 - 12 with Jan is month 1)
      year_   = year(unix_epoch) - 2000; // get year with 4 digits - 2000 results 2 digits year (ex: 2018 --> 18)

      Time[7] = second_ % 10 + '0';
      Time[6] = second_ / 10 + '0';
      Time[4] = minute_ % 10 + '0';
      Time[3] = minute_ / 10 + '0';
      Time[1] = hour_   % 10 + '0';
      Time[0] = hour_   / 10 + '0';
      Date[9] = year_   % 10 + '0';
      Date[8] = year_   / 10 + '0';
      Date[4] = month_  % 10 + '0';
      Date[3] = month_  / 10 + '0';
      Date[1] = day_    % 10 + '0';
      Date[0] = day_    / 10 + '0';

      display.setCursor(12, 0);
      display.setTextSize(2);
      display.print(Time);        // display time (format: hh:mm:ss)
      display.display();

      last_second = second_;
    }
    delay(200);

    unsigned long timeNow = millis();
    if ((timeNow > whenDueToCheck))  {
      getInstagramStatsForUser();
      whenDueToCheck = timeNow + delayBetweenChecks;
    }
  }
}

void getInstagramStatsForUser() {
  Serial.println("Getting Instagram user stats for " + userName);
  display.setTextSize(1);
  display.setCursor(20, 27);
  display.print(userName);
  display.display();
  InstagramUserStats response = instaStats.getUserStats(userName);
  Serial.println("Response:");
  Serial.print("Number of followers: ");
  Serial.println(response.followedByCount);
  display.setTextSize(2);
  display.setCursor(40, 44);
  display.print(response.followedByCount);
  display.display();
}

void display_wday()
{
  switch (wday)
  {
    case 1:  display.print("SUNDAY    "); break;
    case 2:  display.print("MONDAY    "); break;
    case 3:  display.print("TUESDAY   "); break;
    case 4:  display.print("WEDNESDAY "); break;
    case 5:  display.print("THURSDAY  "); break;
    case 6:  display.print("FRIDAY    "); break;
    default: display.print("SATURDAY  ");
  }

}

// End of code.
