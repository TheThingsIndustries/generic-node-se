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

#-----------------------------
# Compiler and generic options
#-----------------------------
set(CROSS_TOOLCHAIN arm-none-eabi-)
set(TOOLCHAIN_PREFIX "/Users/ahmedelsalahy/opt/gcc-arm-none-eabi-6-2017-q2-update/")
set(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)
set(TOOLCHAIN_INC_DIR ${TOOLCHAIN_PREFIX}/${TOOLCHAIN}/include)
set(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_PREFIX}/${TOOLCHAIN}/lib)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_CROSSCOMPILING 1)

set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}gcc${TOOLCHAIN_EXT} CACHE INTERNAL "C Compiler")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}g++${TOOLCHAIN_EXT} CACHE INTERNAL "C++ Compiler")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}gcc${TOOLCHAIN_EXT} CACHE INTERNAL "ASM Compiler")
set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}objcopy CACHE INTERNAL "Objcopy tool")
set(CMAKE_SIZE_UTIL ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}size CACHE INTERNAL "Size tool")
set(CMAKE_C_GDB ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}gdb-py CACHE INTERNAL "Debugger")
SET(CMAKE_AR ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}gcc-ar CACHE INTERNAL "Assembler")
SET(CMAKE_RANLIB ${TOOLCHAIN_BIN_DIR}/${CROSS_TOOLCHAIN}gcc-ranlib CACHE INTERNAL "Ranlib")

#---------------------------------------------------------------------------------------
# Creates output in hex format
#---------------------------------------------------------------------------------------
function(create_hex_output TARGET)
    add_custom_target(${TARGET}.hex ALL DEPENDS ${TARGET}.elf COMMAND ${CMAKE_OBJCOPY} -Oihex ${TARGET}.elf ${TARGET}.hex)
endfunction()

#---------------------------------------------------------------------------------------
# Creates output in binary format
#---------------------------------------------------------------------------------------
function(create_bin_output TARGET)
    add_custom_target(${TARGET}.bin ALL DEPENDS ${TARGET}.elf COMMAND ${CMAKE_OBJCOPY} -Obinary ${TARGET}.elf ${TARGET}.bin)
endfunction()

#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
