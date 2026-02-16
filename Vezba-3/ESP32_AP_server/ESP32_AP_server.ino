#include <WiFi.h>
#include <WebServer.h>
#include <Arduino_HTS221.h>

/* AP network */
const char* ssid = "ESP32";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

float temperature = 0;
float humidity = 0;

void setup()
{
  Serial.begin(115200);
  
  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while(1);
  }
  
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  server.on("/", handle_home);
  server.on("/sensor", handle_sensorPage);
  server.on("/read", handle_readSensor);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  server.handleClient();
}

void handle_home() {

  String html = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  html +="<title>ESP32 - Temp&Hum</title>";
  html += "<style>";
  html += "body{font-family:Arial; text-align:center; background:#f2f2f2;}";
  html += ".box{margin-top:80px;}";
  html += "button{font-size:22px; padding:15px 30px; border:none; border-radius:10px; background:#4CAF50; color:white;}";
  html += "button:hover{background:#45a049;}";
  html += "</style></head><body>";

  html += "<div class='box'>";
  html += "<h1>Hello from ESP32! :)</h1><br>";
  html += "<h3>Using Access Point (AP) Mode</h3><br>";
  html += "<a href=\"/sensor\"><button>Go to sensor page</button></a>";
  html += "</div>";

  html += "</body></html>";

  server.send(200,"text/html",html);
}

void handle_sensorPage() {

  String html = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  html +="<title>ESP32 - Temp&Hum</title>";
  html += "<style>";
  html += "body{font-family:Arial; text-align:center; background:#f2f2f2;}";
  html += ".box{margin-top:60px;}";
  html += ".value{font-size:22px; margin:10px;}";
  html += "button{font-size:20px; padding:12px 25px; margin:10px; border:none; border-radius:10px; background:#2196F3; color:white;}";
  html += "button:hover{background:#1976D2;}";
  html += "</style></head><body>";

  html += "<div class='box'>";
  html += "<h1>Temperature & Humidity</h1>";
  html += "<div class='value'>Temperature: " + String(temperature) + " &deg;C</div>";
  html += "<div class='value'>Humidity: " + String(humidity) + " %</div><br>";

  html += "<a href=\"/read\"><button>Read sensor</button></a><br>";
  html += "<a href=\"/\"><button style='background:#777'>Back</button></a>";
  html += "</div>";

  html += "</body></html>";

  server.send(200,"text/html",html);
}

void handle_readSensor() {

  temperature = HTS.readTemperature();
  humidity = HTS.readHumidity();
  
  Serial.println("Sensor read from WEB!");
  Serial.println(temperature);
  Serial.println(humidity);
  
  handle_sensorPage();
}
