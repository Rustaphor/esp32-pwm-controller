
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

#include "esp_log.h"
#include "esp_console.h"
#include "cmd_system.h"


// Локальные заголовки
#include "pwmconhelp.h"


/*
 * We warn if a secondary serial console is enabled. A secondary serial console is always output-only and
 * hence not very useful for interactive console applications. If you encounter this warning, consider disabling
 * the secondary serial console in menuconfig unless you know what you are doing.
 */
#if SOC_USB_SERIAL_JTAG_SUPPORTED
#if !CONFIG_ESP_CONSOLE_SECONDARY_NONE
#warning "A secondary serial console is not useful when using the console component. Please disable it in menuconfig."
#endif
#endif


static const char* TAG = __FILE_NAME__;       // Локальный тег логирования модуля
#define PROMPT_STR CONFIG_IDF_TARGET

using namespace std;
using namespace std::chrono;


 #define NUM_TIMERS 1
 TimerHandle_t xTimers[NUM_TIMERS];
 
 const auto sleep_time = seconds {
    10
};

 void vMyTimer_callback(TimerHandle_t xTimer)
 {
    ESP_LOGD(TAG, "Numer of iteration %d", 4);
 }



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


extern "C" void app_main(void)
{
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

    // Initialize Console system
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc. */
    repl_config.prompt = PROMPT_STR ">";
    repl_config.max_cmdline_length = CONFIG_CONSOLE_MAX_COMMAND_LINE_LENGTH;

    /* Register commands */
    esp_console_register_help_command();
    register_system_common();
    esp_console_register_pwmcontrol_command();

#if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));
#endif

    /* Run Console Thread */
    ESP_ERROR_CHECK(esp_console_start_repl(repl));

    // Let the main task do something too
    while (true) {
         this_thread::sleep_for(sleep_time);
    }
}
