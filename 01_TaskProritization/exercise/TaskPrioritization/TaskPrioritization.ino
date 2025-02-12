#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static volatile int globalRate = 1000;
static int ledPin = 16;

void LedBlink(void* parameter) {
  while (1) {
    digitalWrite(ledPin, HIGH);
    vTaskDelay(globalRate / portTICK_PERIOD_MS);
    digitalWrite(ledPin, LOW);
    vTaskDelay(globalRate / portTICK_PERIOD_MS);
  } 
}

void updateRate(void* parameter) {
  int rate = 0;
  while (1) {
    Serial.println("Input blinking rate that you want:");
    while (Serial.available() == 0) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    while (Serial.available() > 0) {
      rate = Serial.parseInt();
      if (rate > 0) {
        Serial.print("The blinking rate now will be changed to ");
        Serial.println(rate);
        globalRate = rate;
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT); 

  // put your setup code here, to run once:
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
            LedBlink,      // Function to be called
            "LedBlink",   // Name of task
            1024,           // Stack size (bytes in ESP32, words in FreeRTOS)
            NULL,           // Parameter to pass
            1,              // Task priority
            NULL,           // Task handle
            app_cpu);       // Run on one core for demo purposes (ESP32 only)

  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
            updateRate,      // Function to be called
            "updateRate",   // Name of task
            1024,           // Stack size (bytes in ESP32, words in FreeRTOS)
            NULL,           // Parameter to pass
            1,              // Task priority
            NULL,           // Task handle
            app_cpu);       // Run on one core for demo purposes (ESP32 only)
}

void loop() {
  // put your main code here, to run repeatedly:

}
