#define uS_TO_S_FACTOR 1000000  //faktor konverzije iz mikrosekundi u sekunde
#define TIME_TO_SLEEP  5        //period spavanja u sekundama

RTC_DATA_ATTR int bootCount = 0;

void setup(){
  Serial.begin(115200);
  delay(1000);

  //inkrementacija i prikaz brojaca ciklusa budjenja
  ++bootCount;
  Serial.println("Budjenje br: " + String(bootCount));

  //prikazi razlog budjenja
  print_wakeup_reason();

  //podesavanje tajmera
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("ESP32 podesen za spavanje na " + String(TIME_TO_SLEEP) +
  " sekundi");

  //spavaj
  esp_deep_sleep_start();
}

void loop(){}

//prikaz razloga budjenja
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0: 
	  Serial.println("Budjenje uzrokovano spoljnim signalom koji koristi RTC_IO"); 
	  break;
    case ESP_SLEEP_WAKEUP_EXT1: 
	  Serial.println("Budjenje uzrokovano spoljnim signalom koji koristi RTC_CNTL"); 
	  break;
    case ESP_SLEEP_WAKEUP_TIMER: 
	  Serial.println("Budjenje uzrokovano tajmerom"); 
	  break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: 
	  Serial.println("Budjenje uzrokovano touchpadom"); 
	  break;
    case ESP_SLEEP_WAKEUP_ULP: 
	  Serial.println("Budjenje uzrokovano ULP programom"); 
	  break;
    default: 
	  Serial.println("Budjenje nije uzrokovano dubokim snom"); 
	  break;
  }
}
