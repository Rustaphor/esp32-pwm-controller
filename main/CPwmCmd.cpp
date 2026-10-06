// SPDX-License-Identifier: MIT
// Copyright (c) 2026
#include "CPwmCmd.h"
#include "argtable3/argtable3.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include <cstdlib>
#include "esp_log.h"
#include "esp_err.h"
#include "mcpwm_private.h"


static const char* logTAG = "PwmCmd";


// Реализация виртуальной функции
int CPwmCmd::exec_func_cb(int argc, char* argv[]) {

    // TODO: в бибилиотеке нало обязательно блокировать семафор
    int exitcode = ESP_OK;
    bool emptyCmd = true;

    /* global arg_xxx structs */
    arg_lit_t *pHelp, *pCmdOn, *pCmdOff;
    // arg_dbl_t *pDC;
    arg_int_t *pVal;
    arg_end_t *pEnd;
    char* _buff = (char*) calloc(50, sizeof(char));
    sprintf(_buff, "set counter value directly in range [0-%d]", ACMOT_SINE_MAX_VALUE);

    void *argtable[] = {
        pHelp   = arg_lit0(NULL, "help", "display this pHelp and exit"),
        pCmdOn  = arg_lit0(NULL, "on", "switch motor on"),
        pCmdOff = arg_lit0(NULL, "off", "switch motor off"),
        // pDC     = arg_dbl0("d", "dc", "<n>", "set duty cycle (DC) output [0-100] in percents"),
        pVal    = arg_int0("v", "val", "<n>", _buff),
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
    // if (pDC->count > 0) {
    //     emptyCmd = false;
    //     if (_isMotorAvalableOrPrintError()) {
    //         exitcode = _motor.setPowerPercents((float) *pDC->dval);
    //         if (exitcode == ACMOT_ERR_INVALID_POWER){
    //             printf("Invalid set DC value. The value must be in range [0-100] percents\n");
    //             goto exit;
    //         }
    //     }
    // }

    // Check direct counter value
    if (pVal->count > 0) {
        emptyCmd = false;
        if (_isMotorAvalableOrPrintError()) {
            exitcode = set_counter_value((acmot_sineval_t) *pVal->ival);
            // Проверка допустимого значения
            if (exitcode) goto exit;
        }
    }

    // Start or stop PWM
    if (pCmdOff->count > 0) {
        emptyCmd = false;
        exitcode = pwm_stop();
    } else if (pCmdOn->count > 0) {
        emptyCmd = false;
        exitcode = pwm_start();
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

    _disp_pwm_status();
    goto exit;

exit_n_help:
    _disp_pwm_status();
    printf("Try '%s --help' for more information.\n", _command);

exit:
    /* deallocate each non-null entry in argtable[] */
    free(_buff);
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    if (exitcode) {
        ESP_LOGE(logTAG, "Command %s exit with error 0x%X", _command, exitcode);
    }
    return exitcode;
}

esp_err_t CPwmCmd::pwm_start() noexcept
{
    _motor.sem.acquire();
    esp_err_t result = ESP_OK;

    if (_isMotorAvalableOrPrintError()) goto exit_error;
    result = set_counter_value(_value);
    if (result != ESP_OK) goto exit_error;

    result = mcpwm_timer_register_event_callbacks(_motor.hTimer_, NULL, &_motor);
    if (result) goto exit_error;
    result = _motor.hw_run(_value);
    printf("WARNING! Don't run motor command it's will be not working.\n");

exit_error:
    _motor.sem.release();
    return result;
}

esp_err_t CPwmCmd::pwm_stop() noexcept
{
    _motor.sem.acquire();
    esp_err_t result = ESP_OK;

    if (_isMotorAvalableOrPrintError()) goto exit_error;
    result = _motor.hw_stop();

exit_error:
    _motor.sem.release();
    return result;
}

esp_err_t CPwmCmd::set_counter_value(acmot_sineval_t val) noexcept
{
    if (val <= 0) {
        printf("PWM value is zero, nothing to start.\n");
        return ESP_ERR_INVALID_ARG;
    } else if (val > ACMOT_SINE_MAX_VALUE) {
        printf("PWM value is out of range [0-%d], unavailable to start.\n", ACMOT_SINE_MAX_VALUE);
        return ESP_ERR_INVALID_ARG;
    }

    _value = val;
    return mcpwm_comparator_set_compare_value(_motor.hComparator_, _value);
}

void CPwmCmd::_disp_pwm_status() const
{
    // Show current motor status
    auto state = _motor.getCurrentState();
    const char* c_state = _getMotorSysStateString(state);
    const char* m_state = _getMotorOnlyStateString((AcMotorState_t) state.reserved);

    printf("Fan motor current status:\n\r");
    printf(" state: %s, %s\n", c_state, m_state);
    printf(" DC: %4.2f %% \n", _motor.getPowerOutPercent());
    printf(" value: %d \n", _value);
}

bool CPwmCmd::_isMotorAvalableOrPrintError() const noexcept
{
    auto cur_state = _motor.getCurrentState();
    if (cur_state.sysState == DEV_INITIALIZED) {
        if (cur_state.reserved == (dev_state_reserved_t) AC_MOTOR_IS_STOPPED) {
            return true;
        }
    }

    printf("Motor is unavailable, current states are '%s' and '%s'. It must be initialized and stopped.\n", _getMotorSysStateString(cur_state), _getMotorOnlyStateString((AcMotorState_t) cur_state.reserved));
    return false;
}

const char *CPwmCmd::_getMotorOnlyStateString(AcMotorState_t state) const noexcept
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

const char *CPwmCmd::_getMotorSysStateString(devState_t state) const noexcept
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
