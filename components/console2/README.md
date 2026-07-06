# Console2 Component

## Overview

Console2 — компонент для ESP-IDF, предоставляющий интерактивную интерфейсы консольных оболочек. В библиотеке уже реализован компонент UART. Компонент реализует объектно-ориентированный подход к управлению консолью и командами представляет удобную оболочку C++ встроенного в ESP-IDF компонента `console`, используя FreeRTOS для многозадачности. Легковесная библиотека *linenoise* для продвинутой работы с командной строкой.

## Features

- Объектно-ориентированная архитектура на C++
- Поддержка регистрации и отмены регистрации пользовательских команд
- Встроенные команды: `help`, `exit`, `info`
- История команд с возможностью сохранения в память
- Автодополнение команд по нажатию Tab
- Навигация по истории команд с помощью стрелок Up/Down
- Мультистрочный ввод команд
- Конфигурируемые параметры через menuconfig

## Architecture

### Основные классы

#### `AConsole2` (абстрактный базовый класс)

Класс `AConsole2` предоставляет общий интерфейс для управления консолью:

- `initialize()` — инициализация периферии консоли
- `run()` — запуск консоли (создание задачи FreeRTOS)
- `stop()` — остановка консоли
- `registerCommand()` — регистрация пользовательской команды
- `unregisterCommand()` — удаление команды из списка
- `getState()` — получение текущего состояния консоли

#### `AConsole2Cmd` (абстрактный класс команды)

Базовый класс для создания пользовательских команд:

- Конструктор принимает имя команды, описание и подсказки
- Метод `exec_func_cb()` — обработчик команды (должен быть переопределен в производных классах)
- Вспомогательный метод `getOpts()` для парсинга аргументов и флагов

#### `CUartConsole2` (реализация UART-консоли)

Класс для использования UART в качестве интерфейса консоли:

- Создает отдельную задачу FreeRTOS для обработки консольного ввода
- Поддерживает конфигурируемый номер UART порта и размер буфера
- Использует драйверы UART/USB-SERIAL-JTAG/USB-CDC в зависимости от конфигурации

### Состояния консоли

- `CONSOLE_STATUS_NOT_INITIALIZED` — консоль не инициализирована
- `CONSOLE_STATUS_INITIALIZED` — консоль инициализирована, но не запущена
- `CONSOLE_STATUS_RUNNED` — консоль активна и обрабатывает команды

## Usage Example

### 1. Создание пользовательской команды

```cpp
#include "AConsole2Cmd.h"

class MyCommand : public AConsole2Cmd {
public:
    MyCommand() : AConsole2Cmd("mycmd", "My custom command") {}
    
    int exec_func_cb(int argc, char* argv[]) override {
        printf("MyCommand executed!\n");
        return 0;
    }
};
```

### 2. Регистрация и использование консоли

```cpp
#include "CUartConsole2.h"

// Создание экземпляра консоли
CUartConsole2 console;

// Инициализация периферии
console.initialize();

// Регистрация пользовательской команды
MyCommand myCmd;
console.registerCommand(myCmd);

// Запуск консоли
console.run();

// В бесконечном цикле (например, в idle-задаче) вызывать dispatch()
console.dispatch();
```

### 3. Команда info

Компонент включает встроенную команду `info`, которая выводит информацию об архитектуре ESP32:
- Модель чипа
- Количество ядер
- Поддерживаемые возможности (WiFi, Bluetooth, BLE и т.д.)
- Версия ESP-IDF
- Размер флеш-памяти

## Configuration

Конфигурация компонента выполняется через `menuconfig`:

```
Component config > Console2 Component
```

| Параметр | По умолчанию | Описание |
|----------|-------------|----------|
| `CONSOLE2_STORE_HISTORY` | `y` | Сохранение истории к��манд в память |
| `CONSOLE2_PRESSENTER_MSG` | `y` | Показ сообщения о входе в консоль |
| `CONSOLE2_MAX_COMMAND_LINE_LENGTH` | `256` | Максимальная длина командной строки |
| `CONSOLE2_TASK_HEAP_STACK_SIZE` | `3072` | Размер стека задачи консоли (байты) |
| `CONSOLE2_GREETINGS_MESSAGE` | "Welcome to the console command line interface" | Приветственное сообщение |

## Dependencies

- `console` (ESP-IDF)
- `cmd_system` (из примеров ESP-IDF)
- `esp_driver_uart`
- `esp_driver_usb_serial_jtag`
- `esp_hw_support`

## Requirements

- ESP-IDF >= 5.0
- C++17 compatible compiler

## License

Этот компонент распространяется в рамках проекта ESP32-PWM-Controller.
