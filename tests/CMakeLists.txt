cmake_minimum_required( VERSION 3.5 )
project(kata1_test)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt PATHS ${CMAKE_SOURCE_DIR}/../../eosio.cdt/build)
find_package(eosio PATHS ${CMAKE_SOURCE_DIR}/../../eos/build)

string(FIND "${EOSIO_VERSION}" "${EOSIO_DEPENDENCY}" output)
if (NOT "${output}" EQUAL 0)
    message(FATAL_ERROR "Incorrect EOSIO version, please use version ${EOSIO_DEPENDENCY}.x")
endif()

set( EOSIO_ROOT "../../../" )
configure_file( ${CMAKE_SOURCE_DIR}/contracts.hpp.in ${CMAKE_BINARY_DIR}/contracts.hpp )
enable_testing()
message("EOSIO.CDT dir = ${EOSIO_CDT_ROOT}")
include_directories( ${CMAKE_BINARY_DIR}  
                     ${EOSIO_ROOT}/eosio.contracts/contracts/eosio.token/include 
                     ${CMAKE_SOURCE_DIR}/../include
                     ${EOSIO_CDT_ROOT}/opt/eosio.cdt/include/eosiolib/contracts/)

file( GLOB KATA1_TESTS "*.cpp" "*.hpp" )

add_eosio_test( kata1_test ${KATA1_TESTS} )

target_compile_options( kata1_test PUBLIC -ftemplate-backtrace-limit=0 )