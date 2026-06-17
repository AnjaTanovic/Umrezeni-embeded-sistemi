#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_HTS221.h>
#include "SPIFFS.h"

const char *ssid = "...";
const char *password = "...";

AsyncWebServer server(80);

float temperature = 0;
float humidity = 0;

void setup()
{
  Serial.begin(115200);
  
  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while(1);
  }

  if(!SPIFFS.begin(true))
  {
    Serial.println("Failed to initialize SPIFFS");
    return;
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.css", "text/css");
  });
  
  server.on("/sensor", HTTP_GET, [](AsyncWebServerRequest *request){  
    request->send(SPIFFS, "/sensor.html", String(), false, processor);
  });

  server.on("/sensor.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/sensor.css", "text/css");
  });
  
  server.on("/read", HTTP_GET, [](AsyncWebServerRequest *request){  
    temperature = HTS.readTemperature();
    humidity = HTS.readHumidity();
  
    request->send(SPIFFS, "/sensor.html", String(), false, processor);
  });
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
}

String processor(const String& var)
{
  if(var == "TEMP") 
    return String(temperature);
  if(var == "HUM") 
    return String(humidity);
    
  return String();
}
