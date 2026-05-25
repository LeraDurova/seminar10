cmake_minimum_required(VERSION 3.16)
project(dz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0167 NEW)

# Installed at C:\local\boost_1_91_0
set(BOOST_ROOT "C:/local/boost_1_91_0" CACHE PATH "Boost root (must contain boost/ subdirectory)")

find_path(Boost_INCLUDE_DIR
    NAMES boost/asio.hpp
    PATHS
        ${BOOST_ROOT}
        ${BOOST_ROOT}/include
    NO_DEFAULT_PATH
)

if(NOT Boost_INCLUDE_DIR)
    message(FATAL_ERROR
        "Boost not found in ${BOOST_ROOT}\n"
        "Check that boost/asio.hpp exists inside")
endif()

message(STATUS "Boost: ${Boost_INCLUDE_DIR}")

function(setup_target target)
    target_include_directories(${target} PRIVATE ${Boost_INCLUDE_DIR})
    target_compile_definitions(${target} PRIVATE BOOST_ASIO_HEADER_ONLY)
    if(WIN32)
        target_link_libraries(${target} PRIVATE ws2_32 mswsock)
    endif()
endfunction()

add_executable(dz server.cpp)
setup_target(dz)

add_executable(client client.cpp)
setup_target(client)
