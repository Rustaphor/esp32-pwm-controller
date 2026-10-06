// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#include "CPwmCmd.h"
#include "argtable3/argtable3.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include "esp_log.h"
#include "esp_err.h"


static const char* logTAG = "PwmCmd";

// Реализация виртуальной функции
int CPwmCmd::exec_func_cb(int argc, char* argv[]) {

    // TODO: в бибилиотеке нало обязательно блокировать семафор
    int exitcode = ESP_OK;
    bool emptyCmd = true;

    /* global arg_xxx structs */
    arg_lit_t *pHelp, *pCmdOn, *pCmdOff;
    arg_dbl_t *pDC;
    arg_int_t *pVal;
    arg_end_t *pEnd;


    void *argtable[] = {
        pHelp   = arg_lit0(NULL, "help", "display this pHelp and exit"),
        pCmdOn  = arg_lit0(NULL, "on", "switch motor on"),
        pCmdOff = arg_lit0(NULL, "off", "switch motor off"),
        pDC     = arg_dbl0("d", "dc", "<n>", "set duty cycle (DC) output [0-100] in percents"),
        pVal    = arg_int0("v", "val" "<n>", "set counter value directly in range [0-" ACMOT_SINE_MAX_VALUE "]"),
        pEnd    = arg_end(20)
    };

    int nerrors = arg_parse(argc,argv,argtable);

    // Check Help flag first
    if (pHelp->count > 0)
    {
        printf("Usage: %s\n", _command);
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-25s %s\n");
        goto exit;
    }

    // Check percentage DC
    if (pDC->count > 0) {
        emptyCmd = false;
        // TODO: Проверить значение на диапазон [0-100]
        if (exitcode == ACMOT_ERR_INVALID_POWER){
            printf("Invalid set power value. The value must be in range [0-100]\n");
            goto exit;
        }

        // TODO: Изменить значение счетчика ШИМ напрямую в процентах
    }

    // Check direct counter value
    if (pVal->count > 0) {
        emptyCmd = false;
        // TODO: Проверить значение на диапазон [0-ACMOT_SINE_MAX_VALUE]
        if (exitcode == ACMOT_ERR_INVALID_POWER){
            printf("Invalid set pwm value. The value must be in range [0-" ACMOT_SINE_MAX_VALUE "]\n");
            goto exit;
        }

        // TODO: Изменить значение счетчика ШИМ напрямую
    }

    // Start or stop PWM
    if (pCmdOff->count > 0) {
        emptyCmd = false;
        // exitcode = _motor.stop();
    } else if (pCmdOn->count > 0) {
        emptyCmd = false;
        // exitcode = _motor.run();
        // _value = _motor._currentPower;
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

    disp_pwm_status();
    goto exit;

exit_n_help:
    disp_pwm_status();
    printf("Try '%s --help' for more information.\n", _command);

exit:
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    if (exitcode) {
        ESP_LOGE(logTAG, "Command %s exit with error 0x%X", _command, exitcode);
    }
    return exitcode;
}

void CPwmCmd::disp_pwm_status() const
{
    // Show current motor status
    auto state = _motor.getCurrentState();
    const char* c_state = _getMotorSysStateString(state);
    const char* m_state = _getMotorOnlyStateString((AcMotorState_t) state.reserved);

do_print:
    printf("Fan motor current status:\n\r");
    printf(" state: %s, %s\n", c_state, m_state);
    printf(" DC: %4.2f %% \n", _motor.getPowerOutPercent());
    printf(" value: %d \n", _motor._currentPower);
}

bool CPwmCmd::_isMotorAvalableOrPrintError(devState_t expectedState) const noexcept
{
    AcMotorState_t motState = (AcMotorState_t) expectedState.reserved;
    if (expectedState.sysState == DEV_INITIALIZED) {
        if (motState == AC_MOTOR_IS_STOPPED) {
            return true;
        }
    }

    printf("Motor is unavailable state '%s' and '%s'. It must be initialized and stopped.\n", _getMotorSysStateString(expectedState), _getMotorOnlyStateString(motState));        
    return false;
}

char *CPwmCmd::_getMotorOnlyStateString(AcMotorState_t state) const noexcept
{
    switch (state) {
        case AC_MOTOR_IS_STOPPED:
            return "stopped";
        case AC_MOTOR_IS_RUNNING:
            return "running";
        case AC_MOTOR_IN_FAILURE:
            return "failure";
        default:
            return "unknown";
    }
    return "\0";
}

char *CPwmCmd::_getMotorSysStateString(devState_t state) const noexcept
{
    switch (state.sysState) {
        case DEV_NOT_INITIALIZED:
            return "not initialized";
        case DEV_INITIALIZED:
            return "initialized";
        case DEV_FAILURE:
            return "failure";
        case DEV_BUSY:
            return "busy";
        default:
            return "unknown";
    }
    return "\0";
}
