# Description

The goal of this project is to use a **Motorola T82 talkie-walkie** as an autonomous **antitheft system**. A tiny PCB composed of a regulator, an MCU and an accelerometer is internally connected to some interface signals of the talkie. Once armed, the board is able to **wake-up the T82 and trigger the PTT button** when suspicious movement is detected on the protected asset.

# Embedded software

## Environment

The embedded software is developed under **Eclipse IDE** version 2025-06 (4.36.0) and **GNU MCU** plugin. The `script` folder contains Eclipse run/debug configuration files and **JLink** scripts to flash the MCU.

## Target

The T82 Accelero board is based on the **STM32L011F4U6** microcontroller of the STMicroelectronics L0 family. Each hardware revision has a corresponding **build configuration** in the Eclipse project, which sets up the code for the selected board version.

## Structure

The project is organized as follow:

* `drivers` :
    * `device` : MCU **startup** code and **linker** script.
    * `registers` : MCU **registers** address definition.
    * `peripherals` : internal MCU **peripherals** drivers.
    * `components` : external **components** drivers.
    * `utils` : **utility** functions.
* `application` : Main **application**.