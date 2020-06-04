cmake_minimum_required(VERSION 3.1.0)

####### Global parameters #############################

project(qjrad)
set(QJRAD ${CMAKE_PROJECT_NAME})
set(KANJICONV kanjiconv)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION 5.14)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050E00
)

find_package(Threads REQUIRED)
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Xml
    Network
    DBus
)

add_subdirectory(zdict)

####### Optional Tesseract linking #####################

find_package(PkgConfig REQUIRED)
 
pkg_search_module(TESSERACT tesseract)
if (TESSERACT_FOUND)
     add_compile_definitions(WITH_OCR)
 
     find_package(Qt5X11Extras)
     find_package(XCB REQUIRED COMPONENTS XFIXES IMAGE)
     
     set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${TESSERACT_CFLAGS_OTHER} -Wno-ignored-qualifiers")
     set(TESS_LIBS Qt5::X11Extras ${TESSERACT_LIBRARIES} ${XCB_LIBRARIES} -llept)
     include_directories(
         ${TESSERACT_INCLUDE_DIRS}
         ${XCB_INCLUDE_DIRS}
     )
     
     message("Using Tesseract OCR:  YES")
else()
     message("Using Tesseract OCR:  NO")
endif()

####### Main project sources ###########################

set(SOURCES
    main.cpp
    mainwindow.cpp
    kdictionary.cpp
    kanjimodel.cpp
    settingsdlg.cpp
    global.cpp
    dbusdict.cpp
    regiongrabber.cpp
    xcbtools.cpp
)

set(SOURCES_UI
    mainwindow.ui
    settingsdlg.ui
)

qt5_wrap_ui(HEADERS_UI ${SOURCES_UI})
qt5_add_resources(RESOURCES qjrad.qrc)
qt5_add_dbus_adaptor(DBUS_ADAPTORS org.qjrad.dictionary.xml dbusdict.h ZKanjiDBusDict dictionary_adaptor)

set(LIBS
    -lz -ltbb
)

######## Kanjiconv subproject sources ##############################

set(KANJICONV_SOURCES
    converter/main.cpp
    kdictionary.cpp
)

qt5_add_resources(KANJICONV_RESOURCES converter/kanjiconv.qrc)

####### Main project executable ########################

add_executable(${QJRAD}
    ${SOURCES}
    ${HEADERS_UI}
    ${RESOURCES}
    ${DBUS_ADAPTORS}
)

target_link_libraries(${QJRAD}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Xml
    Qt5::Network
    Qt5::DBus
    ${LIBS}
    ${TESS_LIBS}
    Threads::Threads
    zdict
)

####### Kanjiconv executable ##########################

add_executable(${KANJICONV}
    ${KANJICONV_SOURCES}
    ${KANJICONV_RESOURCES}
)

target_link_libraries(${KANJICONV}
    Qt5::Core
    Qt5::Xml
)
