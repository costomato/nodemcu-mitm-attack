#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

Adafruit_BMP280 bmp;

const char *ssid =  "<wifi_ssid>";
const char *pass =  "<wifi_password>";

long channelID = 1234567;
const char* writeAPIKey = "<api_key>";

WiFiClient client;

void setup()
{
  Serial.begin(9600);
  
  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  ThingSpeak.begin(client);
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  
  float t = bmp.readTemperature();
  float p = bmp.readPressure();
  float a = bmp.readAltitude(1013.25);

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" deg. C");

  Serial.print("Pressure: ");
  Serial.print(p);
  Serial.println(" Pa");

  Serial.print("Altitude: ");
  Serial.print(a);
  Serial.println(" m");

  Serial.println("Writing data to ThingSpeak...");

  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, p);
  ThingSpeak.setField(3, a);
  
  int writeStatus = ThingSpeak.writeFields(channelID, writeAPIKey);

  if (writeStatus == 200)
    Serial.println("Channel updated successfully. Writing again in 15 seconds...");
  else
    Serial.println("Problem updating channel. HTTP error code = " + String(writeStatus) + ". Trying again in 15 seconds...");

  delay(15000);
}
