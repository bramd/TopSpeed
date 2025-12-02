# Common static library
# Platform-independent sources
set(COMMON_SOURCES
    Src/Algorithm.cpp
    Src/File.cpp
    Src/Tracer.cpp
)

if(EMSCRIPTEN)
    # WASM: Add stub implementations for Windows-specific functionality
    list(APPEND COMMON_SOURCES
        Src/Common_Emscripten.cpp
    )
else()
    # Windows: Full implementations
    list(APPEND COMMON_SOURCES
        Src/Common.cpp
        Src/Mutex.cpp
        Src/Network.cpp
        Src/Window.cpp
    )
endif()

add_library(Common STATIC ${COMMON_SOURCES})

target_include_directories(Common PUBLIC
    ${CMAKE_SOURCE_DIR}/vs_projects
)

if(EMSCRIPTEN)
    target_compile_definitions(Common PUBLIC
        _LIB
        COMMON_STATIC
    )
else()
    target_compile_definitions(Common PUBLIC
        WIN32
        _LIB
        COMMON_STATIC
    )
    # Windows libraries needed by Common
    target_link_libraries(Common PUBLIC
        Ws2_32
    )
endif()
