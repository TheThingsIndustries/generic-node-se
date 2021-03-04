#  Copyright © 2021 The Things Industries B.V.
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#      http://www.apache.org/licenses/LICENSE-2.0
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

#-------------------
# Cmake Setup
#-------------------
cmake_minimum_required(VERSION 3.11.1)

if(NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "[ERRR] CMAKE_TOOLCHAIN_FILE not specified")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    message(WARNING "[WARN] CMAKE_BUILD_TYPE not specified: Using ${CMAKE_BUILD_TYPE} by default")
endif()

if(NOT TARGET_APP)
    set(TARGET_APP basic CACHE STRING "Choose the target app." FORCE)
    message(WARNING "[WARN] TARGET_APP not specified: Using ${TARGET_APP} app by default")
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
#-------------------
# Project Setup
#-------------------
enable_language(C ASM)
project(main)
set (APP_LIST
"basic"
"basic_lorawan"
"basic_bootloader"
"basic_freertos"
"freefall_lorawan"
"freertos_lorawan")
#-------------------
# MCU Setup
#-------------------
set(MCU STM32WL55xx)
set(MCU_DIR target)
set(MCU_SPEC  cortex-m4)
set(STARTUP_FILE ${MCU_DIR}/startup_stm32wl55xx.s)
set(SYSTEM_FILE  ${MCU_DIR}/system_stm32wlxx.c)
set(LINKER_SCRIPT_MEM ${MCU_DIR}/memory_map.ld)
# Adjust stm32wl55xx_flash.ld to app.ld or bootloader.ld when using a bootloader
set(LINKER_SCRIPT ${MCU_DIR}/stm32wl55xx_flash.ld)
#-------------------
# HAL Setup
#-------------------
set(HAL_DIR target/stm32wlxx_hal_driver)
#-------------------
# LoRaWAN Setup
#-------------------
set(LORAWAN_DIR lib/STM32WLxx_LoRaWAN)
#-------------------
# CMSIS Setup
#-------------------
# Set the path to the CMSIS folder
set(CMSIS_DIR target/CMSIS)
#-------------------
# General Flags
#-------------------
set(OBJECT_GEN_FLAGS " \
-fno-builtin \
-Wall \
-ffunction-sections -fdata-sections \
-fomit-frame-pointer \
" CACHE INTERNAL "Common flags for C/CXX/ASM/Linker")
#-------------------
# CFLAGS
#-------------------
set(CMAKE_C_FLAGS " \
" CACHE INTERNAL "C Compiler options")
#-------------------
# ASMFLAGS for cross
#-------------------
set(CMAKE_ASM_FLAGS " \
-x assembler-with-cpp \
" CACHE INTERNAL "ASM Compiler options")
#-------------------
# LFLAGS for cross
#-------------------
set(CMAKE_EXE_LINKER_FLAGS " \
-Wl,-Map=${PROJECT_NAME}.map \
-Wl,--print-memory-usage \
-Wl,--gc-sections \
" CACHE INTERNAL "Linker options")
#------------------
# Debug Flags
#------------------
set(CMAKE_C_FLAGS_DEBUG "-Og -g -gdwarf-3 -gstrict-dwarf " CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g -gdwarf-3 -gstrict-dwarf " CACHE INTERNAL "C++ Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "-Og -g -gdwarf-3 -gstrict-dwarf " CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Linker options for debug build type")
#------------------
# Release Flags
#-----------------
set(CMAKE_C_FLAGS_RELEASE "-Os -flto " CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -flto " CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto " CACHE INTERNAL "Linker options for release build type")
#------------------
# Cross Compilation Flags
#-----------------
if(CMAKE_CROSSCOMPILING STREQUAL "1")
    message(STATUS "[INFO] Cross compiling for ${MCU}")
    message(STATUS "[INFO] Startup file used is ${STARTUP_FILE}")
    # Control ARM Semihosting support
    if(NOT SEMIHOSTING)
        set(SEMIHOSTING 0)
        message(WARNING "[WARN] Semihosting support not specified: Enabling by default")
    endif()
    #-------------------
    # General Flags for cross
    #-------------------
    string(APPEND OBJECT_GEN_FLAGS " \
    -mcpu=${MCU_SPEC} \
    -mthumb \
    -mthumb-interwork \
    -mabi=aapcs \
    ${FLOAT_SPEC} \
    ")
    #-------------------
    # CFLAGS for cross
    #-------------------
    string(APPEND CMAKE_C_FLAGS " \
    ${OBJECT_GEN_FLAGS} \
    ")
    #-------------------
    # ASMFLAGS for cross
    #-------------------
    string(APPEND CMAKE_ASM_FLAGS " \
    ${OBJECT_GEN_FLAGS} \
    ")
    #-------------------
    # LFLAGS for cross
    #-------------------
    string(APPEND CMAKE_EXE_LINKER_FLAGS " \
    ${OBJECT_GEN_FLAGS} \
    ")
    find_file(LINKER_SCRIPT_PATH
        NAMES "${LINKER_SCRIPT}"
        PATHS
        ${CMAKE_CURRENT_LIST_DIR}
        )
    if(DEFINED LINKER_SCRIPT_PATH)
        message(STATUS "[INFO] Using linker file at ${LINKER_SCRIPT_PATH}")
        string(APPEND CMAKE_EXE_LINKER_FLAGS "-T${LINKER_SCRIPT_PATH} ")
    else()
        message(FATAL_ERROR "[ERRR] Could not find linker script ${LINKER_SCRIPT}")
    endif()

    find_file(LINKER_SCRIPT_MEM_PATH
        NAMES "${LINKER_SCRIPT_MEM}"
        PATHS
        ${CMAKE_CURRENT_LIST_DIR}
        )
    if(DEFINED LINKER_SCRIPT_MEM_PATH)
        message(STATUS "[INFO] Using memory map linker at ${LINKER_SCRIPT_MEM_PATH}")
        string(APPEND CMAKE_EXE_LINKER_FLAGS "-L${PROJECT_SOURCE_DIR}/${MCU_DIR} ")
    endif()

    if("${SEMIHOSTING}" STREQUAL "1")
        string(APPEND CMAKE_EXE_LINKER_FLAGS "--specs=rdimon.specs -lc -lrdimon ")
    else()
        string(APPEND CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs ")
    endif()
#------------------
# Native Compilation Flags
#-----------------
else()
    # Flags and options to set while compiling natively
    message(STATUS "[INFO] Compiling natively")
    #-------------------
    # General Flags for native
    #-------------------
    string(APPEND OBJECT_GEN_FLAGS " \
    --coverage \
    ")
    #-------------------
    # CFLAGS for native
    #-------------------
    string(APPEND CMAKE_C_FLAGS " \
    ${OBJECT_GEN_FLAGS} \
    ")
    #-------------------
    # ASMFLAGS for native
    #-------------------
    string(APPEND CMAKE_ASM_FLAGS " \
    ${OBJECT_GEN_FLAGS} \
    ")
    #-------------------
    # LFLAGS for native
    #-------------------
    string(APPEND CMAKE_EXE_LINKER_FLAGS " \
    ${OBJECT_GEN_FLAGS} \
    ")
endif()

# Buidling Specified Application
if(TARGET_APP IN_LIST APP_LIST)
    add_subdirectory(target)
    add_subdirectory(app/${TARGET_APP})
else()
    message(FATAL_ERROR "Given TARGET_APP unknown")
endif()

# Unset all cache
unset(SEMIHOSTING)
unset(CMAKE_TOOLCHAIN_FILE)
unset(CMAKE_BUILD_TYPE)
unset(TARGET_APP)
