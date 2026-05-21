cmake_minimum_required(VERSION 3.10)
project(LiveKitServer C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_library(sqlite3 STATIC third_party/sqlite3/sqlite3.c)
target_compile_definitions(sqlite3 PRIVATE SQLITE_THREADSAFE=1)
target_include_directories(sqlite3 PUBLIC third_party/sqlite3)

add_executable(LiveKitServer
    src/main.cpp
    src/core/Logger.cpp
    src/core/EpollWrapper.cpp
    src/core/EventLoop.cpp
    src/core/ThreadPool.cpp
    src/core/TaskQueue.cpp
    src/network/Buffer.cpp
    src/network/Connection.cpp
    src/network/HttpRequest.cpp
    src/network/HttpResponse.cpp
    src/network/HttpServer.cpp
    src/network/Router.cpp
    src/network/WebSocketFrame.cpp
    src/network/WebSocketHandler.cpp
    src/utils/Config.cpp
    src/utils/Crypto.cpp
    src/utils/TokenGenerator.cpp
    src/database/Database.cpp
    src/database/UserDao.cpp
    src/database/RoomDao.cpp
    src/database/GiftDao.cpp
    src/business/UserService.cpp
    src/business/RoomService.cpp
    src/business/RoomManager.cpp
    src/business/GiftService.cpp
)

target_include_directories(LiveKitServer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
)

target_link_libraries(LiveKitServer PRIVATE
    Threads::Threads
    sqlite3
    dl
    ssl
    crypto
)
