RTC_DATA_ATTR int bootCount = 0;

void setup(){
  Serial.begin(115200);
  delay(1000); 

  //inkrementacija i prikaz brojaca ciklusa budjenja
  ++bootCount;
  Serial.println("Budjenje br: " + String(bootCount));

  //prikazi razlog budjenja
  print_wakeup_reason();

  //konfigurisi GPIO0 kao ext0 okidac za budjenje na nizak logicki nivo
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0,LOW);

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
