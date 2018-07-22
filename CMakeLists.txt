cmake_minimum_required(VERSION 3.0)

include_directories(.)

project("wDoomEd")

set(CMAKE_CXX_STANDARD 11)

set(USE_ASAN OFF CACHE BOOL "Use ASAN")

if(USE_ASAN)	
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address ")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address ")
endif()

add_executable(wDoomEd
    src/app.cpp
    src/main_frame.cpp
    src/canvas.cpp
    src/map_editor.cpp
    src/properties_dialog.cpp
    src/polygon_2d.cpp
    src/math_util.cpp
    src/texture.cpp
    src/w_wad.cpp)

find_package(wxWidgets COMPONENTS gl core base)
    
include(${wxWidgets_USE_FILE})

target_link_libraries(wDoomEd ${wxWidgets_LIBRARIES} GL GLU GLEW $<$<BOOL:${USE_ASAN}>:asan>)

set_target_properties(wDoomEd
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

file(COPY res DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)