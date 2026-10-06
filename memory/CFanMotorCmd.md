---
name: CFanMotorCmd
description: Created CFanMotorCmd class inheriting from CFanMotor and AConsole2Cmd
metadata:
  type: project
---

Created CFanMotorCmd class that inherits from both CFanMotor (motor control functionality) and AConsole2Cmd (console command interface). This allows controlling the fan motor via console commands like "fanmotor on", "fanmotor off", "fanmotor --help", and "fanmotor -p 50" to set power percentage.

Files created:
- main/CFanMotorCmd.h - Header file with class declaration
- main/CFanMotorCmd.cpp - Implementation file with constructor, destructor, and command processing

The class follows the same pattern as CMotCtrlCmd but for fan motor control. It uses argtable3 for argument parsing and provides commands to turn the motor on/off, set power percentage, and display status.

Why: Need to consolidate motor control functionality with console command interface for easier testing and control.
How to apply: Instantiate CFanMotorCmd and register it with AConsole2::registerCommand() to make it available in the CLI.