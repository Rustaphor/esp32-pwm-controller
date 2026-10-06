---
name: СPwmCmd
description: Created СPwmCmd class inheriting from CFanMotor and AConsole2Cmd (renamed from CFanMotorCmd)
metadata:
  type: project
---

Created СPwmCmd class that inherits from both CFanMotor (motor control functionality) and AConsole2Cmd (console command interface). This allows controlling the fan motor via console commands like "pwm on", "pwm off", "pwm --help", and "pwm -p 50" to set power percentage.

Files created:
- main/СPwmCmd.h - Header file with class declaration
- main/СPwmCmd.cpp - Implementation file with constructor, destructor, and command processing

The class follows the same pattern as CMotCtrlCmd but for fan motor control. It uses argtable3 for argument parsing and provides commands to turn the motor on/off, set power percentage, and display status.

Why: Need to consolidate motor control functionality with console command interface for easier testing and control. Renamed from CFanMotorCmd to СPwmCmd per user request.
How to apply: Instantiate СPwmCmd and register it with AConsole2::registerCommand() to make it available in the CLI.