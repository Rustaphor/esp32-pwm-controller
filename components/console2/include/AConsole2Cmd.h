#pragma once

#include <string>
#include <map>
#include "esp_console.h"
#include "esp_log.h"

class AConsole2;

using namespace std;


#define console2_logTAG     "CONSOLE2"

class AConsole2Cmd {

    esp_console_cmd_t _console_cmd;

    friend class AConsole2;

public:
    const char* getCommand() const { return _console_cmd.command; };

    virtual int exec_func_cb(int argc, char* argv[]) = 0;

protected:

    AConsole2Cmd(const char* command, const char* help = NULL, const char* hint = NULL) {
        _console_cmd = {
            .command = command,
            .help = help,
            .hint = hint,
            .func = NULL,
            .argtable = NULL,
            .func_w_context = exec_func_cb,
            .context = this
        };
    };

    AConsole2Cmd(const char* command, const char* help, const char* hint, int (*func)(int, char**)) {
        _console_cmd = {
            .command = command,
            .help = help,
            .hint = hint,
            .func = func,
            .argtable = NULL,
            .func_w_context = NULL,
            .context = this
        };
    };


    // Функция обратного вызова команды для сопряжения с библиотекой esp_console
    static int exec_func_cb(void *context, int argc, char** argv) {
        AConsole2Cmd *cmd = (AConsole2Cmd*) context;
        ESP_LOGD(console2_logTAG, "Run console2 command: %s", cmd->_console_cmd.command);
        return cmd->exec_func_cb(argc, argv);
    };

    // Парсер параметров и ключей, начинающихся символом '-'
    static map<const char,string> &getOpts(map<const char,string> &arglist, int argc, char **argv);
};
