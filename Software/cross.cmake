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

#-----------------------------
# Compiler/Toolchain options
#-----------------------------
# Important: TOOLCHAIN_PREFIX must be configured as it's system and user dependant
# MAC OSX and Linux Example -> set(TOOLCHAIN_PREFIX "/Users/USER/opt/gcc-arm-none-eabi-9-2020-q2-update/")
# Ubuntu install from 'sudo apt install gcc-arm-none-eabi' -> set(TOOLCHAIN_PREFIX "/usr")
# Windows Example -> set(TOOLCHAIN_PREFIX "C:/Program Files (x86)/GNU Tools ARM Embedded/9 2020-q2-update/")
# Docker Example -> set(TOOLCHAIN_PREFIX "/toolchain/gcc-arm-none-eabi-9-2020-q2-update/")

if(NOT TOOLCHAIN_PREFIX)
    message(FATAL_ERROR "[ERRR] TOOLCHAIN_PREFIX not specified, please update the with compiler toolchain location")
endif()

set(CROSS_TOOLCHAIN arm-none-eabi-)
set(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)
set(TOOLCHAIN_INC_DIR ${TOOLCHAIN_PREFIX}/${TOOLCHAIN}/include)
set(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_PREFIX}/${TOOLCHAIN}/lib)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(SEMIHOSTING 0)

if (WIN32)
set (TOOLCHAIN_EXT ".exe")
endif (WIN32)

set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}gcc${TOOLCHAIN_EXT} CACHE INTERNAL "C Compiler")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}g++${TOOLCHAIN_EXT} CACHE INTERNAL "C++ Compiler")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}gcc${TOOLCHAIN_EXT} CACHE INTERNAL "ASM Compiler")
set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}objcopy${TOOLCHAIN_EXT} CACHE INTERNAL "Objcopy tool")
set(CMAKE_SIZE_UTIL ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}size${TOOLCHAIN_EXT} CACHE INTERNAL "Size tool")
set(CMAKE_C_GDB ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}gdb-py${TOOLCHAIN_EXT} CACHE INTERNAL "Debugger")
SET(CMAKE_AR ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}gcc-ar${TOOLCHAIN_EXT} CACHE INTERNAL "Assembler")
SET(CMAKE_RANLIB ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}gcc-ranlib${TOOLCHAIN_EXT} CACHE INTERNAL "Ranlib")

#---------------------------------------------------------------------------------------
# Creates output in hex format
#---------------------------------------------------------------------------------------
function(create_hex_output TARGET TARGET_PATH)
    add_custom_target(${TARGET}.hex ALL DEPENDS ${TARGET}.elf COMMAND ${CMAKE_OBJCOPY} -Oihex ${TARGET_PATH}/${TARGET}.elf ${TARGET_PATH}/${TARGET}.hex)
endfunction()

#---------------------------------------------------------------------------------------
# Creates output in binary format
#---------------------------------------------------------------------------------------
function(create_bin_output TARGET TARGET_PATH)
    add_custom_target(${TARGET}.bin ALL DEPENDS ${TARGET}.elf COMMAND ${CMAKE_OBJCOPY} -Obinary ${TARGET_PATH}/${TARGET}.elf ${TARGET_PATH}/${TARGET}.bin)
endfunction()
