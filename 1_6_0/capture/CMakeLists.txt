find_package(PkgConfig)

pkg_check_modules(LIBXML libxml-2.0)
pkg_check_modules(LIBGST gstreamer-1.0)

link_directories(
    ${LIBXML_LIBRARY_DIRS}
    ${LIBGST_LIBRARY_DIRS}
)

include_directories(
    ${LIBXML_INCLUDE_DIRS}
    ${LIBGST_INCLUDE_DIRS}
)

add_library(capture STATIC capture_manager.c capture_session.c capture_config.c)
target_link_libraries(capture ${LIBXML_LIBRARIES} ${LIBGST_LIBRARIES})

add_executable(gst_capture capture.c)
target_link_libraries(gst_capture capture)

if (BUILD_DOC)
    include(GtkDoc)
    gtkdoc(capture ${CMAKE_CURRENT_SOURCE_DIR} "${LIBGST_LDFLAGS};$<TARGET_FILE:capture>")
endif()

#install(DIRECTORY  PUBLIC_HEADERS DESTINATION include/avstream)
install(TARGETS gst_capture RUNTIME DESTINATION bin)
#install(TARGETS capture LIBRARY DESTINATION lib)
