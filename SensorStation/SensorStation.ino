#include "rpcWiFi.h"
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include <HttpClient.h>

#include "SparkFun_SCD4x_Arduino_Library.h"
SCD4x mySensor;
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); 

const char* ssid = "iPhone (3)";
const char* password =  "Pchs1987!";

String serverName = "https://www.toptal.com/developers/postbin/b/1711221224661-7851452671457";

JsonDocument doc;
int co2;
float temp;
float humid;
int counter = 0;

char output[512];

String json;

void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(F(" lux"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(F(" lux"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution, 4); Serial.println(F(" lux"));  
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  delay(500);
}

void configureSensor(void)
{
  tsl.setGain(TSL2591_GAIN_MED);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);


  Serial.println(F("------------------------------------"));
  Serial.print  (F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
  Serial.print  (F("Timing:       "));
  Serial.print((tsl.getTiming() + 1) * 100, DEC); 
  Serial.println(F(" ms"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
}

void simpleRead(void)
{
  // Simple data read example. Just read the infrared, fullspecrtrum diode 
  // or 'visible' (difference between the two) channels.
  // This can take 100-600 milliseconds! Uncomment whichever of the following you want to read
  uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);
  //uint16_t x = tsl.getLuminosity(TSL2591_FULLSPECTRUM);
  //uint16_t x = tsl.getLuminosity(TSL2591_INFRARED);

  Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
  Serial.print(F("Luminosity: "));
  Serial.println(x, DEC);
}


void setup() {
    Serial.begin(115200);
    while(!Serial); // Wait for Serial to be ready

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial.println("Connecting to WiFi..");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
        WiFi.begin(ssid, password);
    }
    Serial.println("Connected to the WiFi network");
    Serial.print("IP Address: ");
    Serial.println (WiFi.localIP()); // prints out the device's IP address

    Wire.begin();
    if (mySensor.begin() == false)
    {
      Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
      while (1)
        ;
    }

    if (mySensor.stopPeriodicMeasurement() == true)
    {
      Serial.println(F("Periodic measurement is disabled!"));
    }  

    //Now we can enable low power periodic measurements
    if (mySensor.startLowPowerPeriodicMeasurement() == true)
    {
      Serial.println(F("Low power mode enabled!"));
    }

    if (tsl.begin()) 
    {
      Serial.println(F("Found a TSL2591 sensor"));
    } 
    else 
    {
      Serial.println(F("No sensor found ... check your wiring?"));
      while (1);
    }
    /* Display some basic information on this sensor */
    displaySensorDetails();
    configureSensor();

    }

void loop() {
  if (mySensor.readMeasurement()) // readMeasurement will return true when fresh data is available
  {

    counter += 1;
    uint16_t lux = tsl.getLuminosity(TSL2591_VISIBLE);
    int32_t lum = tsl.getFullLuminosity();
    uint16_t ir;
    ir = lum >> 16;

    co2 = mySensor.getCO2();
    temp = mySensor.getTemperature();
    humid = mySensor.getHumidity();
    Serial.println();

    Serial.print(F("CO2(ppm):"));
    Serial.print(co2);


    Serial.print(F("\tTemperature(C):"));
    Serial.print(temp, 1);

    Serial.print(F("\tHumidity(%RH):"));
    Serial.print(humid, 1);

    Serial.print(F("\tLuminosity: "));
    Serial.print(lux, DEC);

    Serial.print(F("\tIR: ")); 
    Serial.print(ir);  
    Serial.println(F("  "));

    doc["action"] = "addreadings";
    doc["type"] = "test";

    JsonArray readings = doc.createNestedArray("readings");

    JsonObject readings_0 = readings.createNestedObject();
    readings_0["userid"] = "-1";
    readings_0["name"] = "CO2";
    readings_0["value"] = co2;

    JsonObject readings_1 = readings.createNestedObject();
    readings_1["userid"] = "-1";
    readings_1["name"] = "Temperature";
    readings_1["value"] = temp;

    JsonObject readings_2 = readings.createNestedObject();
    readings_2["userid"] = "-1";
    readings_2["name"] = "Humidity";
    readings_2["value"] = humid;

    JsonObject readings_3 = readings.createNestedObject();
    readings_3["userid"] = "-1";
    readings_3["name"] = "Luminosity";
    readings_3["value"] = lux;

    JsonObject readings_4 = readings.createNestedObject();
    readings_4["userid"] = "-1";
    readings_4["name"] = "IR";
    readings_4["value"] = ir;

    Serial.println();
    serializeJson(doc, json);
    //Serial.print(json);
    //Serial.println();
    Serial.println(counter);
    

    if((WiFi.status()== WL_CONNECTED) && (counter == 10)){
      WiFiClientSecure client;
      HTTPClient http;
      //DynamicJsonDocument doc1(2048);
      //doc1["hello"] = "world";

      //String json1;
      //serializeJson(doc1, json1);

      http.begin(client, "https://us-central1-aiot-fit-xlab.cloudfunctions.net/healthbaseline");
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Accept", "*/*");
      http.POST(json);
      //int httpResponseCode = http.POST(json);
      Serial.println(http.getString());

      //Serial.print("HTTP Response code: ");
      //Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
      counter= 0;
    }
    

  }
  else
    Serial.print(F("."));

  delay(1000);

}