cmake_minimum_required(VERSION 3.16)

project(TLMAnalyzer)

set(CMAKE_PREFIX_PATH "E:/Qt/6.10.0/msvc2022_64/lib/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Qt6 COMPONENTS
    Core
    Gui
    Quick
    QuickControls2
    Qml
    REQUIRED)

qt6_standard_project_setup()

qt6_add_executable(${PROJECT_NAME}
        main.cpp
        csvprocessor.cpp
        calculator.cpp
        datamanager.cpp
        include/datapoint.h
        include/csvprocessor.h
        include/calculator.h
        include/datamanager.h
        qml.qrc
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Qml
)

    # Run windeployqt after build to collect all required DLLs
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_INSTALL_PATH}/bin")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} 
            --dir "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            --qmldir "${CMAKE_SOURCE_DIR}/qml"
            --plugindir "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins"
            --pdb
            $<TARGET_FILE:${PROJECT_NAME}>
            COMMENT "Running windeployqt to collect required DLLs..."
        )
    endif()