#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>



// Semaphore libraries
#include <FreeRTOS.h>
#include <semphr.h>
#include "task.h"
#include <pico/time.h>


#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 4) // Highest priority
#define TASK_PRIORITY_HIGH      ( tskIDLE_PRIORITY + 3)
#define TASK_PRIORITY_MID       ( tskIDLE_PRIORITY + 2)
#define TASK_PRIORITY_LOW       ( tskIDLE_PRIORITY + 1) // Lowest priority

#define MAIN_TASK_STACK_SIZE     1024
#define TEST_TASK_STACK_SIZE     1024


SemaphoreHandle_t share_semaphore;
SemaphoreHandle_t mutex;

int status_number; // 3 for higest task, 2 for mid, 1 for lowest

// Inversion log
static volatile int log_arr[16];
static volatile int log_index;
static volatile int last_value;

// Mutex log
static volatile int mutex_log_arr[16];
static volatile int mutex_log_index;
static volatile int last_mutex_value;

void setUp(void) {}

void tearDown(void) {}


static inline void log_push(int v) {            // Tells the compiler to put code where its called.
    taskENTER_CRITICAL();                       // Disable interrupts
        if (last_value != v) {
            log_arr[log_index++] = v;
            last_value = v;                     // Update last logged value
        }
    taskEXIT_CRITICAL();
}

static inline void mutex_log_push(int v) {      // Tells the compiler to put code where its called.
    taskENTER_CRITICAL();                       // Disable interrupts
        if (last_mutex_value != v) {
            mutex_log_arr[mutex_log_index++] = v;
            last_mutex_value = v;               // Update last logged value
        }
    taskEXIT_CRITICAL();
}

void high_priority_thread(void *) {
    printf("High priority thread waiting for semaphore\n");
    xSemaphoreTake(share_semaphore, portMAX_DELAY);
    printf("High priority thread started\n");
    for(volatile int i = 0; i < 100000; i++){
        log_push(3);
    }  
    printf("High priority thread finished\n");
    xSemaphoreGive(share_semaphore);
    vTaskDelete(NULL);
}


void middle_priority_thread(void *) {
    printf("Middle priority thread started\n");
    for(volatile int i = 0; i < 200000; i++){
        log_push(2);
    }
    vTaskDelete(NULL);  
}


void low_priority_thread(void *) {
    printf("Low priority thread waiting for semaphore\n");

    xSemaphoreTake(share_semaphore, portMAX_DELAY);
    printf("Low priority thread started\n");
    for(volatile int i = 0; i < 2000000; i++){
        log_push(1);
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
        mutex_log_push(3);
    }  
    printf("Mutex High Finished\n");
    xSemaphoreGive(mutex);
    vTaskDelete(NULL);
}


void middle_priority_thread_mutex(void *) {
    printf("Mutex Middle Started\n");
    for(volatile int i = 0; i < 200000; i++){
        mutex_log_push(2);
    }
    vTaskDelete(NULL);  
}


void low_priority_thread_mutex(void *) {
    printf("Low Mutex Waiting\n");

    xSemaphoreTake(mutex, portMAX_DELAY);
    printf("Low Mutex Started\n");
    for(volatile int i = 0; i < 2000000; i++){
        mutex_log_push(1);
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

// Activity 2: Compteting Threads
void busy_busy(void *)
{
    for (int i = 0; ; i++);
}

void busy_yield(void *)
{
    for (int i = 0; ; i++) {
        taskYIELD();
    }
}

void competing_busy(TaskFunction_t t1_func, TaskFunction_t t2_func, uint64_t *t1_exec_time, uint64_t *t2_exec_time, uint64_t t1_t2_delay_ms, BaseType_t t1_priority, BaseType_t t2_priority) {
    TaskHandle_t t1;
    TaskHandle_t t2;
    
    TickType_t start_ticks = xTaskGetTickCount();
    uint64_t start_count = portGET_RUN_TIME_COUNTER_VALUE();

    xTaskCreate(t1_func, "first_thread", TEST_TASK_STACK_SIZE,
            NULL, t1_priority, &t1);

            
    vTaskDelay(pdMS_TO_TICKS(t1_t2_delay_ms)); // Delay before starting second task


    xTaskCreate(t2_func, "second_thread", TEST_TASK_STACK_SIZE,
                NULL, t2_priority, &t2);

    vTaskDelay(pdMS_TO_TICKS(1000)); 
    
    TaskStatus_t t1_status, t2_status;

    TaskStatus_t s1, s2;
    vTaskGetInfo(t1, &s1, pdTRUE, eInvalid);   // pdTRUE -> include ulRunTimeCounter
    vTaskGetInfo(t2, &s2, pdTRUE, eInvalid);

    *t1_exec_time = s1.ulRunTimeCounter;       // units = whatever portGET_RUN_TIME_COUNTER_VALUE returns (µs here)
    *t2_exec_time = s2.ulRunTimeCounter;
    
    printf("Task 1 Runtime: %llu\n", *t1_exec_time);
    printf("Task 2 Runtime: %llu\n", *t2_exec_time);

    vTaskDelete(t1);
    vTaskDelete(t2);
}

void both_busy_busy(void) {
    uint64_t time_1 = 0;
    uint64_t time_2 = 0;
    
    competing_busy(busy_busy, busy_busy, &time_1, &time_2, 0, TASK_PRIORITY_LOW, TASK_PRIORITY_LOW);
    TEST_ASSERT(time_1 > 400000 && time_1 < 600000);
    TEST_ASSERT(time_2 > 400000 && time_2 < 600000);
}

void both_busy_yield(void) {
    uint64_t time_1;
    uint64_t time_2;
    
    competing_busy(busy_yield, busy_yield, &time_1, &time_2, 0, TASK_PRIORITY_LOW, TASK_PRIORITY_LOW);
    TEST_ASSERT(time_1 > 400000 && time_1 < 600000);
    TEST_ASSERT(time_2 > 400000 && time_2 < 600000);
}

void t1_busy_t2_yield(void) {
    uint64_t time_1;
    uint64_t time_2;
    
    competing_busy(busy_busy, busy_yield, &time_1, &time_2, 0, TASK_PRIORITY_LOW, TASK_PRIORITY_LOW);
    TEST_ASSERT(time_1 > 900000);
    TEST_ASSERT(time_2 < 100000);
}


void busy_busy_different_priority_high_first(void) {
    uint64_t time_1;
    uint64_t time_2;
    
    competing_busy(busy_busy, busy_busy, &time_1, &time_2, 100, TASK_PRIORITY_HIGH, TASK_PRIORITY_LOW);
    TEST_ASSERT(time_1 > 900000);
    TEST_ASSERT(time_2 == 0);
}


void busy_busy_different_priority_low_first(void) {
    uint64_t time_1;
    uint64_t time_2;
    
    competing_busy(busy_busy, busy_busy, &time_1, &time_2, 0, TASK_PRIORITY_LOW, TASK_PRIORITY_HIGH);
    TEST_ASSERT(time_1 < 100000);
    TEST_ASSERT(time_2 > 900000);
}

void busy_yield_different_priority(void) {
    uint64_t time_1;
    uint64_t time_2;
    
    competing_busy(busy_yield, busy_yield, &time_1, &time_2, 0, TASK_PRIORITY_LOW, TASK_PRIORITY_HIGH);
    TEST_ASSERT(time_1 < 100000);
    TEST_ASSERT(time_2 > 900000);
}


void supervisor(void *){
    while (1) {

        // Test inversion using binary semaphore.
        share_semaphore = xSemaphoreCreateBinary();
        log_index = 0;
        status_number = 0;
        xSemaphoreGive(share_semaphore); // Initialize semaphore as available
        for(int i = 0; i < 16; i++) {
            log_arr[i] = 0;
        }
        
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

        RUN_TEST(both_busy_busy);
        RUN_TEST(both_busy_yield);
        RUN_TEST(t1_busy_t2_yield);

        RUN_TEST(busy_busy_different_priority_high_first);
        RUN_TEST(busy_busy_different_priority_low_first);
        RUN_TEST(busy_yield_different_priority);

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
