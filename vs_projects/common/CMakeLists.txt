# Common static library
add_library(Common STATIC
    src/Algorithm.cpp
    src/Common.cpp
    src/File.cpp
    src/Mutex.cpp
    src/Network.cpp
    src/Tracer.cpp
    src/Window.cpp
)

target_include_directories(Common PUBLIC
    ${CMAKE_SOURCE_DIR}/vs_projects
)

target_compile_definitions(Common PUBLIC
    WIN32
    _LIB
    COMMON_STATIC
)

# Windows libraries needed by Common
target_link_libraries(Common PUBLIC
    Ws2_32
)
