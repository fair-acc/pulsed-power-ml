# CMake generated Testfile for 
# Source directory: /home/johanna/gr-digitizers/python
# Build directory: /home/johanna/gr-digitizers/python
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(qa_power_calc_ff_prepper "/usr/bin/sh" "qa_power_calc_ff_prepper_test.sh")
set_tests_properties(qa_power_calc_ff_prepper PROPERTIES  _BACKTRACE_TRIPLES "/usr/lib/x86_64-linux-gnu/cmake/gnuradio/GrTest.cmake;116;add_test;/home/johanna/gr-digitizers/python/CMakeLists.txt;34;GR_ADD_TEST;/home/johanna/gr-digitizers/python/CMakeLists.txt;0;")
subdirs("bindings")
