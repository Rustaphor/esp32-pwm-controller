#include "consolexec.h"
#include "esp_log.h"
#include <stdint.h>
#include "pwmctrl.h"
#include <iostream>
#include <utility>

static const char* logTAG = __FILE_NAME__;       // Локальный тег логирования модуля

extern CPwmControl PwmCtrl1;


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
    bool any_changed = false;
    for (const auto& [key, val] : executor.getOpts()) {

        // Обработка команд
        if (val == CONSOLE_PWMCTRL_CMD_ON) {
            PwmCtrl1.enable();      // TODO: вынести из цикла единым запуском
            any_changed = true;
            continue;
        } else if (val == CONSOLE_PWMCTRL_CMD_OFF) {
            PwmCtrl1.disable();
            any_changed = true;
            continue;
        }


        float dc;
        uint32_t freq;
        switch (key) {

            // Freq parameters
            case 'f':
                freq = atoi(val.data());
                PwmCtrl1.initialize(freq);
                any_changed = true;
                break;

            // DC parameters
            case 'd':
                dc = (float) atof(val.data());
                PwmCtrl1.setDutyCycle(dc);
                PwmCtrl1.update();
                any_changed = true;
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

    if (!any_changed) {
        cout << "PWM-status: ";
        if (PwmCtrl1.isEnabled()) {
            cout << "ON ";
        } else {
            cout << "OFF ";
        }

        PwmCtrl1.update();

        pair<uint32_t,uint32_t> sys_dc = PwmCtrl1.getSysDC();
        cout << "dc=" << PwmCtrl1.getDutyCycle()*100<< "% (" << sys_dc.first <<'/' << sys_dc.second << ") freq=" << PwmCtrl1.getFrequency() << "Hz pin#=" << PwmCtrl1.getPin() << endl;
    }

    return ESP_OK;
}
