#
# CMakeLists.txt
#
#  Created on: 17 may. 2026
#      Author: Ludo
#

# Minimum CMake version.
cmake_minimum_required(VERSION 3.23)

# Project creation.
project(t82-accelero)
add_executable(${PROJECT_NAME})

# Use object build mode in all sub-directories.
set(BUILD_MODE "OBJECT")

# Macro to convert options to variables.
macro(add_compilation_flag FLAG_NAME FLAG_DESCRIPTION FLAG_DEFAULT_VALUE)
    option(FLAG_NAME FLAG_DESCRIPTION FLAG_DEFAULT_VALUE)
    set(${FLAG_NAME} ${FLAG_DEFAULT_VALUE} CACHE STRING ${FLAG_DESCRIPTION})
    list(APPEND COMPILATION_FLAGS_LIST ${FLAG_NAME})
endmacro()

# Hardware compilation flags.
set(T82_ACCELERO_HW_VERSION "NONE" CACHE STRING "Hardware version")

# Hardware specific settings.
# T82_ACCELERO HW1.0.
if(T82_ACCELERO_HW_VERSION STREQUAL "HW1_0")   
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -O0")
    set(T82_ACCELERO_MCU "stm32l0xx")
    set(PROJECT_LINKER_SCRIPT "stm32l011x4xx.ld")
# Unknown version.
else()
    message(FATAL_ERROR "Invalid hardware version.")
endif()
set(PROJECT_LINKER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/drivers/device/${T82_ACCELERO_MCU}-device/linker")

# Add hardware compilation flags.
add_compile_definitions(
    __T82_ACCELERO_FLAGS_H__
    ${T82_ACCELERO_HW_VERSION}
)

# Add software compilation flags.
foreach(FLAG ${COMPILATION_FLAGS_LIST})
    # Macros only defined.
    if(${FLAG} STREQUAL ON)
        add_compile_definitions(${FLAG})
    endif()
    # Macros with value.
    if((NOT ${FLAG} STREQUAL OFF) AND (NOT ${FLAG} STREQUAL ON))
        set(${FLAG} "(${${FLAG}})")
        add_compile_definitions(${FLAG}=${${FLAG}}) 
    endif()
endforeach()

# Add project sources files.
target_sources(${PROJECT_NAME}
    PRIVATE
        drivers/peripherals/src/mcu_mapping.c
        drivers/components/src/fxls89xxxx_hw.c
        application/src/main.c
)

# Project include folders.
include_directories(${PROJECT_NAME}
    PRIVATE
        drivers/device/inc
        drivers/registers/inc
        drivers/registers/${T82_ACCELERO_MCU}-registers/inc
        drivers/peripherals/inc
        drivers/peripherals/${T82_ACCELERO_MCU}-drivers/inc
        drivers/utils/inc
        drivers/utils/embedded-utils/inc
        drivers/components/inc
        application/inc
)

# Add submodules sources files.
add_subdirectory(drivers/device/${T82_ACCELERO_MCU}-device EXCLUDE_FROM_ALL)
add_subdirectory(drivers/peripherals/${T82_ACCELERO_MCU}-drivers EXCLUDE_FROM_ALL)
add_subdirectory(drivers/components/fxls89xxxx-driver EXCLUDE_FROM_ALL)
add_subdirectory(drivers/utils/embedded-utils EXCLUDE_FROM_ALL)

# Link libraries.
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${T82_ACCELERO_MCU}-device
        ${T82_ACCELERO_MCU}-drivers
        fxls89xxxx-driver
        embedded-utils
        c_nano
        nosys
        gcc
)

# Linker and artifact.
include(script/cmake-arm-none-eabi/linker.cmake)
include(script/cmake-arm-none-eabi/artifact.cmake)
