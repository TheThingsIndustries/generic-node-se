#-------------------
# HAL library
#-------------------
file(GLOB HAL_SRC
    "${PROJECT_SOURCE_DIR}/${HAL_DIR}/Src/*.c"
    )
add_library(hal STATIC
    ${HAL_SRC}
    )
target_include_directories(hal
    PUBLIC
    ${PROJECT_SOURCE_DIR}/${HAL_DIR}/Inc
    ${PROJECT_SOURCE_DIR}/${MCU_DIR}/Include
    ${PROJECT_SOURCE_DIR}/${CMSIS_DIR}/Core/Include
    PRIVATE
    ${PROJECT_SOURCE_DIR}/target
    )
target_compile_definitions(hal
    PUBLIC
    ${MCU}
    )
