#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "pulsed_power_daq_python" for configuration "Release"
set_property(TARGET pulsed_power_daq_python APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(pulsed_power_daq_python PROPERTIES
  IMPORTED_COMMON_LANGUAGE_RUNTIME_RELEASE ""
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/python3.8/dist-packages/pulsed_power_daq/pulsed_power_daq_python.cpython-38-x86_64-linux-gnu.so"
  IMPORTED_NO_SONAME_RELEASE "TRUE"
  )

list(APPEND _IMPORT_CHECK_TARGETS pulsed_power_daq_python )
list(APPEND _IMPORT_CHECK_FILES_FOR_pulsed_power_daq_python "${_IMPORT_PREFIX}/lib/python3.8/dist-packages/pulsed_power_daq/pulsed_power_daq_python.cpython-38-x86_64-linux-gnu.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
