cmake_minimum_required(VERSION 3.10)
project(HttpServer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(apiserver ApiServer.cc UserSign.srpc.h FileManager.srpc.h UserSign.pb.cc UserSign.pb.h FileManager.pb.cc
FileManager.pb.h)


target_link_libraries(apiserver
    srpc
    protobuf
    workflow
    crypto
    wfrest
    lz4
    snappy
)

add_executable(userpc_server use_rpcserver.cc UserSign.srpc.h UserSign.pb.cc UserSign.pb.h)


target_link_libraries(userpc_server
        srpc
        protobuf
        workflow
        crypto
        wfrest
        lz4
        snappy
)

add_executable(filerpc_server dirinfoserver.cc FileManager.pb.cc FileManager.pb.h FileManager.srpc.h)


target_link_libraries(filerpc_server
        srpc
        protobuf
        workflow
        crypt
        wfrest
        lz4
        snappy
)



