project(kata1)

cmake_minimum_required(VERSION 3.5)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( kata1 kata1 kata1.cpp )

#set( EOSIO_ROOT "$ENV{EOSIO_ROOT}" )
set( EOSIO_ROOT "../../" )

target_include_directories(
        kata1 PUBLIC
        ${EOSIO_ROOT}/eosio.contracts/contracts/eosio.token/include
        ${EOSIO_ROOT}/eos/libraries/chain/include
        ${CMAKE_SOURCE_DIR}/../include )