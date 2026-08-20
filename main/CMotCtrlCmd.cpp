#include "CMotCtrlCmd.h"
#include "argtable3/argtable3.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_err.h"
#include "CFanMotor.h"


extern CFanMotor fanmot;
static const char* logTAG = "MotorControl";

using namespace std;


void disp_motor_status(void){
    // TODO: показать текущий статус мотора
    auto state = fanmot.getCurrentState();
    const char* c_state;
    if (state.sysState == DEV_INITIALIZED) {
        c_state = "initialized";
        goto do_print;
    } else if(state.sysState==DEV_NOT_INITIALIZED){
        c_state = "not initialized";
        goto do_print;
    }

    switch ((AcMotorState_t) state.reserved) {
        case AC_MOTOR_IS_STOPPED:
            c_state = "stopped";
            break;
        case AC_MOTOR_IS_RUNNING:
            c_state = "running";
            break;
        case AC_MOTOR_IN_FAILURE:
            c_state = "failure";
            break;
        default:
            c_state = "unknow";
    }

do_print:
    printf("Motor current status:\n\r");
    printf(" state: %s\n", c_state);
    printf(" power: %4.2f %% \n", fanmot.getPowerOutPercent());
    //  printf("\tfreq: %d Hz\n", fanmot.);
}


// Реализация виртуальной функции
int CMotCtrlCmd::exec_func_cb(int argc, char* argv[]) {

    // TODO: в бибилиотеке нало обязательно блокировать семафор
    int exitcode = ESP_OK;
    bool emptyCmd = true;

    /* global arg_xxx structs */
    arg_lit_t *pHelp, *pCmdOn, *pCmdOff;
    arg_dbl_t *pPwr;
    arg_end_t *pEnd;


    void *argtable[] = {
        pHelp   = arg_lit0(NULL, "help", "display this pHelp and exit"),
        pCmdOn  = arg_lit0(NULL, "on", "switch motor on"),
        pCmdOff = arg_lit0(NULL, "off", "switch motor off"),
        // pFreq   = arg_int0("f", "freq", "<n>", "set pwm frequency (30-100) kHz"),
        pPwr    = arg_dbl0("p", "power", "<n>", "set output power (0-100) percents"),
        pEnd    = arg_end(20)
    };

    int nerrors = arg_parse(argc,argv,argtable);

    if (pHelp->count > 0)
    {
        printf("Usage: %s\n", _command);
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-25s %s\n");
        goto exit;
    }

    if (pPwr->count > 0) {
        emptyCmd = false;
        exitcode = fanmot.setPowerPercents((float) *pPwr->dval);
        if (exitcode == AC_ERR_MOTOR_INVALID_POWER){
            printf("Invalid set power value. The value must be in range [0-100]\n");
            goto exit;
        }
    }

    if (pCmdOff->count > 0) {
        emptyCmd = false;
        exitcode = fanmot.stop();
    } else if (pCmdOn->count > 0) {
        emptyCmd = false;
        exitcode = fanmot.run();
    }
    if (exitcode) goto exit;

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

    disp_motor_status();
    goto exit;

exit_n_help:
    disp_motor_status();
    printf("Try '%s --help' for more information.\n", _command);

exit:
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    if (exitcode) {
        ESP_LOGE(logTAG, "Command %s exit with error #%d", _command, exitcode);
    }
    return exitcode;
}

