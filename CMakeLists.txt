cmake_minimum_required(VERSION 3.16)
project(fotainjectionplugin LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED DLT_VIEWER_SDK)
    message(FATAL_ERROR "Define DLT_VIEWER_SDK con la ruta del SDK de DLT Viewer")
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

find_library(QDLT_LIBRARY
    NAMES qdlt
    PATHS "${DLT_VIEWER_SDK}/lib"
    REQUIRED
)

add_library(fotainjectionplugin MODULE
    fota_injection_plugin.cpp
    fota_injection_plugin.h
)

target_include_directories(fotainjectionplugin PRIVATE
    "${DLT_VIEWER_SDK}/include"
    "${DLT_VIEWER_SDK}/include/qdlt"
)

target_link_libraries(fotainjectionplugin PRIVATE
    "${QDLT_LIBRARY}"
    Qt5::Core
    Qt5::Widgets
)

set_target_properties(fotainjectionplugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "fotainjectionplugin"
)

if(WIN32)
    set_target_properties(fotainjectionplugin PROPERTIES SUFFIX ".dll")
endif()
