cmake_minimum_required(VERSION 3.0)

include_directories(.)

add_executable(wDoomEd
    src/app.cpp
    src/main_frame.cpp
    src/canvas.cpp
    src/wad.cpp
    src/map_editor.cpp)

find_package(wxWidgets COMPONENTS gl core base)
    
include(${wxWidgets_USE_FILE})

target_link_libraries(wDoomEd ${wxWidgets_LIBRARIES} GL GLU)

set_target_properties(wDoomEd
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

file(COPY res DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)