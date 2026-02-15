#include "pwmconexec.h"
#include "esp_log.h"

// For Debug
#include <iostream>
#include <string>
#include <stdint.h>


static const char* logTAG = __FILE_NAME__;       // Локальный тег логирования модуля

using namespace std;

int CPwmCtrlConsoleExecutor::command_callback(int argc, char **argv)
{
    char f_par = 0;
    for (int i = 0; i < argc; ++i) {
        string str{argv[i]};

        if (f_par) {

            float par; uint32_t freq;
            switch (f_par) {

                // Freq parameters
                case 'f':
                    freq = atoi(argv[i]);
                    if (freq >= 0) {
                        cout << "Current f = " << freq << " \n";
                    };
                    break;

                // DC parameters
                case 'd':
                    par = (float) atof(argv[i]);
                    if (par >= 0) {
                        cout << "Current d = " << par << " \n";
                    };
                    break;

            }
            
            f_par = false;
            continue;
        }

        // Обработка флагов
        if (str.starts_with("-")) {
            string str2(str.substr(1));
            for (uint8_t i2 = 0; i2 < str2.length(); i2++) {
                switch (str2[i2])
                {
                    case 'f':       // Флаг установки частоты
                        f_par = str2[i2];
                        goto end_parse;
                    default:
                        ESP_LOGW(logTAG,"Unknow flag -%s",str2[i2]);
                } 
            };

            continue;
        }

        // Обработка команд
        if (str == CONSOLE_PWMCTRL_CMD_ON) {
            cout << "Command on" << "\n";
        } else if (str == CONSOLE_PWMCTRL_CMD_OFF) {
            cout << "Command off" << "\n";
        }

end_parse:

    }

    return ESP_OK;
}
