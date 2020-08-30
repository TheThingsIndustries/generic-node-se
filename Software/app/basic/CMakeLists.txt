#-------------------
# Main elf
#-------------------
file(GLOB MAIN_SRC
    "${PROJECT_SOURCE_DIR}/app/basic/*.c"
    "${PROJECT_SOURCE_DIR}/target/*.c"
    "${PROJECT_SOURCE_DIR}/lib/STNODE_BSP/*.c"
    "${PROJECT_SOURCE_DIR}/lib/Utilities/*.c"
    "${PROJECT_SOURCE_DIR}/lib/SHTC3/*.c"
    "${PROJECT_SOURCE_DIR}/lib/MX25R1635/*.c"
    "${PROJECT_SOURCE_DIR}/lib/LIS2DH12/*.c"
    "${PROJECT_SOURCE_DIR}/lib/BUZZER/*.c"
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
    ${PROJECT_SOURCE_DIR}/app
    ${PROJECT_SOURCE_DIR}/app/basic
    ${PROJECT_SOURCE_DIR}/lib/STNODE_BSP
    ${PROJECT_SOURCE_DIR}/lib/Utilities
    ${PROJECT_SOURCE_DIR}/lib/SHTC3
    ${PROJECT_SOURCE_DIR}/lib/MX25R1635
    ${PROJECT_SOURCE_DIR}/lib/LIS2DH12
    ${PROJECT_SOURCE_DIR}/lib/BUZZER
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
