#include <M5StickC.h>
#include <WiFi.h>
#include "time.h"
#include "DHT12.h"
#include <Wire.h> //The DHT12 uses I2C comunication.
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>

DHT12 dht12; //Preset scale CELSIUS and ID 0x5c.
Adafruit_BMP280 bme;

const char* ssid = "Your SSID";        //Add your own wifi connection info and time server.
const char* password = "Your Password";
const char* ntpServer = "ADD TIME SERVER";

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

int battery = 0;
float b = 0;
int state = 1;


void setup() {
  // put your setup code here, to run once:

  M5.begin(true,true,true);
  M5.Lcd.setRotation(3);
  M5.Axp.ScreenBreath(8);  //screen brightness 7-15
  M5.Lcd.fillScreen(BLACK);
  setTime();
}

void loop() {
  // put your main code here, to run repeatedly:

  int modeButton = digitalRead(M5_BUTTON_HOME);
  int sleepButton = digitalRead(M5_BUTTON_RST);  //prefered using a button for sleep instead of a timer
  
  switch(state) {
    case 1:
      
      if(modeButton == HIGH && sleepButton == HIGH) {
        
            loadDisplay();

            batteryPercent();

            delay(1000);
        

        } else if(modeButton == LOW && sleepButton == HIGH) {
          M5.Lcd.fillScreen(BLACK);    //Added visual indicator to add time between the button press and state change.
          M5.Lcd.setCursor(2, 25);
          M5.Lcd.setTextSize(1);
          M5.Lcd.printf("Changing Modes");
          Serial.begin(115200);
          // Serial2.begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert)
          Serial2.begin(115200, SERIAL_8N1, 26, 0); //Sets the pin configuration for mode 2 serial pass through
          delay(1000);
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(2, 2);
          state = 2;
        } else if(modeButton == HIGH && sleepButton == LOW) {
          M5.Lcd.fillScreen(BLACK);
          M5.Axp.SetSleep();
        }
      break;

      case 3:
        if(modeButton == HIGH && sleepButton == HIGH) {
  
          if (!bme.begin(0x76)){  
            Serial.println("Could not find a valid BMP280 sensor, check wiring!");
            state = 1;
          }
  
          float tmp = dht12.readTemperature();
          float hum = dht12.readHumidity();
          float pressure = bme.readPressure();
          Serial.printf("Temperatura: %2.2f*C  Humedad: %0.2f%%  Pressure: %0.2fPa\r\n", tmp, hum, pressure);

          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0, 15);
          M5.Lcd.setTextColor(WHITE, BLACK);
          M5.Lcd.setTextSize(1);
          M5.Lcd.printf("Temp: %2.1f  \r\nHumi: %2.0f%%  \r\nPressure:%2.0fPa\r\n", tmp, hum, pressure);
  
          delay(1000);
          
        } else if(modeButton == LOW && sleepButton == HIGH) {
          M5.Lcd.fillScreen(BLACK);   //Added visual indicator to add time between the button press and state change.
          M5.Lcd.setCursor(2, 25);
          M5.Lcd.setTextSize(1);
          M5.Lcd.printf("Changing Modes");
          delay(1000);
          state = 1;
        } else if(modeButton == HIGH && sleepButton == LOW) {
          M5.Lcd.fillScreen(BLACK);
          M5.Axp.SetSleep();
        }
      break;

      case 2:

        // This is a serial passthrough Mode for the RS485 Hat
        if(modeButton == HIGH && sleepButton == HIGH) {

          M5.Lcd.print(".");  // a visual indicator that things are still moving along........

          if(Serial.available())  // If stuff was typed in the serial monitor
          {
              // Send any characters the Serial monitor prints to the rs485 device
              Serial2.print((char)Serial.read());
          }
          
          if(Serial2.available()) {    // If data was sent from the rs485 device show it
              char ch = Serial2.read();
              M5.Lcd.setCursor(2, 2);
              M5.Lcd.fillScreen(BLACK);
              M5.Lcd.print(ch);
          }
            delay(500);
            

            }else if(modeButton == LOW && sleepButton == HIGH) {
              M5.Lcd.fillScreen(BLACK);   //Added visual indicator to add time between the button press and state change.
              M5.Lcd.setCursor(2, 25);
              M5.Lcd.setTextSize(1);
              M5.Lcd.printf("Changing Modes");
              Wire.begin(0,26);  //Wire.begin(sda, scl); Sets the pin configuration for mode 3 and the ENV hat
              delay(1000);
              state = 3;
            }else if(modeButton == HIGH && sleepButton == LOW) {
              M5.Lcd.fillScreen(BLACK);
              M5.Axp.SetSleep();
            }
      break;
   }
}

void batteryPercent() {
  M5.Lcd.setCursor(115, 2);
  M5.Lcd.setTextSize(1);
  b = M5.Axp.GetVbatData() * 1.1 / 1000;
  battery = ((b - 3.0) / 1.2) * 100;

  if (battery > 100)
    battery = 100;
  else if (battery < 100 && battery > 9)
    M5.Lcd.print(" ");
  else if (battery < 9)
    M5.Lcd.print("  ");
  if (battery < 10)
    M5.Axp.DeepSleep();
  M5.Lcd.print(battery);
  M5.Lcd.print("%");

}

void loadDisplay() {

  static const char *wd[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
  
  M5.Rtc.GetTime(&RTC_TimeStruct);
  M5.Rtc.GetData(&RTC_DateStruct);
  if (RTC_TimeStruct.Hours > 12) {  //change to 12h format
      RTC_TimeStruct.Hours -= 12;
    }
  M5.Lcd.fillScreen(BLACK);  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(2, 2);
  M5.Lcd.printf("%s   %04d-%02d-%02d", wd[RTC_DateStruct.WeekDay], RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date);
  M5.Lcd.drawLine(0, 15, 159, 15, TFT_WHITE);
  M5.Lcd.setCursor(2, 25);
  M5.Lcd.setTextSize(3);
  M5.Lcd.printf("%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes);
  M5.Lcd.setCursor(100, 32);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("%02d", RTC_TimeStruct.Seconds);
}

void setTime() {

    // connect to wifi
  Serial.printf("Attempting to connect to %s ", ssid);
  WiFi.begin(ssid, password);
  delay(1000);
  while (WiFi.status() == WL_CONNECTED) {  //if wifi does connect go ahead and set the time with NTP
    Serial.println(" CONNECTED, SETTING TIME");
    
        // Set NTP time to local
    configTime(-5 * 3600, 3600, ntpServer);  // -5 is for my timezone. (utc - 5)
  
    // Get local time
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
  
      // Set RTC time
      RTC_TimeTypeDef TimeStruct;
      TimeStruct.Hours   = timeInfo.tm_hour;
      TimeStruct.Minutes = timeInfo.tm_min;
      TimeStruct.Seconds = timeInfo.tm_sec;
      M5.Rtc.SetTime(&TimeStruct);
      
      RTC_DateTypeDef DateStruct;
      DateStruct.WeekDay = timeInfo.tm_wday;
      DateStruct.Month = timeInfo.tm_mon + 1;
      DateStruct.Date = timeInfo.tm_mday;
      DateStruct.Year = timeInfo.tm_year + 1900;
      M5.Rtc.SetData(&DateStruct);
    }
      // Disconnect Wifi if it did connect
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("Wifi disconnected");  
  }
}
