cmake_minimum_required(VERSION 3.16)
project(LiveKitClient VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt5 REQUIRED COMPONENTS Widgets Network WebSockets Multimedia Svg OpenGL)

set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/third_party/ffmpeg-4.2.2" CACHE PATH "FFmpeg root directory")
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/third_party/SDL2-2.0.10" CACHE PATH "SDL2 root directory")
set(OPENCV_DIR "${CMAKE_SOURCE_DIR}/third_party/opencv-4.2.0" CACHE PATH "OpenCV root directory")

set(HAS_FFMPEG OFF)
set(HAS_SDL2 OFF)
set(HAS_OPENCV OFF)

if(EXISTS "${FFMPEG_DIR}/include")
    include_directories("${FFMPEG_DIR}/include")
    link_directories("${FFMPEG_DIR}/lib")
    set(HAS_FFMPEG ON)
endif()

if(EXISTS "${SDL2_DIR}/include")
    include_directories("${SDL2_DIR}/include")
    link_directories("${SDL2_DIR}/lib")
    set(HAS_SDL2 ON)
endif()

if(EXISTS "${OPENCV_DIR}/include")
    include_directories("${OPENCV_DIR}/include")
    include_directories("${OPENCV_DIR}/include/opencv2")
    link_directories("${OPENCV_DIR}/lib")
    set(HAS_OPENCV ON)
endif()

set(SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/app/AppConfig.cpp
    src/ui/MainWindow.cpp
    src/ui/LoginPage.cpp
    src/ui/RegisterPage.cpp
    src/ui/LiveHallPage.cpp
    src/ui/RoomCard.cpp
    src/ui/OpenGLWidget.cpp
    src/ui/StartLivePage.cpp
    src/ui/PicInPicWidget.cpp
    src/theme/ThemeManager.cpp
    src/network/ApiResponse.cpp
    src/network/IHttpClient.cpp
    src/network/HttpClient.cpp
    src/network/MockHttpClient.cpp
    src/model/UserInfo.cpp
    src/model/RoomInfo.cpp
)

set(HEADERS
    src/app/Application.h
    src/app/AppConfig.h
    src/ui/MainWindow.h
    src/ui/LoginPage.h
    src/ui/RegisterPage.h
    src/ui/LiveHallPage.h
    src/ui/RoomCard.h
    src/ui/OpenGLWidget.h
    src/ui/StartLivePage.h
    src/ui/PicInPicWidget.h
    src/theme/ThemeManager.h
    src/network/ApiResponse.h
    src/network/IHttpClient.h
    src/network/HttpClient.h
    src/network/MockHttpClient.h
    src/model/UserInfo.h
    src/model/RoomInfo.h
)

if(HAS_FFMPEG AND HAS_SDL2 AND HAS_OPENCV)
    list(APPEND SOURCES
        src/core/PacketQueue.cpp
        src/core/CameraCapture.cpp
        src/core/DesktopCapture.cpp
        src/core/AudioCapture.cpp
        src/core/PicInPic.cpp
        src/core/VideoPusher.cpp
        src/core/VideoPlayer.cpp
        src/ui/AnchorRoomPage.cpp
        src/ui/LiveRoomPage.cpp
    )
    list(APPEND HEADERS
        src/core/PacketQueue.h
        src/core/CameraCapture.h
        src/core/DesktopCapture.h
        src/core/AudioCapture.h
        src/core/PicInPic.h
        src/core/VideoPusher.h
        src/core/VideoPlayer.h
        src/ui/AnchorRoomPage.h
        src/ui/LiveRoomPage.h
    )
    add_definitions(-DHAS_FFMPEG -DHAS_SDL2 -DHAS_OPENCV)
endif()

set(RESOURCES
    resources/resources.qrc
)

add_executable(LiveKitClient WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

target_include_directories(LiveKitClient PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(LiveKitClient PRIVATE
    Qt5::Widgets
    Qt5::Network
    Qt5::WebSockets
    Qt5::Multimedia
    Qt5::Svg
    Qt5::OpenGL
)

if(HAS_FFMPEG)
    target_link_libraries(LiveKitClient PRIVATE
        avcodec avformat avutil swscale swresample avdevice avfilter
    )
endif()

if(HAS_SDL2)
    target_link_libraries(LiveKitClient PRIVATE SDL2 SDL2main)
endif()

if(HAS_OPENCV)
    target_link_libraries(LiveKitClient PRIVATE opencv_world420)
endif()
