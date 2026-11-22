cmake_minimum_required(VERSION 3.20)

project(HorizonCompatibilityRuntime
    VERSION 0.1.0
    DESCRIPTION "Experimental Xbox-to-Windows compatibility research runtime"
    LANGUAGES CXX
)

option(HORIZON_BUILD_TESTS "Build HORIZON synthetic tests" ON)

add_library(horizon_core
    src/pe_inspector.cpp
)

target_include_directories(horizon_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(horizon_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(horizon_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(horizon_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(horizon-inspect
    src/main.cpp
)

target_link_libraries(horizon-inspect PRIVATE horizon_core)

if(HORIZON_BUILD_TESTS)
    enable_testing()

    add_executable(horizon-tests
        tests/pe_inspector_tests.cpp
    )

    target_link_libraries(horizon-tests PRIVATE horizon_core)
    add_test(NAME horizon-pe-inspector COMMAND horizon-tests)
endif()
