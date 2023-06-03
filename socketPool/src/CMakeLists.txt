cmake_minimum_required(VERSION 2.8)

include_directories(../include)

aux_source_directory(. SRC_LIST)

add_executable(tcpsocket ${SRC_LIST})


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
