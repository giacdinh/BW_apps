find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(hashfile hashfile.c)
target_link_libraries(hashfile Flash2AppSM ${OPENSSL_LIBRARIES})
install(TARGETS hashfile RUNTIME DESTINATION bin)