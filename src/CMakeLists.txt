cmake_minimum_required(VERSION 3.5)

project(img_recogn VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(UI_SOURCES
    ui/utility_ctx.h
    ui/proc_ctx.h
    ui/proc_ctx.cpp
    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui
)

set(PROCESSING_SOURCES
    processing/image.h
    processing/processor_api.h
    processing/common_processors.h
    processing/pixel_view.h
    processing/pixel_view.cpp

    processing/processors/grayscale.cpp
    processing/processors/duotone.cpp
    processing/processors/fill.cpp
    processing/processors/thin_letters.cpp
    processing/processors/irreg_cleanup.cpp
    processing/processors/letter_finder.cpp

    processing/processors/letters/letter_reader.h
    processing/processors/letters/letter_reader.cpp
)

set(PSD_SOURCES
    psd/psd_manager.h
    psd/psd_file_handlers.cpp
    psd/psd_manager.cpp
)

set(PROJECT_SOURCES
    ${PROCESSING_SOURCES}
    ${PSD_SOURCES}
    ${UI_SOURCES}
    main.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(img_recogn
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET img_recogn APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(img_recogn SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(img_recogn
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(img_recogn PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(img_recogn PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS img_recogn
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(img_recogn)
endif()
