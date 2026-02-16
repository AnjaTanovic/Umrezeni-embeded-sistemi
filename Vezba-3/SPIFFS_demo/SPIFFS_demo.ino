#include "SPIFFS.h"
 
void setup()
{
  Serial.begin(115200);
  
  if(!SPIFFS.begin(true))
  {
    Serial.println("Greska prilikom inicijalizacije SPIFFS");
    return;
  }
  
  File file = SPIFFS.open("/primer.txt");
  if(!file)
  {
    Serial.println("Greska prilikom otvaranja datoteke");
    return;
  }
  
  Serial.println("Sadrzaj datoteke:");
  while(file.available())
    Serial.write(file.read());
  file.close();
}
 
void loop() 
{
}
