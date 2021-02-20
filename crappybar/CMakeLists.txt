cmake_minimum_required(VERSION 3.19.2)
project(bar)

file(GLOB SOURCE_LIST
     "src/*.c"
)

file(GLOB LIB_LIST
     "lib/*.c"
)

set(CMAKE_BUILD_TYPE Debug)

set(SOURCE_FILES ${SOURCE_LIST} ${LIB_LIST})
add_executable(bar ${SOURCE_FILES})

target_include_directories(bar PRIVATE "lib/")

find_package(X11 REQUIRED)
target_link_libraries(bar ${X11_LIBRARIES})
target_include_directories(bar PRIVATE ${X11_INCLUDE_DIR})

set(CAIRO_INCLUDE_DIRS "/usr/include/cairo/")
set(CAIRO_LIBRARIES "/usr/lib/libcairo.so")
target_link_libraries(bar ${CAIRO_LIBRARIES})
target_include_directories(bar PRIVATE ${CAIRO_INCLUDE_DIRS})

find_package(Threads REQUIRED)
target_link_libraries(bar ${CMAKE_THREAD_LIBS_INIT})

message("X11 Includes: ${X11_INCLUDE_DIR}")
message("X11 Libraries: ${X11_LIBRARIES}")
message("Cairo Includes: ${CAIRO_INCLUDE_DIRS}")
message("Cairo Libraries: ${CAIRO_LIBRARIES}")

message("\n")
