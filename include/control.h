// control.h
// Definitions for the plant controller

#ifndef Control_h
#define Control_h

#include <Arduino.h>
#include <ThingsBoard.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DHT.h> //DHT11 temp and humidity
#include <Wire.h>
#include <Adafruit_SSD1306.h> 
#include <NTPClient.h> // For NTP time
#include <WiFiUdp.h> 
#include <DallasTemperature.h>

// mins to ms macro
#define MIN_TO_MS(min) (min * 60000)



// Finite states for: Lights, pumps, 
enum class TriState  {
    ON,         // Turn on state
    OFF,        // Turn off state
    IDLE        // No change
};

// Outputs managed by the 4-channel relay.
enum class OutputType {
    Lights = 0,
    Pump = 2, 
    Seedpump = 25,   // Aeroponic pump for germination.
    Dehumidifier = 26
};

// Hold airTemp, soiltemps, humidity
struct TelemetryModel
{ 
    float airTemp; 
    float soilTemp1; 
    float soilTemp2; 
    float humidity;

};

const int delay_quant = 20; //ms
const long interval = 5000;      // 10 sec delay between loops
const long ntp_interval = 30000; // 5 min interval to request NTP packet 

// Wifi
#define MYSSID "AllenFamily"
#define MYPASS "beach1418mountains"
// TB
#define TOKEN               "C0UVgiVHe0OBS0jwKbAL"
#define THINGSBOARD_SERVER  "161.35.108.236"
// OLED
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET     4
#define SCREEN_ADDRESS 0x3C
// Pins
#define DHTPIN 15
#define DHTTYPE DHT11
#define ONE_WIRE_BUS 13  // for OneWire DS18B20's

// Schedule defs
#define SET_HUM 60.0  //  (Range: 50 - 60 % )
// in hrs
#define LIGHT_ON 6
#define LIGHT_OFF 24
// in mins
#define FEED_ON 1
#define FEED_OFF 60


#endif



