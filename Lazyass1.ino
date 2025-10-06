#include <Arduino.h>   // ESP32 Arduino core automatically includes FreeRTOS

//------------------ Global Variables ------------------
uint8_t G_DataID = 1;
int32_t G_DataValue = 0;

//------------------ Data Structure --------------------
typedef struct {
    uint8_t dataID;
    int32_t DataValue;
} Data_t;

//------------------ Handles ---------------------------
QueueHandle_t Queue1;
TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;

//------------------ Task 1: Sender --------------------
void ExampleTask1(void *pV) {
    Data_t dataToSend;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(500);

    while (1) {
        // Fill structure with global values
        dataToSend.dataID = G_DataID;
        dataToSend.DataValue = G_DataValue;

        // Send data to queue
        if (xQueueSend(Queue1, &dataToSend, portMAX_DELAY) == pdPASS) {
            Serial.printf("Task1: Sent data -> ID: %d, Value: %ld\n",
                          dataToSend.dataID, dataToSend.DataValue);
        }

        // Delay precisely for 500ms
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

//------------------ Task 2: Receiver ------------------
void ExampleTask2(void *pV) {
    Data_t receivedData;
    UBaseType_t originalPriority = uxTaskPriorityGet(NULL);
    BaseType_t priorityIncreased = pdFALSE;

    while (1) {
        // Wait indefinitely for data
        if (xQueueReceive(Queue1, &receivedData, portMAX_DELAY) == pdPASS) {
            Serial.printf("Task2: Received data -> ID: %d, Value: %ld\n",
                          receivedData.dataID, receivedData.DataValue);

            // Logic according to problem statement
            if (receivedData.dataID == 0) {
                Serial.println("Task2: Deleting itself (dataID == 0)");
                vTaskDelete(NULL);
            } 
            else if (receivedData.dataID == 1) {
                if (receivedData.DataValue == 0 && !priorityIncreased) {
                    vTaskPrioritySet(NULL, originalPriority + 2);
                    priorityIncreased = pdTRUE;
                    Serial.println("Task2: Priority increased by 2");
                } 
                else if (receivedData.DataValue == 1 && priorityIncreased) {
                    vTaskPrioritySet(NULL, originalPriority);
                    priorityIncreased = pdFALSE;
                    Serial.println("Task2: Priority restored");
                } 
                else if (receivedData.DataValue == 2) {
                    Serial.println("Task2: Deleting itself (DataValue == 2)");
                    vTaskDelete(NULL);
                }
            }
        }
    }
}

//------------------ Setup Function --------------------
void setup() {
    Serial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Give time for serial start
    Serial.println("FreeRTOS Queue and Task Example Start");

    // Create Queue
    Queue1 = xQueueCreate(5, sizeof(Data_t));
    if (Queue1 == NULL) {
        Serial.println("Failed to create Queue!");
        while (1);
    }

    // Create Tasks
    xTaskCreatePinnedToCore(ExampleTask1, "SenderTask", 2048, NULL, 2, &TaskHandle_1, 1);
    xTaskCreatePinnedToCore(ExampleTask2, "ReceiverTask", 2048, NULL, 3, &TaskHandle_2, 1);
}

void loop() {
    // Update global data values periodically for testing
    static uint32_t counter = 0;
    vTaskDelay(pdMS_TO_TICKS(2000));
    counter++;
    G_DataID = 1;
    G_DataValue = counter % 3; // cycles through 0, 1, 2
}
