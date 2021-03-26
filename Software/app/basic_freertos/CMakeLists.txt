#-------------------
# Main elf
#-------------------
file(GLOB MAIN_SRC
    "${PROJECT_SOURCE_DIR}/app/basic_freertos/*.c"
    "${PROJECT_SOURCE_DIR}/target/*.c"
    "${PROJECT_SOURCE_DIR}/target/FreeRTOS_CMSIS/*.c"
    "${PROJECT_SOURCE_DIR}/lib/GNSE_BSP/*.c"
    "${PROJECT_SOURCE_DIR}/lib/Utilities/*.c"
    "${PROJECT_SOURCE_DIR}/lib/GNSE_TRACER/adv_tracer/*.c"
    "${PROJECT_SOURCE_DIR}/lib/GNSE_TRACER/tiny_printf/*.c"
    "${PROJECT_SOURCE_DIR}/lib/Utilities/freertos/*.c"
    "${PROJECT_SOURCE_DIR}/lib/SHTC3/*.c"
    "${PROJECT_SOURCE_DIR}/lib/MX25R1635/*.c"
    "${PROJECT_SOURCE_DIR}/lib/FreeRTOS-Kernel/*.c"
    #GCC ARM_CM3 seems functional with ARM CM4
    "${PROJECT_SOURCE_DIR}/lib/FreeRTOS-Kernel/portable/GCC/ARM_CM3/*.c"
    "${PROJECT_SOURCE_DIR}/lib/FreeRTOS-Kernel/portable/MemMang/heap_4.c"
    )
set(SOURCES
    ${MAIN_SRC}
    ${PROJECT_SOURCE_DIR}/${STARTUP_FILE}
    ${PROJECT_SOURCE_DIR}/${SYSTEM_FILE}
    )
add_executable(${PROJECT_NAME}.elf
    ${SOURCES}
    )
target_include_directories(${PROJECT_NAME}.elf
    PUBLIC
    ${PROJECT_SOURCE_DIR}/target
    ${PROJECT_SOURCE_DIR}/target/FreeRTOS_CMSIS
    ${PROJECT_SOURCE_DIR}/app
    ${PROJECT_SOURCE_DIR}/app/basic_freertos
    ${PROJECT_SOURCE_DIR}/app/basic_freertos/conf
    ${PROJECT_SOURCE_DIR}/lib/GNSE_BSP
    ${PROJECT_SOURCE_DIR}/lib/Utilities
    ${PROJECT_SOURCE_DIR}/lib/GNSE_TRACER
    ${PROJECT_SOURCE_DIR}/lib/GNSE_TRACER/adv_tracer
    ${PROJECT_SOURCE_DIR}/lib/GNSE_TRACER/tiny_printf
    ${PROJECT_SOURCE_DIR}/lib/Utilities/freertos
    ${PROJECT_SOURCE_DIR}/lib/SHTC3
    ${PROJECT_SOURCE_DIR}/lib/MX25R1635
    ${PROJECT_SOURCE_DIR}/lib/LIS2DH12
    ${PROJECT_SOURCE_DIR}/lib/BUZZER
    ${PROJECT_SOURCE_DIR}/lib/ATECC608A-TNGLORA
    ${PROJECT_SOURCE_DIR}/lib/FreeRTOS-Kernel/include
    ${PROJECT_SOURCE_DIR}/lib/FreeRTOS-Kernel/portable/GCC/ARM_CM3
    )
target_link_libraries(${PROJECT_NAME}.elf
    PUBLIC
    hal
    )
target_compile_definitions(${PROJECT_NAME}.elf
    PUBLIC
    ${MCU}
    SEMIHOSTING=${SEMIHOSTING}
    )
    # Create output in hex and binary format
    create_bin_output(${PROJECT_NAME})
    create_hex_output(${PROJECT_NAME})
    # Add additional files to the make clean
    set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    "${PROJECT_NAME}.map"
    "${PROJECT_NAME}.bin"
    "${PROJECT_NAME}.hex"
    )
