// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const char command[] = "avg"; 
static const size_t cmd_len = 3;
static const uint16_t timer_divider = 8;
static const uint64_t timer_max_count = 1000000;
static const int adc_pin = 36;
static const int buffer_len = 10;

static hw_timer_t *timer = NULL;
static SemaphoreHandle_t overrunSemaphore = NULL;
static SemaphoreHandle_t samplingCompleteSemaphore = NULL;
static SemaphoreHandle_t averageMutex = NULL;
static int head = 0;
static int tail = 0;
static float average = 0;
static int myBuffer[buffer_len];

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
  if (xSemaphoreTakeFromISR(overrunSemaphore, &xHigherPriorityTaskWoken) == pdTRUE) {
    //Serial.println("Take ok!!!");
    myBuffer[head] = analogRead(adc_pin);
    head = (head + 1) % buffer_len;
  } 
  else {
    Serial.println("Overrun!!!");
  }

  if (head == buffer_len-1) {
    xSemaphoreGiveFromISR(samplingCompleteSemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
}

void calculateAvg(void *parameters) {
  while (1) {
    int cnt = 0;
    float temp;
    if (xSemaphoreTake(samplingCompleteSemaphore, portMAX_DELAY) == pdTRUE) {
      temp = 0;
      while (cnt < buffer_len) {
        temp = temp + myBuffer[tail];
        tail = (tail + 1) % buffer_len;
        cnt += 1;
        xSemaphoreGive(overrunSemaphore);
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      xSemaphoreTake(averageMutex, 0);
      average = temp / buffer_len;
      xSemaphoreGive(averageMutex);
    }
  }
}

void readSerialAndEcho(void* parameter) {
  char c = 0;
  size_t idx = 0;
  char msgBuff[255];
  memset(msgBuff, 0, 255);

  while (1) {
    if (Serial.available() > 0) {
      c = Serial.read();
      if (c == '\r') 
      {
        Serial.println(c);
        if (memcmp(msgBuff, command, cmd_len) == 0) {
          Serial.print("Average: ");
          xSemaphoreTake(averageMutex, 0);
          Serial.println(average);
          xSemaphoreGive(averageMutex);
        }
        idx = 0;
        memset(msgBuff, 0, 255);
      }
      else 
      {
        Serial.print(c);
        msgBuff[idx++] = c;
      }
    }
  }
}

void setup() {
  // put your setup code here, to run once:
    // Configure Serial
  Serial.begin(9600);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS ISR Critical Section Demo---");

  // Create and start timer (frequency)
  timer = timerBegin(80000000/timer_divider);
  
  if (timer != NULL) Serial.println("Timer created successfully");

  // Provide ISR to timer (timer, function)
  timerAttachInterrupt(timer, &onTimer);

  // Allow ISR to trigger
  timerAlarm(timer, timer_max_count, true, 0);

  overrunSemaphore = xSemaphoreCreateCounting(buffer_len, buffer_len);
  samplingCompleteSemaphore = xSemaphoreCreateBinary();
  averageMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(calculateAvg,
                      "calculateAvg",
                      1024,
                      NULL,
                      2,
                      NULL,
                      app_cpu);

  xTaskCreatePinnedToCore(readSerialAndEcho,
                          "readSerialAndEcho",
                          2048,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
