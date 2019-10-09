
/**************************************************************************************
 
 ESP8266 NodeMCU WiFi Internet clock and weather station and with SSD1306 OLED display
 This is a free software with NO WARRANTY.
 http://simple-circuit.com/
 
***************************************************************************************/
 
// ESP8266 WiFi main library
#include <ESP8266WiFi.h>
 
// Libraries for internet time
#include <WiFiUdp.h>
#include <NTPClient.h>          // include NTPClient library
#include <TimeLib.h>            // include Arduino time library
 
// Libraries for internet weather
#include <ESP8266HTTPClient.h>  // http web access library
#include <ArduinoJson.h>        // JSON decoding library
 
// Libraries for SSD1306 OLED display
#include <SPI.h>
#include <Wire.h>              // include wire library (for I2C devices such as the SSD1306 display)
#include <Adafruit_GFX.h>      // include Adafruit graphics library
#include <Adafruit_SSD1306.h>  // include Adafruit SSD1306 OLED display driver

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
 
#define OLED_RESET   -1     // define SSD1306 OLED reset at ESP8266 GPIO5 (NodeMCU D1)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
// set Wi-Fi SSID and password
const char *ssid     = "Sayantan";
const char *password = "sayantan";
 
WiFiUDP ntpUDP;
// 'time.nist.gov' is used (default server) with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
NTPClient timeClient(ntpUDP, "time.nist.gov", 3600, 60000);
 
// set location and API key
String Location = "Basirhat, IN";
String API_Key  = "602ed00549e101036851ecf421164d33";
 
void setup(void)
{
  Serial.begin(9600);
  delay(1000);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
 
  Wire.setClock(400000L);   // set I2C clock to 400kHz
 
  display.clearDisplay();   // clear the display buffer
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);
  display.display();
 
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
 
  timeClient.begin();
 
  delay(1000);
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
 
      display.setCursor(0, 0);
      display.print("Time:");
      display.setCursor(60, 0);
      display.print(Time);        // display time (format: hh:mm:ss)
      display.setCursor(0, 11);
      display_wday();
      display.print(Date);        // display date (format: dd-mm-yyyy)
      display.display();
 
      last_second = second_;
    }
 
    if (last_minute != minute_)       // update weather every 1 minute
    {
      HTTPClient http;  // declare an object of class HTTPClient
 
      // specify request destination
      http.begin("http://api.openweathermap.org/data/2.5/weather?q=" + Location + "&APPID=" + API_Key);  // !!
 
      int httpCode = http.GET();  // send the request
 
      if (httpCode > 0)  // check the returning code
      {
        String payload = http.getString();   // get the request response payload
 
        DynamicJsonBuffer jsonBuffer(512);
 
        // Parse JSON object
        JsonObject& root = jsonBuffer.parseObject(payload);
        if (!root.success())
        {
          Serial.println(F("Parsing failed!"));
          return;
        }
 
        float temp = (float)(root["main"]["temp"]) - 273.15;        // get temperature in °C
        int   humidity = root["main"]["humidity"];                  // get humidity in %
        float pressure = (float)(root["main"]["pressure"]) / 1000;  // get pressure in bar
        float wind_speed = root["wind"]["speed"];                   // get wind speed in m/s
        int  wind_degree = root["wind"]["deg"];                     // get wind degree in °
 
        display.setCursor(0, 24);
        display.printf("Temperature: %5.2f C\r\n", temp);
        display.printf("Humidity   : %d %%\r\n", humidity);
        display.printf("Pressure   : %.3fbar\r\n", pressure);
        display.printf("Wind speed : %.1f m/s\r\n", wind_speed);
        display.printf("Wind degree: %d", wind_degree);
        display.drawRect(109, 24, 3, 3, WHITE);     // put degree symbol ( ° )
        display.drawRect(97, 56, 3, 3, WHITE);
        display.display();
 
      }
 
      http.end();   // close connection
 
      last_minute = minute_;
    }
 
    delay(200);
 
  }
}
 
void display_wday()
{
  switch(wday)
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
