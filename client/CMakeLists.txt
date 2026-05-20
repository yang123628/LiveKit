cmake_minimum_required(VERSION 3.16)
project(LiveKitClient VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt5 REQUIRED COMPONENTS Widgets Network WebSockets Multimedia)

set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/third_party/ffmpeg-4.2.2" CACHE PATH "FFmpeg root directory")
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/third_party/SDL2-2.0.10" CACHE PATH "SDL2 root directory")
set(OPENCV_DIR "${CMAKE_SOURCE_DIR}/third_party/opencv-4.2.0" CACHE PATH "OpenCV root directory")

if(EXISTS "${FFMPEG_DIR}/include")
    include_directories("${FFMPEG_DIR}/include")
    link_directories("${FFMPEG_DIR}/lib")
endif()

if(EXISTS "${SDL2_DIR}/include")
    include_directories("${SDL2_DIR}/include")
    link_directories("${SDL2_DIR}/lib")
endif()

if(EXISTS "${OPENCV_DIR}/include")
    include_directories("${OPENCV_DIR}/include")
    include_directories("${OPENCV_DIR}/include/opencv2")
    link_directories("${OPENCV_DIR}/lib")
endif()

set(SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/app/AppConfig.cpp
    src/ui/MainWindow.cpp
    src/theme/ThemeManager.cpp
)

set(HEADERS
    src/app/Application.h
    src/app/AppConfig.h
    src/ui/MainWindow.h
    src/theme/ThemeManager.h
)

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
)

if(EXISTS "${FFMPEG_DIR}/lib")
    target_link_libraries(LiveKitClient PRIVATE
        avcodec avformat avutil swscale swresample avdevice avfilter
    )
endif()

if(EXISTS "${SDL2_DIR}/lib")
    target_link_libraries(LiveKitClient PRIVATE SDL2 SDL2main)
endif()

if(EXISTS "${OPENCV_DIR}/lib")
    target_link_libraries(LiveKitClient PRIVATE opencv_world420)
endif()
