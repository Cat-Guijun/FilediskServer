cmake_minimum_required(VERSION 3.10)
project(FilediskServer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(workflow REQUIRED)

file(GLOB SOURCE_FILES "*.cc")
file(GLOB HEADER_FILES "*.h")


# target_link_libraries(
#     logserver
#     pthread
#     log4cpp
#     SimpleAmqpClient 
#     rabbitmq
# )



add_executable(
    filedisk_server
    ${SOURCE_FILES}
    ${HEADER_FILES}
        ../../../../Logger.cc
)


target_link_libraries(
    filedisk_server
    pthread
    workflow
    redis++
        hiredis
    mysqlclient
    log4cpp
    SimpleAmqpClient 
    rabbitmq
    crypto
)
