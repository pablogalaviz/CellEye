cmake_minimum_required(VERSION 3.10)
project(CellEye)

set(CMAKE_CXX_STANDARD 14)


set(CMAKE_PREFIX_PATH "/Users/pgalaviz/Qt5.12.9/5.12.9/clang_64")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(CellEye main.cpp src/logger.h src/utils.h src/cellEye.cpp src/cellEye.h src/qt/main.ui src/qt/plot.ui src/qt/preferences.ui resources/resources.qrc src/ImageItem.cpp src/ImageItem.h src/sessionData.cpp src/sessionData.h src/frameData.cpp src/frameData.h src/plotDialog.cpp src/plotDialog.h src/pointItem.cpp src/pointItem.h src/preferencesDialog.cpp src/preferencesDialog.h)

#BOOST SETUP
set(BOOST_ROOT "/opt/boost")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost COMPONENTS program_options  iostreams  filesystem system REQUIRED)

#message(STATUS ${Boost_LIBRARIES})

set(OpenCV_STATIC ON)
find_package(OpenCV REQUIRED )

message(${OpenCV_LIBS})

#include_directories(/usr/local/include/)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

#QT SETUP
set(CMAKE_AUTOUIC_SEARCH_PATHS src/qt)

message(${CMAKE_AUTOUIC_SEARCH_PATHS})


if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets Core REQUIRED)

target_link_libraries(CellEye ${OpenCV_LIBS} Qt5::Widgets Boost::program_options Boost::iostreams  Boost::filesystem Boost::system )

install(TARGETS CellEye DESTINATION bin CONFIGURATIONS Release)