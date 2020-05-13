project(kata1)

cmake_minimum_required(VERSION 3.5)

include(ExternalProject)

# if no cdt root is given use default path
if(EOSIO_CDT_ROOT STREQUAL "" OR NOT EOSIO_CDT_ROOT)
    find_package(eosio.cdt)
endif()
message("EOSIO_CDT_ROOT = ${EOSIO_CDT_ROOT}")

ExternalProject_Add(
        kata1_project
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
        BINARY_DIR ${CMAKE_BINARY_DIR}/kata1
        CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
)

ExternalProject_Add(
        kata1_test_project
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/tests
        BINARY_DIR ${CMAKE_BINARY_DIR}/tests
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DEOSIO_ROOT=${EOSIO_ROOT} -DEOSIO_DEPENDENCY=${EOSIO_DEPENDENCY}
        TEST_COMMAND ./kata1_test --color_output  --log_level=all -- --verbose
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
)

add_dependencies( kata1_test_project kata1_project)