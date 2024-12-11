cmake_minimum_required(VERSION 3.16)

project(OpenCV_Integration VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt6 REQUIRED COMPONENTS Concurrent UiTools PrintSupport)

find_package(OpenCV REQUIRED)


include_directories(${CMAKE_SOURCE_DIR}/fftw-3.3.5-dll64)
link_directories(${CMAKE_SOURCE_DIR}/fftw-3.3.5-dll64)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ja)

set(PROJECT_SOURCES
        mainwindow.cpp mainwindow.h mainwindow.ui
        image.png
        main.cpp mainwindow.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(OpenCV_Integration
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        utils.cpp
        utils.h

        FlameProcessing.h
        FlameProcessing.cpp
        calculateWidth.h calculateWidth.cpp calculateWidth.ui
        scalingtool.h scalingtool.cpp scalingtool.ui
        frequencydetection.h frequencydetection.cpp frequencydetection.ui
        qgraphicsviewresize.h qgraphicsviewresize.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET OpenCV_Integration APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(OpenCV_Integration SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(OpenCV_Integration
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# MAKE SURE TO GO TO BUILD STEPS TO ADD update_translations AND release_translations AS BUILD STEPS!
qt6_add_translations(TARGETS OpenCV_Integration
    QM_FILES_OUTPUT_VARIABLE qm_files
    INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}
    LUPDATE_TARGET update_translations
    LUPDATE_OPTIONS "-noobsolete"
)
install(FILES ${qm_files} DESTINATION ${CMAKE_SOURCE_DIR}/translations)


target_link_libraries(OpenCV_Integration PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${OpenCV_LIBS} Qt6::Concurrent Qt6::UiTools -llibfftw3-3 Qt6::PrintSupport)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.OpenCV_Integration)
endif()
set_target_properties(OpenCV_Integration PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS OpenCV_Integration
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(OpenCV_Integration)
endif()

qt_generate_deploy_app_script(
    TARGET OpenCV_Integration
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
