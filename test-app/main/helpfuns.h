#pragma once
#include <stdlib.h>

// Функция проверки относительной точности
template<typename T>
static bool check2ValuesByTolerance(T val1, T val2, const float relTol) {
    if (val1 == val2) return true;
    else if (val1 == 0 || val2 == 0) {
        if (val1 + val2 <= relTol) return true;
    }
    else if (val1 > val2 && (val1 - val2) / val1 * 100.0f <= relTol) {
        return true;
    }
    else if (val2 > val1 && (val2 - val1) / val2 * 100.0f <= relTol) {
        return true;
    }

    return false;
}

// Функция проверки относительной точности
template<typename T>
static float calcRelTolErr(const T val1, const T val2) {

    float result = 0;
    if (val1 == 0 || val2 == 0 ){
        
    } elseif (val1 > val2) {
        result = (val1 - val2)/val1 * 100.0f;
    } elseif (val1 < val2) {
        result = (val2 - val1)/val2 * 100.0f;
    }

    return result;
}

// Функция проверки сравнительной точности
template<typename T>
static bool check2ValuesByDelta(T val1, T val2, const T max_delta) {
    if (val1 == val2) return true;
    else if (val1 > val2 && val1-val2 <= max_delta) {
        return true;
    }
    else if (val2 > val1 && val2-val1 <= max_delta) {
        return true;
    }

    return false;
}

