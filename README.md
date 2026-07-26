# Description

The goal of this project is to use a **Motorola T82 talkie-walkie** as an autonomous **antitheft system**. A tiny PCB composed of a regulator, an MCU and an accelerometer is internally connected to some interface signals of the talkie. Once armed, the board is able to **wake-up the T82 and trigger the PTT button** when suspicious movement is detected on the protected asset.

# Embedded software

## Environment

The firmware is developed under **Eclipse IDE** and **GNU MCU** plugin. The `script` folder contains Eclipse run/debug configuration files and **JLink** scripts to flash the MCU.

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

## Build

The project can be compiled by command line with `cmake`.

```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE="script/cmake-arm-none-eabi/toolchain.cmake" \
      -DTOOLCHAIN_PATH="<arm_none_eabi_gcc_path>" \
      -DT82_ACCELERO_HW_VERSION="<cmake_hw_version>" \
      -DT82_ACCELERO_ARMING_PRESS_DURATION_SECONDS=5 \
      -DT82_ACCELERO_ARMING_TIME_SECONDS=30 \
      -DT82_ACCELERO_ARMING_LED_PULSE_MS=200 \
      -DT82_ACCELERO_ARMING_LED_PERIOD_MS=1000 \
      -DT82_ACCELERO_ACTIVE_LED_PULSE_MS=200 \
      -DT82_ACCELERO_ACTIVE_LED_PERIOD_MS=3000 \
      -DT82_ACCELERO_MOTION_IRQ_COUNT_THRESHOLD=5 \
      -G "Unix Makefiles" ..
make all
```

## Flash

### Preparation

* **Build** the desired version (with IDE or `cmake`) or **download** a specific [firmware release](https://github.com/Ludovic-Lesur/t82-accelero/releases) (expand the `Assets` menu, download the corresponding artifact and extract the binary files from the `zip`).
* Connect the flashing tool to the **J6 connector** located in the corner of the PCB (standard SWD pinout).

### ST-Link on Nucleo board

* Make sure that the ST-LINK/NUCLEO jumpers (generally designated by **CN2**) are not fitted, in order to **select the external programming connector** instead of the internal MCU.
* An **MSC disk** named `NODE_XXXXXX` should be mounted by the system after USB plugging. If not, download the [ST Cube Programmer](https://www.st.com/en/development-tools/stm32cubeprog.html) software which will install the required drivers. If the MSC disk is still not mounted, follow the ST-Link probe procedure thereafter.
* **Copy/paste** or **click/drop** the `bin` file into the disk.

### ST-Link probe

* Download the [ST Cube Programmer](https://www.st.com/en/development-tools/stm32cubeprog.html) software.
* Launch the software (it might be necessary to run it as **root** or to install specific **USB rules** for the probe to be recognized).
* In the right panel, select `ST-LINK` and click `Connect`.
* Click on the `Open file` tab and select the `hex` file to flash.
* Click on the `Download` button.
* Perform a **memory check** with the `Verify` button located under the `Download` button menu.
* If the operation completed successfully, click on `Disconnect` in the right panel.

### Segger J-Link probe

* Download the [Segger J-Link](https://www.segger.com/downloads/jlink/) software.
* Launch the `JFlashLite` tool.
* Set target device to **STM32L011F4**, target interface to **SWD**, speed to **4000kHz** and click `OK`.
* Open the `hex` file to flash.
* Click on the `Program Device` button.
