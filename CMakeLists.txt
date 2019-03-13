cmake_minimum_required(VERSION 3.6.0)
project(GoldSprintsFatracing)

message(STATUS ===========================================================)
message(STATUS ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)


# code below is for QT5
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

if (Qt5_POSITION_INDEPENDENT_CODE)
	SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()


set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.61.0 REQUIRED system filesystem thread regex date_time serialization locale)

set(core_dir Core/)
set(common_dir Common/)
set(xml_dir XML/)
set(black_box_dir BlackBox/)
set(ui_dir UI/)

set(core_sources
        ${core_dir}Settings.h
        ${core_dir}Settings.cpp
        ${core_dir}Race.h
        ${core_dir}Race.cpp
        ${core_dir}Defines.h
)

set(black_box_sources
        ${black_box_dir}BlackBox.h
        ${black_box_dir}BlackBox.cpp
)
set(ui_sources
        ${ui_dir}GoldSprintsFatracing.cpp
        ${ui_dir}GoldSprintsFatracing.h
        ${ui_dir}goldsprintsfatracing.ui

        ${ui_dir}RaceWindow.cpp
        ${ui_dir}RaceWindow.h
        ${ui_dir}racewindow.ui

        ${ui_dir}SettingsWindow.cpp
        ${ui_dir}SettingsWindow.h
        ${ui_dir}settingswindow.ui
)
set(common_sources
	${common_dir}AsyncQueue.h
	${common_dir}BaseSettings.h
	${common_dir}BaseSettingsList.h
	${common_dir}BaseThread.cpp
	${common_dir}BaseThread.h
	${common_dir}Logger.cpp
	${common_dir}Logger.h
	${common_dir}Singleton.h
	${common_dir}Utils.cpp
	${common_dir}Utils.h
)

set(xml_sources
        ${xml_dir}GoldSprintsSettings.xml
)


set(sources
        main.cpp
        ${core_sources}
        ${common_sources}
        ${black_box_sources}
	${ui_sources}
	${xml_sources}
)

add_executable(${PROJECT_NAME} ${sources})

include_directories(${Qt5Widgets_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)


include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} -licuuc -ldl)

include_directories(${common_dir})
configure_file(XML/GoldSprintsSettings.xml ${CMAKE_CURRENT_BINARY_DIR}/GoldSprintsSettings.xml)

