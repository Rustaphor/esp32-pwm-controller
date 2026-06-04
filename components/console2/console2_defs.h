#pragma once 

#include "sdkconfig.h"

// Строка приветствия
#define CONSOLE2_PROMPT_STR CONFIG_IDF_TARGET
#define CONSOLE2_MAX_CMDLINE_ARGS 8
#define CONSOLE2_PROMPT_MAX_LEN (32)
#define CONSOLE2_HISTORY_BUFF_SIZE 100

char prompt[CONSOLE2_PROMPT_MAX_LEN]; // Prompt to be printed before each line

