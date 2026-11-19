cmake_minimum_required(VERSION 3.16)
project(W_AMR_Sim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

qt_standard_project_setup()

find_package(SQLite3 REQUIRED)

set(PROJECT_SOURCES
    src/main.cpp
    src/CRole.cpp
    src/CUser.cpp
    src/CCell.cpp
    src/CWarehouseMap.cpp
    src/CTask.cpp
    src/CRobot.cpp
    src/PathFinder.cpp
    src/CSimulationSession.cpp
    src/CDatabaseManager.cpp
    src/WarehouseGridWidget.cpp
    src/MapEditorWidget.cpp
    src/SimulationMapWidget.cpp
    src/OperatorScenarioWidget.cpp
    src/SimulationControlWidget.cpp
    src/AdminUsersWidget.cpp
    src/MainWindow.cpp
)

set(PROJECT_HEADERS
    include/CRole.h
    include/CUser.h
    include/CCell.h
    include/CWarehouseMap.h
    include/CTask.h
    include/CRobot.h
    include/PathFinder.h
    include/CSimulationSession.h
    include/CDatabaseManager.h
    include/WarehouseGridWidget.h
    include/MapEditorWidget.h
    include/SimulationMapWidget.h
    include/OperatorScenarioWidget.h
    include/SimulationControlWidget.h
    include/AdminUsersWidget.h
    include/MainWindow.h
)


add_executable(w_amr_sim
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

target_include_directories(w_amr_sim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLITE3_INCLUDE_DIRS}
)

target_link_libraries(w_amr_sim PRIVATE
    Qt6::Widgets
    ${SQLITE3_LIBRARIES}
)

target_link_libraries(w_amr_sim PRIVATE Qt6::Widgets SQLite::SQLite3)