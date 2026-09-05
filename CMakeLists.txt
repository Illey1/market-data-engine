cmake_minimum_required(VERSION 3.20)
project(market_data_engine LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_library(market_data_engine src/system_event.cpp)
target_include_directories(market_data_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(market_data_engine PUBLIC cxx_std_20)

include(CTest)
if(BUILD_TESTING)
    add_executable(system_event_test tests/system_event_test.cpp)
    target_link_libraries(system_event_test PRIVATE market_data_engine)
    add_test(NAME system_event_test COMMAND system_event_test)
endif()
