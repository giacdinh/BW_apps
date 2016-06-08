find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig)

pkg_check_modules(LIBXML libxml-2.0)
pkg_check_modules(JSON json-c)
pkg_check_modules(CURL libcurl)

include_directories(
    ${OPENSSL_INCLUDE_DIR}
    ${LIBXML_INCLUDE_DIRS}
    ${JSON_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

file(GLOB MONITOR_SOURCES *.c)
add_executable(monitor ${MONITOR_SOURCES})
target_link_libraries(
    monitor
    dl
    mosquitto
    ezxml
    Flash2AppSM
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBRARIES}
    ${LIBXML_LIBRARIES}
    ${JSON_LIBRARIES}
    ${CURL_LIBRARIES}
)

install(TARGETS monitor RUNTIME DESTINATION bin)