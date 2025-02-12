// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

const char msg[] = "-----------------------------Testing preemption in FreeRTOS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

static TaskHandle_t taskHandle_1 = NULL;
static TaskHandle_t taskHandle_2 = NULL;

void PrintMessage(void *parameter) {
  int msg_len = strlen(msg);
  while (1) {
    for (int i=0; i<msg_len; i++) {
      Serial.print(msg[i]);
    }
    Serial.println();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void PrintStarts(void *parameter) {
  while (1) {
    Serial.print('*');
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  //set up serial
  Serial.begin(300);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("---- FreeRTOS Tasks Preemption Demo ----");
  
  Serial.print("Setup and loop task running on core");
  Serial.print(xPortGetCoreID());
  Serial.print(" with priority ");
  Serial.println(uxTaskPriorityGet(NULL));

  // put your setup code here, to run once:
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
            PrintMessage,      // Function to be called
            "PrintMessage",   // Name of task
            1024,           // Stack size (bytes in ESP32, words in FreeRTOS)
            NULL,           // Parameter to pass
            1,              // Task priority
            &taskHandle_1,           // Task handle
            app_cpu);       // Run on one core for demo purposes (ESP32 only)
          
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
            PrintStarts,      // Function to be called
            "PrintStart",   // Name of task
            1024,           // Stack size (bytes in ESP32, words in FreeRTOS)
            NULL,           // Parameter to pass
            2,              // Task priority
            &taskHandle_2,           // Task handle
            app_cpu);       // Run on one core for demo purposes (ESP32 only)
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i=0; i<10; i++) {
    Serial.println("Suspend");
    vTaskSuspend(taskHandle_2);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    Serial.println("Resume");
    vTaskResume(taskHandle_2);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
  if (taskHandle_1 != NULL) {
    vTaskDelete(taskHandle_1);
    taskHandle_1 = NULL;
  }
}
