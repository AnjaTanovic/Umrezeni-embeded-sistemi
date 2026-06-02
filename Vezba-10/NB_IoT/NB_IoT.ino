#include "Quectel_BG600L.h"

#define uS_TO_S_FACTOR 1000000  //faktor konverzije iz mikrosekundi u sekunde
#define TIME_TO_SLEEP  5 * 60        //period spavanja u sekundama

void setup() 
{
  DEBUG_STREAM.begin(115200);
  delay(2000);
  DEBUG_STREAM.println("--- NB-IoT low power test ---");

  BG600L_turnON();
//  BG600L_powerDown();
  BG600L_nwkSetup();
  char payload[] = "PoC2 udp echo test...";
  char response[256];
  bool tx_response;
  BG600L_TxUDPbin("45.76.87.164", 50123, (uint8_t *)payload, strlen(payload), &tx_response);
  response[BG600L_RxUDPbin((uint8_t *)response, tx_response)] = '\0'; 
  BG600L_enterPSM();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start(); 
}

void loop()
{
  if (DEBUG_STREAM.available())
  {
    delay(100);
    char str[128];
    int len = DEBUG_STREAM.available();
    DEBUG_STREAM.readBytes(str, len);
    str[len] = '\0';
    
    DEBUG_STREAM.print("CMD -> ");
    DEBUG_STREAM.print(str);
    
    NBIOT_STREAM.print(str);
  }
  if (NBIOT_STREAM.available())
    DEBUG_STREAM.write(NBIOT_STREAM.read());
}
