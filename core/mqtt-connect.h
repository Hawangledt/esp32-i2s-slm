#include "secrets.h"
#include <ArduinoJson.h>
#include <NTPClient.h> //https://github.com/taranais/NTPClient
#include <PubSubClient.h>
#include <TimeLib.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>


#define AWS_IOT_PUBLISH_TOPIC "esp32/pub"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 60 * 60);

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  timeClient.begin();
  Serial.println("Sync timeClient");
  while (!timeClient.update())
  {
    Serial.print(".");
    delay(100);
  }
  
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  Serial.println("AWS IoT Connected!");
}

void publishMessage(float wecSoundLevel)
{
  StaticJsonDocument<200> doc;
  doc["time"] = timeClient.getFormattedTime();
  doc["Weighted equivalent continous sound level"] = wecSoundLevel;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
