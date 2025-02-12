// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const int led_pin = 16;
static TimerHandle_t xTimer = NULL;

void readSerialAndEcho(void* parameter) {
  char c = 0;

  while (1) {
    if (Serial.available() > 0) {
      c = Serial.read();
      if (c == '\r') Serial.println(c);
      else Serial.print(c);
      digitalWrite(led_pin, HIGH);
      if (xTimerReset(xTimer, 0) == pdFALSE) {
        Serial.println("Error on Timer Reset!!!!");
      }
    }
  }
}

void turnOffLed(TimerHandle_t xTimer) {
  digitalWrite(led_pin, LOW);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // Set up pin
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();

  xTimer = xTimerCreate("My Timer",
                        2000 / portTICK_PERIOD_MS,
                        pdFALSE,
                        (void*)0,
                        turnOffLed);

  // Start Serial receive task
  xTaskCreatePinnedToCore(readSerialAndEcho,
                          "readSerial",
                          2048,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
