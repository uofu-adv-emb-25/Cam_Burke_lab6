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
SemaphoreHandle_t mutex;

int status_number; // 3 for higest task, 2 for mid, 1 for lowest

// Inversion log
static volatile int log_arr[16];
static volatile int log_index;

// Mutex log
static volatile int mutex_log_arr[16];
static volatile int mutex_log_index;

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


// Mutex semaphore version of the above threads

void high_priority_thread_mutex(void *) {
    printf("Mutex High Waiting\n");
    xSemaphoreTake(mutex, portMAX_DELAY);
    printf("Mutex High Start\n");
    for(volatile int i = 0; i < 100000; i++){
        status_number = 3;
        if (mutex_log_arr[mutex_log_index] != status_number) {
            mutex_log_index++;
            mutex_log_arr[mutex_log_index] = status_number;
        }
    }  
    printf("Mutex High Finished\n");
    xSemaphoreGive(mutex);
    vTaskDelete(NULL);
}


void middle_priority_thread_mutex(void *) {
    printf("Mutex Middle Started\n");
    for(volatile int i = 0; i < 200000; i++){
        status_number = 2;
        if (mutex_log_arr[mutex_log_index] != status_number) {
            mutex_log_index++;
            mutex_log_arr[mutex_log_index] = status_number;
        }
    }
    vTaskDelete(NULL);  
}


void low_priority_thread_mutex(void *) {
    printf("Low Mutex Waiting\n");

    xSemaphoreTake(mutex, portMAX_DELAY);
    printf("Low Mutex Started\n");
    for(volatile int i = 0; i < 2000000; i++){
        status_number = 1;

        if (mutex_log_index == 0){
           mutex_log_arr[mutex_log_index] = status_number;
        }
        
        else if (mutex_log_arr[mutex_log_index] != status_number) {
            mutex_log_index++;
            mutex_log_arr[mutex_log_index] = status_number;
        }
    }
    printf("Low Mutex Finished\n");
    xSemaphoreGive(mutex);
    vTaskDelete(NULL);   
}


void higher_thread_mutex(void) {
    
    TaskHandle_t higher_T_mutex;  
    xTaskCreate(high_priority_thread_mutex, "higher_thread_mutex", TEST_TASK_STACK_SIZE,
                NULL, TASK_PRIORITY_HIGH, &higher_T_mutex);
}

void middle_thread_mutex(void) {
    TaskHandle_t middle_T_mutex;  
    xTaskCreate(middle_priority_thread_mutex, "middle_thread_mutex", TEST_TASK_STACK_SIZE,
                NULL, TASK_PRIORITY_MID, &middle_T_mutex);   
}

void lower_thread_mutex(void) {
    TaskHandle_t lower_T_mutex;  
    xTaskCreate(low_priority_thread_mutex, "lower_thread_mutex", TEST_TASK_STACK_SIZE,
                NULL, TASK_PRIORITY_LOW, &lower_T_mutex);
}


// Test cases
void test_priority_inversion(void) {
    int expected_log[] = {1, 2, 1, 3};
    for (int i = 0; i < 4; i++) {
        printf("Log arr[%d]: %d, expected: %d\n", i, log_arr[i], expected_log[i]);
        TEST_ASSERT_EQUAL_INT(expected_log[i], log_arr[i]);
    }
}

void test_priority_inversion_mutex(void) {
    int expected_log[] = {1, 3, 2};
    for (int i = 0; i < 3; i++) {
        printf("Mutex Log arr[%d]: %d, expected: %d\n", i, mutex_log_arr[i], expected_log[i]);
        TEST_ASSERT_EQUAL_INT(expected_log[i], mutex_log_arr[i]);
    }
}
 

void supervisor(void *){
    while (1) {

        // Test inversion using binary semaphore.
        share_semaphore = xSemaphoreCreateBinary();
        log_index = 0;
        status_number = 0;
        for(int i = 0; i < 16; i++) {
            log_arr[i] = 0;
        }
        xSemaphoreGive(share_semaphore); // Initialize semaphore as available
        

        lower_thread();
        vTaskDelay(pdMS_TO_TICKS(10));
        higher_thread();
        vTaskDelay(pdMS_TO_TICKS(10));
        middle_thread();
        
        vTaskDelay(pdMS_TO_TICKS(2000));
        

        // Test inversion using mutex semaphore.
        mutex = xSemaphoreCreateMutex(); 
        mutex_log_index = 0;
        status_number = 0;
        xSemaphoreGive(mutex); 
        for(int i = 0; i < 16; i++) {
            mutex_log_arr[i] = 0;
        }

        lower_thread_mutex();
        vTaskDelay(pdMS_TO_TICKS(10));
        higher_thread_mutex();
        vTaskDelay(pdMS_TO_TICKS(10));
        middle_thread_mutex();
        
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Run tests
        printf("Start tests\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
        UNITY_BEGIN();
        
        RUN_TEST(test_priority_inversion);
        RUN_TEST(test_priority_inversion_mutex);

        printf("All done!\n");
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
