
/* PWM AC FAN Motor Controller

*/


#include <string>

// Thread загововки
// #include <iostream>
// #include <sstream>
#include <chrono>
#include <thread>
// #include <memory>
// #include "esp_pthread.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "consolexec.h"

#include "esp_log.h"




static const char* TAG = __FILE_NAME__;       // Локальный тег логирования модуля


using namespace std;
using namespace std::chrono;


 #define NUM_TIMERS 1
 TimerHandle_t xTimers[NUM_TIMERS];
 
 const auto sleep_time = seconds {
    10
};

 void vMyTimer_callback(TimerHandle_t xTimer)
 {
    // ESP_LOGD(TAG, "Numer of iteration %d", 4);
 }

 CPwmControl PwmCtrl1(PWM_OUTPUT_IO_DEFAULT);


// void print_thread_info(const char *extra = nullptr)
// {
//     stringstream ss;
//     if (extra) {
//         ss << extra;
//     }
//     ss << "Core id: " << xPortGetCoreID()
//        << ", prio: " << uxTaskPriorityGet(nullptr)
//        << ", minimum free stack: " << uxTaskGetStackHighWaterMark(nullptr) << " bytes.";
//     ESP_LOGI(pcTaskGetName(nullptr), "%s", ss.str().c_str());
// }

// void thread_func_inherited()
// {
//     while (true) {
//         print_thread_info("This is the INHERITING thread with the same parameters as our parent, including name. ");
//         this_thread::sleep_for(sleep_time);
//     }
// }

// void spawn_another_thread()
// {
//     // Create a new thread, it will inherit our configuration
//     thread inherits(thread_func_inherited);

//     while (true) {
//         print_thread_info();
//         this_thread::sleep_for(sleep_time);
//     }
// }

// void thread_func_any_core()
// {
//     while (true) {
//         print_thread_info("This thread (with the default name) may run on any core.");
//         this_thread::sleep_for(sleep_time);
//     }
// }

// void thread_func()
// {
//     while (true) {
//         print_thread_info();
//         this_thread::sleep_for(sleep_time);
//     }
//     }

// Thread config creator
// esp_pthread_cfg_t create_config(const char *name, int core_id, int stack, int prio)
// {
//     auto cfg = esp_pthread_get_default_config();
//     cfg.thread_name = name;
//     cfg.pin_to_core = core_id;
//     cfg.stack_size = stack;
//     cfg.prio = prio;
//     return cfg;
// }


extern "C" [[noreturn]] void app_main(void)
{

    // Инициализация ШИМ-контроллера
    PwmCtrl1.initialize();

    // wifi_initialize(WIFI_MODE_APSTA);
    
    // // Create a thread using default values that can run on any core
    // auto cfg = esp_pthread_get_default_config();
    // esp_pthread_set_cfg(&cfg);
    // std::thread any_core(thread_func_any_core);

    // // Create a thread on core 0 that spawns another thread, they will both have the same name etc.
    // cfg = create_config("Thread 1", 0, 3 * 1024, 5);
    // cfg.inherit_cfg = true;
    // esp_pthread_set_cfg(&cfg);
    // std::thread thread_1(spawn_another_thread);

    // // Create a thread on core 1.
    // cfg = create_config("Thread 2", 1, 3 * 1024, 5);
    // esp_pthread_set_cfg(&cfg);
    // std::thread thread_2(thread_func);

    // Start Software Timer
    xTimers[0] = xTimerCreate("myTimer", 200, pdTRUE, (void*) 0, vMyTimer_callback);
    if( xTimerStart(xTimers[0], 0 ) != pdPASS ) {
        ESP_LOGE(TAG,"Error starting timer.");
    }

    // CLI initialization
    CConsoleExecutor::init();

     // Let the main task do something too
    while (true) {
         this_thread::sleep_for(sleep_time);
    }
}
