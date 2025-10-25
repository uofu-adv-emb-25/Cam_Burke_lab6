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

int status_number; // 3 for higest task
static volatile int log_arr[16];
static volatile int log_index;;

void setUp(void) {}

void tearDown(void) {}

void high_priority_thread(void *) {
    printf("High priority thread waiting for semaphore\n");
    xSemaphoreTake(share_semaphore, portMAX_DELAY);
    printf("High priority thread started\n");
    for(volatile int i = 0; i < 100000; i++){
        status_number = 3;
        if (log_arr[log_index] != status_number) {
            log_index++;
            log_arr[log_index] = status_number;
        }
    }  
    printf("High priority thread finished\n");
    xSemaphoreGive(share_semaphore);
    vTaskDelete(NULL);
}

void middle_priority_thread(void *) {
    printf("Middle priority thread started\n");
    for(volatile int i = 0; i < 200000; i++){
        status_number = 2;
        if (log_arr[log_index] != status_number) {
            log_index++;
            log_arr[log_index] = status_number;
        }
    }
    vTaskDelete(NULL);  
}

void low_priority_thread(void *) {
    printf("Low priority thread waiting for semaphore\n");

    xSemaphoreTake(share_semaphore, portMAX_DELAY);
    printf("Low priority thread started\n");
    for(volatile int i = 0; i < 2000000; i++){
        status_number = 1;

        if (log_index == 0){
           log_arr[log_index] = status_number;
        }
        
        else if (log_arr[log_index] != status_number) {
            log_index++;
            log_arr[log_index] = status_number;
        }
    }
    printf("Low priority thread finished\n");
    xSemaphoreGive(share_semaphore);
    vTaskDelete(NULL);
    
    
}

void higher_thread(void) {
    
    TaskHandle_t higher_T;  
    xTaskCreate(high_priority_thread, "higher_thread", TEST_TASK_STACK_SIZE,
                NULL, TASK_PRIORITY_HIGH, &higher_T);
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


void test_priority_inversion(void) {
    int expected_log[] = {1, 2, 1, 3};
    printf("Log index: %d\n", log_index);
    for (int i = 0; i <= log_index; i++) {
        printf("Log arr[%d]: %d, expected: %d\n", i, log_arr[i], expected_log[i]);
        TEST_ASSERT_EQUAL_INT(expected_log[i], log_arr[i]);
    }
    
    
}
 
void supervisor(void *){
    while (1) {
        share_semaphore = xSemaphoreCreateBinary();
        log_index = 0;
        status_number = 0;
        xSemaphoreGive(share_semaphore); // Initialize semaphore as available
        printf("Start tests\n");

        lower_thread();
        vTaskDelay(pdMS_TO_TICKS(10));
        higher_thread();
        vTaskDelay(pdMS_TO_TICKS(10));
        middle_thread();
        vTaskDelay(pdMS_TO_TICKS(10));
        
        vTaskDelay(pdMS_TO_TICKS(10000));
        UNITY_BEGIN();
        printf("All done!\n");
        RUN_TEST(test_priority_inversion);
        UNITY_END();

        vTaskDelay(pdMS_TO_TICKS(5000));
        
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
