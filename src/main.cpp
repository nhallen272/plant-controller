#include "state_processor.h"
#include "state_control.h"
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

TimerHandle_t feedTimer; 

// TB View
WiFiClient espClient;
ThingsBoard TB(espClient);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE); 
OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);
// DS18B20 addresses
DeviceAddress TempSensor1 = {0x28, 0x80, 0x72, 0x56, 0xB5, 0x1, 0x3C, 0x9B };
DeviceAddress TempSensor2 = {0x28, 0x6, 0xBA, 0x7, 0xD6, 0x1, 0x3C, 0xC9 };
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// FSM
StateProcessor sp(SET_HUM);
StateController sc;

// TelemetryModel storage for air, soilTemps, humidity, etc. 
std::unique_ptr<TelemetryModel> telem(new TelemetryModel{0.0, 0.0, 0.0, 0.0}); // airTemp, soilTemp1, soilTemp2, humidity


float humidity = 0.0;
float airTemp = 0.0;
float soilTemp1 = 0.0;  
float soilTemp2 = 0.0;

volatile uint8_t currHr, currMin;
long int interval_passed = 0;
long int ntp_passed = 0;

bool connectWifi() {
wl_status_t status = WiFi.status();
switch (status)
{
  case WL_CONNECTED :
  {
    Serial.print("[WiFi] connected to ");
    Serial.println(MYSSID);
    return true;
  }
  case WL_CONNECT_FAILED    :
  case  WL_CONNECTION_LOST  :
  case  WL_DISCONNECTED     :
  default : 
  {
    // Attempt connection
    WiFi.begin(MYSSID, MYPASS);
    delay(10);
    // Connect to WiFi network
    Serial.println("[WiFi]");
    Serial.print("Connecting to ");
    Serial.println(MYSSID);

    while ((WiFi.status() != WL_CONNECTED)) {
      delay(500);
      Serial.print(".");
      Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
    }
  }
}
return false;
}

// Callback for the seedpump and topfeed timer
// Post: pumps both aeroponic sprayer and topfeed system for 1 min
void handlePump()
{
  Serial.println("Seedpump: ON");
  Serial.println("Topfeed: ON");
  digitalWrite(int(OutputType::Seedpump), LOW);
  digitalWrite(int(OutputType::Pump), LOW);
  delay(MIN_TO_MS(FEED_ON));
  digitalWrite(int(OutputType::Seedpump), HIGH);
  digitalWrite(int(OutputType::Pump), HIGH); 
  Serial.println("Seedpump: OFF");
  Serial.println("Topfeed: OFF");
  xTimerStart(feedTimer, 0);
}


// Pre: temps and humidity has been read. TB is connected.
// Post: Display temps, humidity, time to Serial and screen. Send telemetry to TB
void post() {
  // Print to serial
  Serial.print("Air Temp: ");
  Serial.print(telem->airTemp);
  Serial.println("F");
  Serial.print("Humidity: ");
  Serial.print(telem->humidity);
  Serial.println("%");
  Serial.print("Soil Temp1: ");
  Serial.print(telem->soilTemp1);
  Serial.println("F");
  Serial.print("Soil Temp2: ");
  Serial.print(telem->soilTemp2);
  Serial.println("F");
  Serial.print("Time: ");
  Serial.println(timeClient.getFormattedTime());

  // Print States:
  Serial.print("Light state: ");
  switch(sc.getState(OutputType::Lights))
  {
    case TriState::ON : 
    {
      Serial.println("ON");
      break;
    }
    default : 
    {
      Serial.println("OFF");
      break;
    }
  }
    Serial.print("Topfeed state: ");
  switch(sc.getState(OutputType::Pump))
  {
    case TriState::ON : 
    {
      Serial.println("ON");
      break;
    }
    default : 
    {
      Serial.println("OFF");
      break;
    }
  }
    Serial.print("Dehumidifier state: ");
  switch(sc.getState(OutputType::Dehumidifier))
  {
    case TriState::ON : 
    {
      Serial.println("ON");
      break;
    }
    default : 
    {
      Serial.println("OFF");
      break;
    }
  }
    Serial.print("Seedpump state: ");
  switch(sc.getState(OutputType::Seedpump))
  {
     case TriState::ON : 
    {
      Serial.println("ON");
      break;
    }
    default : 
    {
      Serial.println("OFF");
      break;
    }
  }


  // SSD1306  
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Air Temp: ");
  display.print(telem->airTemp);
  display.println("F");
  display.print("Humidity: ");
  display.print(telem->humidity);
  display.println("%");
  display.print("Soil Temp1: ");
  display.print(telem->soilTemp1);
  display.println("F");
  display.print("Soil Temp2: ");
  display.print(telem->soilTemp2);
  display.println("F");
  display.print("Time: ");
  display.print(timeClient.getFormattedTime());
  display.display();
  delay(2000);
}

// Pre: 10 sec interval passed, connected to WiFi and TB.
// Post: Sent telemetry to TB
bool send() {
  Serial.println("Sending telemetry");
  try
  {
    TB.sendTelemetryFloat("airTemp", telem->airTemp);
    TB.sendTelemetryFloat("humidity", telem->humidity);
    TB.sendTelemetryFloat("soilTemp1", telem->soilTemp1);
    TB.sendTelemetryFloat("soilTemp2", telem->soilTemp2);

    // Send States (lights, pump, humidifer, seedPump)
    TB.sendTelemetryBool("lights", bool(sc.getState(OutputType::Lights)));
    TB.sendTelemetryBool("topfeed", static_cast<bool>(sc.getState(OutputType::Pump)));
    TB.sendTelemetryBool("dehumidifier", static_cast<bool>(sc.getState(OutputType::Dehumidifier)));
    TB.sendTelemetryBool("seedpump", static_cast<bool>(sc.getState(OutputType::Seedpump)));

  }
  catch(...)
  {
    Serial.println("Error sending telemtry to TB");
    return false;
  }
  return true;
}


//Post: Humidity and air temp is read from the DHT11
void readDHT(float& h, float& t) {
  h = dht.readHumidity();
  t = dht.readTemperature(true);
    // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor."));
    return;
  }
}

//Post: Temps from the DS18B20's are read
void readSoilTemp(float& t1, float& t2) {
  sensors.requestTemperatures();
  t1 = sensors.getTempF(TempSensor1);
  t2 = sensors.getTempF(TempSensor2);


  if (isnan(t1) || isnan(t2)) {
    Serial.println(F("Failed to read soil temp from DS18B20 sensor."));
    return;
  }

}

// Post: Init screen, connect wifi, init DHT11, init DS18B20(soil temp sensor)
void setup() {
  Serial.begin(115200);

  // Setup ssd1306
  Wire.begin(5, 4);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed."));
    for(;;); 
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);
  display.setCursor(0,0);

  // Setup Wifi 
  WiFi.begin(MYSSID, MYPASS);
  connectWifi();
  delay(200);
  // Setup NTP client
  Serial.println("Initializing NTP Client...");
  timeClient.begin();
  timeClient.setTimeOffset(-14400); // EST = GMT-4:00
  delay(5000);
  timeClient.update(); 
  delay(200);
  currHr = timeClient.getHours();
  currMin = timeClient.getMinutes();
  // Init the State pre-processor
  sp.init(currHr, currMin);

  // Init DHT11 temp/humidity sensor
  Serial.println("Initializing DHT11 temp/humidity sensor...");
  dht.begin();

  // Init DallasTemperature DS18B20's
  sensors.begin();  //dallastemp

  /// Setup relay pins
  Serial.println("Setting relay outputs...");
  Serial.print("Lights pin: ");
  Serial.println(int(OutputType::Lights));
  pinMode(int(OutputType::Lights), OUTPUT);        // Normally closed (HIGH == ON)  PIN 0
  pinMode(int(OutputType::Pump), OUTPUT);          // NO (LOW == ON)                PIN 2
  pinMode(int(OutputType::Seedpump), OUTPUT);      // NO                            PIN 12
  pinMode(int(OutputType::Dehumidifier), OUTPUT);  // NO                            PIN 14
  // send initial OFF signals
  digitalWrite(int(OutputType::Lights), HIGH);      
  delay(250);
  digitalWrite(int(OutputType::Pump), HIGH);      
  delay(250);
  digitalWrite(int(OutputType::Seedpump), HIGH);   
  delay(250);
  digitalWrite(int(OutputType::Dehumidifier), HIGH);  
  delay(4000);

  feedTimer = xTimerCreate("feedTimer", pdMS_TO_TICKS(MIN_TO_MS(FEED_OFF)), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(handlePump));
  
  xTimerStart(feedTimer, 0);

}

void loop() {
  delay(delay_quant);
  interval_passed += delay_quant; 
  ntp_passed += delay_quant;

  // On 10 sec interval:
  if (interval_passed >= interval) 
  {
    interval_passed = 0;
    // Read humidity and air temp
    readDHT(telem->humidity, telem->airTemp);
    sc.updateDehumidifier(sp.humidistat(telem->humidity));
    // Read soil temps
    readSoilTemp(telem->soilTemp1, telem->soilTemp2);

    // check lighting
    sc.updateLights(sp.handleLights());


    if (connectWifi())
    { 
      
      // On 5 min interval: update NTP time.
      if (ntp_passed >= ntp_interval) {
        ntp_passed = 0;    
        timeClient.update(); // OR timeView.update();
        currHr = timeClient.getHours();
        currMin = timeClient.getMinutes();
        sp.updateTime(currHr, currMin);

      }
      // Send telemetry/handle TB connection 
      if (TB.connected())
      {
        send();
      }
      else 
      {
        // Connect to the ThingsBoard
        Serial.print("Connecting to: ");
        Serial.print(THINGSBOARD_SERVER);
        Serial.print(" with token ");
        Serial.println(TOKEN);
        if (!TB.connect(THINGSBOARD_SERVER, TOKEN)) 
        {
          Serial.println("Failed to connect");
          return;
        }  
      }
    }
    // Display to Serial and SSD1306
    post();
    sc.loop();
  }

}


