cmake_minimum_required(VERSION 3.0)

project(cyclops-apps C)

set_property(GLOBAL PROPERTY C_STANDARD 11)

include_directories(include)

add_subdirectory(lib)
add_subdirectory(monitor)
#add_subdirectory(hashfile)
#add_subdirectory(gst_capture)
#add_subdirectory(read_rtc)
#add_subdirectory(eth_test)

