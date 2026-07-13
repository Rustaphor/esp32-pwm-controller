#include "CMotorCtrl.h"
#include "argtable3/argtable3.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_err.h"


static const char* logTAG = "MotorControl";

using namespace std;

// Реализация виртуальной функции
int CMotorCtrl::exec_func_cb(int argc, char* argv[]) {

    // TODO: в бибилиотеке нало обязательно блокировать семафор
    int exitcode = ESP_OK;
    bool emptyCmd = true;

    /* global arg_xxx structs */
    arg_lit_t *pHelp, *pCmdOn, *pCmdOff;
    arg_int_t *pFreq, *pPwr;
    arg_end_t *pEnd;


    void *argtable[] = {
        pHelp   = arg_lit0(NULL, "help", "display this pHelp and exit"),
        pCmdOn  = arg_lit0(NULL, "on", "switch motor on"),
        pCmdOff = arg_lit0(NULL, "off", "switch motor off"),
        pFreq   = arg_int0("f", "freq", "<n>", "set pwm frequency (30-100) kHz"),
        pPwr    = arg_int0("p", "power", "<n>", "set output power (0-100) percents"),
        pEnd    = arg_end(20)
    };

    int nerrors = arg_parse(argc,argv,argtable);

    if (pHelp->count > 0)
    {
        printf("Usage: %s", _command);
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-25s %s\n");
        goto exit;
    }

    if (pCmdOff->count > 0)
    {
        // TODO: включить мотор
        emptyCmd = false;
    } else if (pCmdOn->count > 0) {
        //TODO выключить
        emptyCmd = false;
    }

    if (pFreq->count > 0) {
        // TODO: проверить допустимость значений и изменить частоту ШИМ
        emptyCmd = false;
    }

    if (pPwr->count > 0) {
        // TODO: проверить допустимость значений мощности и изменить мощность
        emptyCmd = false;
    }

    /* If the parser returned any errors then display them and exit */
    if (nerrors > 0)
    {
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors(stderr, pEnd, _command);
        exitcode = ARG_EMISSARG;
        goto exit_n_help;
    } else if (emptyCmd) {
        goto exit_n_help;
    }

    // TODO: Выполнить команду мотора
    ESP_LOGI(logTAG, "Motor command run [ToDo]!");
    goto exit;

exit_n_help:
    printf("Try '%s --help' for more information.", _command);

exit:
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    if (exitcode) {
        ESP_LOGE(logTAG, "Command %s exit with error #%d", _command, exitcode);
    }
    return exitcode;
}

