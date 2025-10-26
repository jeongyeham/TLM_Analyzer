cmake_minimum_required(VERSION 3.13)

project(TLMAnalyzer)

set(CMAKE_PREFIX_PATH "E:/Qt/6.10.0/msvc2022_64/lib/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    Charts
    REQUIRED)

# 添加可执行文件
add_executable(${PROJECT_NAME} WIN32
    main.cpp
    mainwindow.cpp
    csvprocessor.cpp
    calculator.cpp
    datamanager.cpp
        include/mainwindow.h
        include/datapoint.h
        include/csvprocessor.h
        include/calculator.h
        include/datamanager.h
)

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
)

# Windows平台特定配置
if (WIN32)
    set(DEBUG_SUFFIX "")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(DEBUG_SUFFIX "d")
    endif()
    
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif()
    endif()
    
    # 复制Qt运行时库
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/"
        )
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/"
        )
    endif()
    
    # 复制必要的Qt DLL文件
    foreach(QT_LIB Core Gui Widgets Charts OpenGL OpenGLWidgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )
    endforeach()

endif()