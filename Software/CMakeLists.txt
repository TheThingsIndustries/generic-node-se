#  Copyright © 2020 The Things Industries B.V.
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#      http://www.apache.org/licenses/LICENSE-2.0
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

cmake_minimum_required(VERSION 3.11.1)

#if(NOT CMAKE_TOOLCHAIN_FILE)
#set(CMAKE_TOOLCHAIN_FILE ../cross.cmake)
#message(WARNING "[WARN] CMAKE_TOOLCHAIN_FILE not specified: Using ${CMAKE_TOOLCHAIN_FILE} by default")
#endif()
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    message(WARNING "[WARN] CMAKE_BUILD_TYPE not specified: Using ${CMAKE_BUILD_TYPE} by default")
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

#################### CONFIGURABLE SECTION ###########################

#-------------------
# Project Setup
#-------------------
enable_language(C ASM)
# Project name
project(main)
# The project version number.
set(VERSION_MAJOR   0   CACHE STRING "Project major version number.")
set(VERSION_MINOR   0   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   1   CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

#------------------
# Modules Setup
#------------------
# set(MODULES_USED
#     simple_module
#     )
#-------------------
# MCU Setup
#-------------------
set(MCU STM32WLE5xx)
set(MCU_DIR target/STM32WLxx/)
set(MCU_SPEC  cortex-m4)

# Dont need to change this if MCU is defined correctly
# Below commands are for fetching the path to the system and startup files
# If execute_process is not able to locate the files properly
# then directly set the STARTUP_FILE and SYSTEM_FILE to the required files
# execute_process(COMMAND
#     bash "-c" "echo -n startup_${MCU}.s | awk '{print tolower($0)}'"
#     OUTPUT_VARIABLE STARTUP_FILE_NAME
#     OUTPUT_STRIP_TRAILING_WHITESPACE
#     )
set(STARTUP_FILE ${MCU_DIR}/Source/Templates/gcc/startup_stm32wle5xx.s)
set(SYSTEM_FILE  ${MCU_DIR}/Source/Templates/system_stm32wlxx.c)

# Set the Highspeed external clock value (HSE) in MHz
# set(HSE_VAL 32000000)

# Define the linker script location
set(LINKER_SCRIPT linker.ld)

#-------------------
# HAL Setup
#-------------------
# Select 1 if STM32 HAL library is to be used. This will add -DUSE_HAL_DRIVER=1 to the CFLAGS
# If enabled then set the correct path of the HAL Driver folder
#set(USE_HAL = 1
#ifeq (1,$(USE_HAL))
set(HAL_DIR components/STM32WLxx_HAL_Driver)
#endif

#-------------------
# LoRaWAN Setup
#-------------------
set(LORAWAN_DIR components/LoRaWAN)
#endif

#-------------------
# RTOS Setup
#-------------------
# Path to FreeRTOS Kernel
# set(RTOS_DIR components/FreeRTOS-Kernel)
# Modify this to the path where your micrcontroller specific port is
# set(RTOS_DIR_MCU ${RTOS_DIR}/portable/GCC/ARM_CM4F) # For cortex-m4 microcontroller
# set(RTOS_HEAP    ${RTOS_DIR}/portable/MemMang/heap_4.c) # Select which heap implementation to use

#-------------------
# CMSIS Setup
#-------------------
# Set the path to the CMSIS folder
set(CMSIS_DIR components/CMSIS)

#################### ADVANCED SECTION ###########################

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

if(CMAKE_CROSSCOMPILING STREQUAL "1")
    # Control specific options if crosscompiling

    message(STATUS "[INFO] Cross compiling for ${MCU}")
    message(STATUS "[INFO] Startup file used is ${STARTUP_FILE}")

    # Control ARM Semihosting support
    if(NOT SEMIHOSTING)
        set(SEMIHOSTING 1)
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

    if("${SEMIHOSTING}" STREQUAL "1")
        string(APPEND CMAKE_EXE_LINKER_FLAGS "--specs=rdimon.specs -lc -lrdimon ")
    else()
        string(APPEND CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs ")
    endif()

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

# add_subdirectory(modules)

# Actual build which will go onto the board
if(TARGET_GROUP STREQUAL production)

    ######################################################################
    # @Target: ${PROJECT_NAME}.elf
    # @Brief: Build the actual elf file of the project
    ######################################################################

    #-------------------
    # HAL library
    #-------------------
    file(GLOB HAL_SRC
        "${HAL_DIR}/Src/*.c"
        )
    add_library(hal STATIC
        ${HAL_SRC}
        )
    target_include_directories(hal
        PUBLIC
        ${HAL_DIR}/Inc
        ${MCU_DIR}/Include
        ${CMSIS_DIR}/Core/Include
        PRIVATE
        ${PROJECT_SOURCE_DIR}/target
        )
    target_compile_definitions(hal
        PUBLIC
        ${MCU}
        # HSE_VALUE=${HSE_VAL}
        )
    #-------------------
    # LoRaWAN library
    #-------------------
    # file(GLOB LORAWAN_SRC
    #     "${LORAWAN_DIR}/Crypto/*.c"
    #     "${LORAWAN_DIR}/Mac/*.c"
    #     "${LORAWAN_DIR}/Mac/region/*.c"
    #     "${LORAWAN_DIR}/Patterns/Basic/*.c"
    #     "${LORAWAN_DIR}/Utilities/*.c"
    #     "${LORAWAN_DIR}/SubGHz_Phy/stm32_radio_driver/*.c"
    #     )
    # add_library(lorawan STATIC
    #     ${LORAWAN_SRC}
    #     )
    # target_include_directories(lorawan
    #     PUBLIC
    #     ${HAL_DIR}/Inc
    #     ${MCU_DIR}/Include
    #     ${CMSIS_DIR}/Core/Include
    #     ${LORAWAN_DIR}/Crypto
    #     ${LORAWAN_DIR}/Mac
    #     ${LORAWAN_DIR}/Mac/region
    #     ${LORAWAN_DIR}/Patterns/Basic
    #     ${LORAWAN_DIR}/Utilities
    #     ${LORAWAN_DIR}/SubGHz_Phy
    #     ${LORAWAN_DIR}/SubGHz_Phy/stm32_radio_driver
    #     PRIVATE
    #     ${PROJECT_SOURCE_DIR}/target
    #     ${PROJECT_SOURCE_DIR}/app/LoRaWAN
    #     )
    # target_compile_definitions(lorawan
    #     PUBLIC
    #     ${MCU}
    #     # HSE_VALUE=${HSE_VAL}
    #     )

    #-------------------
    # FreeRTOS library
    #-------------------
    # string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--undefined=vTaskSwitchContext ")
    # file(GLOB RTOS_SRC
    #     "${RTOS_DIR}/*.c"
    #     "${RTOS_DIR_MCU}/*.c"
    #     ${RTOS_HEAP}
    #     )
    # add_library(rtos STATIC
    #     ${RTOS_SRC}
    #     )
    # target_include_directories(rtos
    #     PUBLIC
    #     ${RTOS_DIR}/include
    #     ${RTOS_DIR_MCU}
    #     PRIVATE
    #     ${PROJECT_SOURCE_DIR}/include
    #     )

    #-------------------
    # Main elf
    #-------------------
    file(GLOB MAIN_SRC
        "app/basic/*.c"
        "target/*.c"
        "lib/BSP/*.c"
        )
    set(SOURCES
        ${MAIN_SRC}
        ${STARTUP_FILE}
        ${SYSTEM_FILE}
        )
    add_executable(${PROJECT_NAME}.elf
        ${SOURCES}
        )
    target_include_directories(${PROJECT_NAME}.elf
        PUBLIC
        ${PROJECT_SOURCE_DIR}/target
        ${PROJECT_SOURCE_DIR}/app/basic
        ${PROJECT_SOURCE_DIR}/lib/BSP
        )
    target_link_libraries(${PROJECT_NAME}.elf
        PUBLIC
        # rtos
        hal
        # lorawan
        # ${MODULES_USED}
        )
    target_compile_definitions(${PROJECT_NAME}.elf
        PUBLIC
        ${MCU}
        # HSE_VALUE=${HSE_VAL}
        SEMIHOSTING=${SEMIHOSTING}
        )

        # Create output in hex and binary format
        create_bin_output(${PROJECT_NAME})
        create_hex_output(${PROJECT_NAME})

    ######################################################################
    # @Target: flash
    # @Brief: flash the release/debug elf using gdb
    ######################################################################
    add_custom_target(flash DEPENDS ${PROJECT_NAME}.elf)
    add_custom_command(TARGET flash
        #COMMAND bash "-c" "pgrep -x \"openocd\" || (echo \"Please start openocd\" && exit -1)"
        COMMAND echo "Starting GDB client and loading ${PROJECT_NAME}.elf dashboard to $(GDB_TTY)"
        COMMAND ${CMAKE_C_GDB} ${PROJECT_NAME}.elf
        -ex "target extended :3333"
        -ex "dashboard -output $(GDB_TTY)"
        -ex "load ${PROJECT_NAME}.elf"
        -ex "monitor arm semihosting enable"
        )

    ######################################################################
    # @Target: doc
    # @Brief: Generate doxygen documentation
    ######################################################################
    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # exclude folders while making documentation
        # should be separated by spaces
        set(DOXYGEN_EXCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components/* ${CMAKE_CURRENT_SOURCE_DIR}/build/* ${CMAKE_CURRENT_SOURCE_DIR}/${MCU_DIR}*")

        # set mainpage in doxygen
        set(DOXYGEN_MAIN_PAGE "README.md")

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target(docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)

    # Add additional files to the make clean
    set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
        "${PROJECT_NAME}.map"
        "${CMAKE_CURRENT_BINARY_DIR}/html"
        "${CMAKE_CURRENT_BINARY_DIR}/latex"
        "${PROJECT_NAME}.bin"
        "${PROJECT_NAME}.hex"
        )

elseif(TARGET_GROUP STREQUAL test)

    add_library(unity STATIC
        components/unity/unity.c
        )

    target_include_directories(unity PUBLIC
        components/unity/
        )

    enable_testing()
    include(CTest)
    # add_subdirectory(test)

    set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
    include(CodeCoverage)
    setup_target_for_coverage_lcov(
        NAME coverage
        EXCLUDE "/usr/include/x86_64-linux-gnu/*" "components/unity/*"
        EXECUTABLE ctest -V
        DEPENDENCIES simple_module
        )
else()
    message(FATAL_ERROR "Given TARGET_GROUP unknown")
endif()

# Unset all cache
unset(SEMIHOSTING)
unset(CMAKE_TOOLCHAIN_FILE)
unset(CMAKE_BUILD_TYPE)
