#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>



// Semaphore libraries
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#define TASK_PRIORITY_HIGH      ( tskIDLE_PRIORITY + 3UL )
#define TASK_PRIORITY_MID       ( tskIDLE_PRIORITY + 2UL )
#define TASK_PRIORITY_LOW       ( tskIDLE_PRIORITY + 1UL )

#define MAIN_TASK_STACK_SIZE     1024
#define MAIN_TASK_PRIORITY       ( tskIDLE_PRIORITY + 4 )
#define TEST_TASK_STACK_SIZE     1024

SemaphoreHandle_t share_semaphore;

void setUp(void) {}

void tearDown(void) {}

void high_priority_thread(void *) {
    xSemaphoreTake(share_semaphore, portMAX_DELAY);
    for(volatile int i = 0; i < 1000; i++);
    xSemaphoreGive(share_semaphore);
    vTaskDelete(NULL);
}

void middle_priority_thread(void *) {
    for(volatile int i = 0; i < 1000; i++);
}

void low_priority_thread(void *) {
    xSemaphoreTake(share_semaphore, portMAX_DELAY);
    for(volatile int i = 0; i < 1000; i++);
    xSemaphoreGive(share_semaphore);
    vTaskDelete(NULL);
    
}

void higher_thread(void) {
    
    TaskHandle_t higher_T;  
    xTaskCreate(high_priority_thread, "higher_thread", TEST_TASK_STACK_SIZE,
                NULL, TASK_PRIORITY_HIGH, &higher_T);
    vTaskDelay(1000);
}

void middle_thread(void) {
    TaskHandle_t middle_T;  
    xTaskCreate(middle_priority_thread, "middle_thread", TEST_TASK_STACK_SIZE,
                NULL, TASK_PRIORITY_MID, &middle_T);

}

void lower_thread(void) {
    TaskHandle_t lower_T;  
    xTaskCreate(low_priority_thread, "lower_thread", TEST_TASK_STACK_SIZE,
                NULL, TASK_PRIORITY_LOW, &lower_T);
}
 
void supervisor(void *){
    while (1) {
        share_semaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(share_semaphore);
        printf("Start tests\n");

        lower_thread();
        lower_thread();
        vTaskDelay(pdMS_TO_TICKS(10));
        higher_thread();
        vTaskDelay(pdMS_TO_TICKS(10));
        middle_thread();


        UNITY_BEGIN();
        printf("All done!\n");
        sleep_ms(5000);
        UNITY_END();
    }
  
}
int main (void)
{
    stdio_init_all();
    sleep_ms(5000);

    xTaskCreate(supervisor, "Supervisor_thread",
                MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL);

    vTaskStartScheduler(); // This function should never return.
}
