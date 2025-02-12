// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint8_t buf_len = 255;     // Size of buffer to look for command
static const char command[] = "delay "; // Note the space!
static const int delay_queue_len = 5;   // Size of delay_queue
static const int msg_queue_len = 5;     // Size of msg_queue
static const uint8_t blink_max = 100;   // Num times to blink before message

// Pins (change this if your Arduino board does not have LED_BUILTIN defined)
static const int led_pin = 16;

// Globals
static QueueHandle_t delay_queue;
static QueueHandle_t msg_queue;

size_t getDelaytime(char* input_command) {
  uint8_t cmd_len = strlen(command);
  size_t delay_time = 0;
  if (memcmp(input_command, command, cmd_len) == 0) {
    char* tail = &input_command[cmd_len];
    delay_time = atoi(tail);
  }
  return delay_time;
}

void readSerialAndEcho(void* parameter) {
  char c = 0;
  uint8_t idx = 0;
  char msg_buf[buf_len];
  char msg_buf1[buf_len];
  size_t delay_time = 0;
  memset(msg_buf, 0, buf_len);
  memset(msg_buf1, 0, buf_len);

  while (1) {
    if (Serial.available() > 0) {
      c = Serial.read();
      if (c == '\r') {
        Serial.println(c);
        msg_buf[idx] = '\0';
        delay_time = getDelaytime(msg_buf);
        if (delay_time > 0) {
          if (xQueueSend(delay_queue, (void *)&delay_time, 10) != pdTRUE) {
            Serial.println("ERROR: Could not put item on delay queue.");
          }
        }
        idx = 0;
        memset(msg_buf, 0, buf_len);
      } else {
        Serial.print(c);
        msg_buf[idx] = c;
        idx++;
      }
    }

    if (xQueueReceive(msg_queue, (void *)&msg_buf1, 0) == pdTRUE) {
      // Best practice: use only one task to manage serial comms
      Serial.println(msg_buf1);
    }
  }
}

void blinkLed(void* parameter) {
  size_t led_delay = 500, counter = 0;
  char msg_buf[buf_len];
  String message = "Blinked ";
  String message1;

  while (1) {
    if (xQueueReceive(delay_queue, (void *)&led_delay, 0) == pdTRUE) {
      // Best practice: use only one task to manage serial comms
      memset(msg_buf, 0, buf_len);
      strcpy(msg_buf, "Received message");
      if (xQueueSend(msg_queue, (void *)&msg_buf, 10) != pdTRUE) {
        Serial.println("ERROR: Could not put item on delay queue.");
      }
    } 
    else {
      //Serial.println("Queue is empty!");
    }

    // Blink
    digitalWrite(led_pin, HIGH);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);

    counter++;
    if ((counter % blink_max) == 0) {
      memset(msg_buf, 0, buf_len);
      message1 = message + String(counter);
      strcpy(msg_buf, message1.c_str());
      xQueueSend(msg_queue, (void *)&msg_buf, 10);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // Set up pin
  pinMode(led_pin, OUTPUT);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();

  // Create queues
  delay_queue = xQueueCreate(delay_queue_len, sizeof(size_t));
  msg_queue = xQueueCreate(msg_queue_len, 255*sizeof(char));

  // Start Serial receive task
  xTaskCreatePinnedToCore(readSerialAndEcho,
                          "readSerial",
                          2048,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  xTaskCreatePinnedToCore(blinkLed,
                        "blinkLed",
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
