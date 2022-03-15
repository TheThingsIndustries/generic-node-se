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
cmake_minimum_required(VERSION 3.16.1)
if (NOT HOSTMODE)
if(NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "[ERRR] CMAKE_TOOLCHAIN_FILE not specified")
endif()
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

project(${TARGET_APP})

if (NOT APP_LIST)
    set (APP_LIST
    "app_template"
    "basic"
    "basic_lorawan"
    "basic_bootloader"
    "basic_freertos"
    "freefall_lorawan"
    "freertos_lorawan"
    "secure_element_lorawan"
    "basic_azurertos"
    "sensors_lorawan"
    "basic_fuota")
endif()
#-------------------
# MCU Setup
#-------------------
if (NOT HOSTMODE)

set(MCU STM32WL55xx)
set(MCU_DIR target)
set(MCU_SPEC  cortex-m4)
set(STARTUP_FILE ${MCU_DIR}/startup_stm32wl55xx.s)

if (NOT LINKER_SCRIPT_MEM)
	set(LINKER_SCRIPT_MEM ${MCU_DIR}/memory_map.ld)
endif()

# Adjust stm32wl55xx_flash.ld to app.ld or bootloader.ld when using a bootloader
if (NOT LINKER_SCRIPT)
	set(LINKER_SCRIPT ${MCU_DIR}/stm32wl55xx_flash.ld)
endif()
set(SYSTEM_FILE  ${MCU_DIR}/system_stm32wlxx.c)

else()
# Host mode

set(MCU host)

if (NOT MCU_DIR)
	set(MCU_DIR hostmode/target)
endif()

if (NOT APP_BUILD_DIR)
    set (APP_BUILD_DIR app/${TARGET_APP})
else()
    message(WARNING "[WARN] Using custom target app ${APP_BUILD_DIR}")
endif()



set(SYSTEM_FILE  ${MCU_DIR}/system_stm32wlxx.c)

set(LORAWAN_DIR ${MCU_DIR}/../lib/STM32WLxx_LoRaWAN)

endif()


#-------------------
# HAL Setup
#-------------------
if (NOT HAL_DIR)
	set(HAL_DIR ${MCU_DIR}/stm32wlxx_hal_driver)
endif()
#-------------------
# LoRaWAN Setup
#-------------------
if (NOT LORAWAN_DIR)
	set(LORAWAN_DIR lib/STM32WLxx_LoRaWAN)
endif()
#-------------------
# CMSIS Setup
#-------------------
# Set the path to the CMSIS folder
if (NOT CMSIS_DIR)
	set(CMSIS_DIR ${MCU_DIR}/CMSIS)
endif()


#-------------------
# General Flags
#-------------------
if (NOT OBJECT_GEN_FLAGS)
set(OBJECT_GEN_FLAGS " \
-fno-builtin \
-Wall \
-ffunction-sections -fdata-sections \
-fomit-frame-pointer \
" CACHE INTERNAL "Common flags for C/CXX/ASM/Linker")
endif()
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
set(CMAKE_C_FLAGS_RELEASE "-Os -flto -DHAVE_LTO" CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -flto -DHAVE_LTO" CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "-DHAVE_LTO" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto " CACHE INTERNAL "Linker options for release build type")
#------------------
# Cross Compilation Flags
#-----------------
message(STATUS "[INFO] Cross compiling for ${MCU}")
message(STATUS "[INFO] Startup file used is ${STARTUP_FILE}")
# Control ARM Semihosting support
if(NOT SEMIHOSTING)
set(SEMIHOSTING 0)
message(WARNING "[WARN] Semihosting support not specified: Disabling by default")
endif()
#-------------------
# General Flags for cross
#-------------------
if (NOT HOSTMODE)
string(APPEND OBJECT_GEN_FLAGS " \
-mcpu=${MCU_SPEC} \
-mthumb \
-mthumb-interwork \
-mabi=aapcs \
-fno-math-errno \
${FLOAT_SPEC} \
")
endif()
#-------------------
# CFLAGS for cross
#-------------------
string(APPEND CMAKE_C_FLAGS " \
${OBJECT_GEN_FLAGS} \
")
#-------------------
# CXXFLAGS for cross
#-------------------
string(APPEND CMAKE_CXX_FLAGS " \
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

if (NOT HOSTMODE)

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

endif()

if("${SEMIHOSTING}" STREQUAL "1")
string(APPEND CMAKE_EXE_LINKER_FLAGS "--specs=rdimon.specs -lc -lrdimon ")
else()
if (NOT HOSTMODE)
 string(APPEND CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs ")
endif()
endif()

if (HOSTMODE)
    set(EXTRA_LIBRARIES pthread intrhandler)
endif()


#-------------------
# Build target application
#-------------------
if(TARGET_APP IN_LIST APP_LIST)
    add_subdirectory(${MCU_DIR})
    add_subdirectory(${APP_BUILD_DIR})
    target_compile_definitions(${PROJECT_NAME}.elf
    PUBLIC
    ${MCU}
    SEMIHOSTING=${SEMIHOSTING}
    -DGNSE_APP_NAME="${TARGET_APP}"
    )
    if (NOT HOSTMODE)
    # Create output in hex and binary format
    create_hex_output(${PROJECT_NAME} ${APP_BUILD_DIR})
    create_bin_output(${PROJECT_NAME} ${APP_BUILD_DIR})
    # Add additional files to the make clean
    set_property(DIRECTORY PROPERTY ADDITIONAL_CLEAN_FILES
    "${APP_BUILD_DIR}/${PROJECT_NAME}.map"
    "${APP_BUILD_DIR}/${PROJECT_NAME}.bin"
    "${APP_BUILD_DIR}/${PROJECT_NAME}.hex"
    )
    endif()
else()
    message(FATAL_ERROR "Given TARGET_APP unknown")
endif()

# Unset all cache
unset(SEMIHOSTING)
unset(CMAKE_TOOLCHAIN_FILE)
unset(CMAKE_BUILD_TYPE)
unset(TARGET_APP)
unset(APP_BUILD_DIR)
