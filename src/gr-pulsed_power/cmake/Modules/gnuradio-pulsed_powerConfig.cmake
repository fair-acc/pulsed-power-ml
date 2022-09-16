find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_PULSED_POWER gnuradio-pulsed_power)

FIND_PATH(
    GR_PULSED_POWER_INCLUDE_DIRS
    NAMES gnuradio/pulsed_power/api.h
    HINTS $ENV{PULSED_POWER_DIR}/include
        ${PC_PULSED_POWER_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_PULSED_POWER_LIBRARIES
    NAMES gnuradio-pulsed_power
    HINTS $ENV{PULSED_POWER_DIR}/lib
        ${PC_PULSED_POWER_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-pulsed_powerTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_PULSED_POWER DEFAULT_MSG GR_PULSED_POWER_LIBRARIES GR_PULSED_POWER_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_PULSED_POWER_LIBRARIES GR_PULSED_POWER_INCLUDE_DIRS)
