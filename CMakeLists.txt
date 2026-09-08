cmake_minimum_required(VERSION 3.20)
project(market_data_engine LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_library(market_data_engine
    src/system_event.cpp
    src/add_order.cpp
    src/binary_file.cpp
    src/modify_order.cpp
    src/order_tracker.cpp
    src/order_book.cpp
    src/stock_directory.cpp
)
target_include_directories(market_data_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(market_data_engine PUBLIC cxx_std_20)

add_executable(market_data_inspect src/market_data_inspect.cpp)
target_link_libraries(market_data_inspect PRIVATE market_data_engine)

include(CTest)
if(BUILD_TESTING)
    add_executable(system_event_test tests/system_event_test.cpp)
    target_link_libraries(system_event_test PRIVATE market_data_engine)
    add_test(NAME system_event_test COMMAND system_event_test)

    add_executable(add_order_test tests/add_order_test.cpp)
    target_link_libraries(add_order_test PRIVATE market_data_engine)
    add_test(NAME add_order_test COMMAND add_order_test)

    add_executable(binary_file_test tests/binary_file_test.cpp)
    target_link_libraries(binary_file_test PRIVATE market_data_engine)
    add_test(NAME binary_file_test COMMAND binary_file_test)

    add_executable(modify_order_test tests/modify_order_test.cpp)
    target_link_libraries(modify_order_test PRIVATE market_data_engine)
    add_test(NAME modify_order_test COMMAND modify_order_test)

    add_executable(order_tracker_test tests/order_tracker_test.cpp)
    target_link_libraries(order_tracker_test PRIVATE market_data_engine)
    add_test(NAME order_tracker_test COMMAND order_tracker_test)

    add_executable(order_book_test tests/order_book_test.cpp)
    target_link_libraries(order_book_test PRIVATE market_data_engine)
    add_test(NAME order_book_test COMMAND order_book_test)

    add_executable(stock_directory_test tests/stock_directory_test.cpp)
    target_link_libraries(stock_directory_test PRIVATE market_data_engine)
    add_test(NAME stock_directory_test COMMAND stock_directory_test)

    add_executable(consistency_test tests/consistency_test.cpp)
    target_link_libraries(consistency_test PRIVATE market_data_engine)
    add_test(NAME consistency_test COMMAND consistency_test)
endif()
