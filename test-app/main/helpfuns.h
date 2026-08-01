#pragma once
#include <stdlib.h>

// Функция проверки относительной точности
template<typename T>
static T calcRelTolErr(const T val1, const T val2) {

    T result = 0;
    if (val1 > val2) {
        if (val1 && val2) {
            result = (val1 - val2)/val1;
        } else {
            result = abs(val1 - val2);
        }
    } else if (val1 < val2) {
        if (val1 && val2) {
            result = (val2 - val1)/val2;
        } else {
            result = abs(val1 - val2);
        }
    }

    return result;
}

// Функция проверки относительной точности
template<typename T>
static bool checkValuesByTolPercents(T val1, T val2, const float relTol) {
	if (calcRelTolErr(val1,val2) * 100.0f <= relTol) return true;
    return false;
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

