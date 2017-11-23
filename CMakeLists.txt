cmake_minimum_required(VERSION 3.0.0)
project(jsonmtz VERSION 1.0.8)

add_subdirectory(jansson)

include_directories("${PROJECT_BINARY_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/ccp4io")
include_directories("${PROJECT_BINARY_DIR}/jansson/include")

configure_file("${PROJECT_SOURCE_DIR}/include/jsonmtz.in.h" "${PROJECT_BINARY_DIR}/include/jsonmtz.h")

add_executable(mtz2json mtz2json.c)
set_property(TARGET mtz2json PROPERTY C_STANDARD 99)

add_executable(json2mtz json2mtz.c)
set_property(TARGET json2mtz PROPERTY C_STANDARD 99)

add_library(cmtz "${PROJECT_SOURCE_DIR}/ccp4io/cmtzlib.c" "${PROJECT_SOURCE_DIR}/ccp4io/ccp4_array.c" "${PROJECT_SOURCE_DIR}/ccp4io/ccp4_parser.c" "${PROJECT_SOURCE_DIR}/ccp4io/ccp4_unitcell.c" "${PROJECT_SOURCE_DIR}/ccp4io/cvecmat.c" "${PROJECT_SOURCE_DIR}/ccp4io/ccp4_general.c" "${PROJECT_SOURCE_DIR}/ccp4io/csymlib.c" "${PROJECT_SOURCE_DIR}/ccp4io/ccp4_program.c" "${PROJECT_SOURCE_DIR}/ccp4io/library_file.c" "${PROJECT_SOURCE_DIR}/ccp4io/library_err.c" "${PROJECT_SOURCE_DIR}/ccp4io/library_utils.c")
set_property(TARGET cmtz PROPERTY C_STANDARD 99)

add_library(jsonmtz "${PROJECT_SOURCE_DIR}/jsonmtz.c")
set_property(TARGET jsonmtz PROPERTY C_STANDARD 99)

if(WIN32)
    target_link_libraries(mtz2json jsonmtz cmtz "${JANSSON_LIBRARIES}")
    target_link_libraries(json2mtz jsonmtz cmtz "${JANSSON_LIBRARIES}")
endif()

if(APPLE)
    target_link_libraries(mtz2json jsonmtz cmtz "${JANSSON_LIBRARIES}")
    target_link_libraries(json2mtz jsonmtz cmtz "${JANSSON_LIBRARIES}")
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(mtz2json jsonmtz cmtz m "${JANSSON_LIBRARIES}")
    target_link_libraries(json2mtz jsonmtz cmtz m "${JANSSON_LIBRARIES}")
endif()