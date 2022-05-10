################################################################################
function(display_option)
    message ("      -> IS_LOGGER_ENABLE       : ${IS_LOGGER_ENABLE}")
    message ("      -> USE_LOGGER_SAMPLE      : ${USE_LOGGER_SAMPLE}")
    message ("      -> USE_TIMEEVT_SAMPLE     : ${USE_TIMEEVT_SAMPLE}")
    message ("      -> USE_PARAMETERS_SAMPLE  : ${USE_PARAMETERS_SAMPLE}")
    message ("      -> USE_CRYPTO_SAMPLE      : ${USE_CRYPTO_SAMPLE}")
    message ("      -> USE_CRC_SAMPLE         : ${USE_CRC_SAMPLE}")
    message ("      -> USE_REEDSOLOMON_SAMPLE : ${USE_REEDSOLOMON_SAMPLE}")
    message ("      -> USE_IMGSTORAGE_SAMPLE  : ${USE_IMGSTORAGE_SAMPLE}")
endfunction(display_option)

################################################################################
include(CMakeDependentOption)

# OpenWize samples options
option(USE_FREERTOS_SAMPLE "Enable the use of FreeRTOS sample provided by OpenWize." ON)
option(USE_CRYPTO_SAMPLE "Enable the use of Crypto sample provided by OpenWize." ON)
option(USE_CRC_SAMPLE "Enable the use of CRC_sw sample provided by OpenWize." ON)
option(USE_REEDSOLOMON_SAMPLE "Enable the use of ReedSolomon sample provided by OpenWize." ON)
option(USE_PARAMETERS_SAMPLE "Enable the use of Parameters sample provided by OpenWize." ON)
option(USE_IMGSTORAGE_SAMPLE "Enable the use of ImgStorage sample provided by OpenWize." ON)
option(USE_TIMEEVT_SAMPLE "Enable the use of TimeEvt sample provided by OpenWize." ON)

option(IS_LOGGER_ENABLE "Enable the Logger in OpenWize." ON)
option(USE_LOGGER_SAMPLE "Enable the use of Logger sample provided by OpenWize." ON)

cmake_dependent_option(USE_LOGGER_SAMPLE "Enable the use of Logger sample provided by OpenWize." ON "IS_LOGGER_ENABLE" OFF)


display_option()

################################################################################
