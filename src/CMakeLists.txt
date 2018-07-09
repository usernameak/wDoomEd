cmake_minimum_required(VERSION 3.0)

include_directories(.)

add_executable(wDoomEd
    app.cpp
    main_frame.cpp
    canvas.cpp
    wad.cpp
    map_editor.cpp)

find_package(wxWidgets COMPONENTS gl core base)
    
include(${wxWidgets_USE_FILE})

target_link_libraries(wDoomEd ${wxWidgets_LIBRARIES} GL GLU)