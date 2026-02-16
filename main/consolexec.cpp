#include "consolexec.h"
#include "esp_log.h"
#include <stdint.h>

// For Debug
#include <iostream>




static const char* logTAG = __FILE_NAME__;       // Локальный тег логирования модуля

map<char, string> &CConsoleExecutor::getOpts(int argc, char **argv)
{
    // this->mArgs.clear();

    bool chk_param = false;
    map<char, string>::iterator it;

    for (int i = 0; i < argc; ++i) {
        string str{argv[i]};

        // Сканирование флагов
        if (str.starts_with("-")) {
            string str2(str.substr(1));
            for (uint8_t i2 = 0; i2 < str2.length(); i2++) {
                it = mArgs.insert({str2[i2],""}).first;
            };

            chk_param = true;
            continue;
        }

        // Проверка флагов на наличие параметров
        if (chk_param) {
            const char K = it->first;
            mArgs.erase(it);
            mArgs.insert({K,str});

            chk_param = false;
            continue;
        }

        // Вставка команд
        mArgs.insert({(char) i, str});
    }
    

    return this->mArgs;
}

int CConsoleExecutor::exec_pwmctrl_callback(int argc, char **argv)
{

    CConsoleExecutor executor(argc,argv);

    // Анализ командной строки
    for (const auto& [key, val] : executor.getOpts()) {
        cout << "Key: " << key << " value: " << val << endl;

        // Обработка команд
        if (val == CONSOLE_PWMCTRL_CMD_ON) {
            cout << "Command on" << "\n";
            continue;
        } else if (val == CONSOLE_PWMCTRL_CMD_OFF) {
            cout << "Command off" << "\n";
            continue;
        }


        float dc;
        uint32_t freq;
        switch (key) {

            // Freq parameters
            case 'f':
                freq = atoi(val.data());
                cout << "Current f = " << freq << " \n";
                break;

            // DC parameters
            case 'd':
                dc = (float) atof(val.data());
                cout << "Current dc = " << dc << " \n";
                break;

            // Inverse flag
            case 'i':
                cout << "Inverse option" << endl;
                break;

            // Test flag
            case 't':
                return ESP_FAIL;

        }

    }
    return ESP_OK;
}
