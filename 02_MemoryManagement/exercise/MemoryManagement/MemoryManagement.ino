// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint8_t buf_len = 255;

// Globals
static char *msgPtr = NULL;
static volatile uint8_t msgAvailable = 0;

void readSerial(void* parameter) {
  String serialBuf;
  const char *msgBuf = NULL;
  while (1) {
    // Read cahracters from serial
    if (Serial.available() > 0) {
      serialBuf = Serial.readStringUntil('\n');
      if (msgAvailable == 0) {
        msgBuf = serialBuf.c_str();

        Serial.print("Free heap before allocating (bytes): ");
        Serial.println(xPortGetFreeHeapSize());
        msgPtr = (char *)pvPortMalloc((serialBuf.length()+1) * sizeof(char));
        Serial.print("Free heap after allocating (bytes): ");
        Serial.println(xPortGetFreeHeapSize());

        if (msgPtr != NULL) {
          memcpy(msgPtr, msgBuf, serialBuf.length()+1);
          msgAvailable = 1;
        }
      }
    }
  }
}

void writeSerial(void* parameter) {
  while (1) {
    if (msgAvailable != 0) {
      Serial.println(msgPtr);
      Serial.print("Free heap before de-allocating (bytes): ");
      Serial.println(xPortGetFreeHeapSize());
      vPortFree(msgPtr);
      Serial.print("Free heap after de-allocating (bytes): ");
      Serial.println(xPortGetFreeHeapSize());
      msgPtr = NULL;
      msgAvailable = 0;
      Serial.println("-------------------------------------------");
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Heap Demo---");
  Serial.println("Enter a string");

  // Start Serial receive task
  xTaskCreatePinnedToCore(readSerial,
                          "readSerial",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
                        
  xTaskCreatePinnedToCore(writeSerial,
                          "writeSerial",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
  
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
